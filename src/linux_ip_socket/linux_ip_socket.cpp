/**@file
 * This file is part of the TASTE Linux Runtime.
 *
 * @copyright 2021 N7 Space Sp. z o.o.
 *
 * TASTE Linux Runtime was developed under a programme of,
 * and funded by, the European Space Agency (the "ESA").
 *
 * Licensed under the ESA Public License (ESA-PL) Permissive,
 * Version 2.3 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://essr.esa.int/license/list
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "linux_ip_socket.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>

linux_ip_socket_private_data::linux_ip_socket_private_data()
    : m_thread(DRIVER_THREAD_PRIORITY, DRIVER_THREAD_STACK_SIZE)
    , m_message_buffer_index(0)
{
}

void
linux_ip_socket_private_data::init(const SystemBus bus_id,
                                   const SystemDevice device_id,
                                   const Socket_IP_Conf_T* const device_configuration,
                                   const Socket_IP_Conf_T* const remote_device_configuration)
{
    m_ip_device_bus_id = bus_id;
    m_ip_device_id = device_id;
    m_ip_device_configuration = device_configuration;
    m_ip_remote_device_configuration = remote_device_configuration;
    m_thread.start(&taste::LinuxIpSocketPoll, this);
}

void
linux_ip_socket_private_data::poll()
{
    addrinfo* servinfo = nullptr;
    fill_addrinfo(&servinfo, m_ip_device_configuration->address, m_ip_device_configuration->port);

    addrinfo* p = nullptr;
    int rc = 0;
    for(p = servinfo; p != NULL; p = p->ai_next) {
        m_ip_sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(m_ip_sockfd == -1) {
            std::cerr << "socket() returned an error: " << strerror(errno) << std::endl;
            continue;
        }
        rc = bind(m_ip_sockfd, p->ai_addr, p->ai_addrlen);
        if(rc == -1) {
            std::cerr << "bind() returned an error: " << strerror(errno) << std::endl;
            continue;
        }

        break;
    }

    if(p == NULL) {
        std::cerr << "Cannot create or bind socket\n";
        exit(3);
    }

    freeaddrinfo(servinfo);

    struct sockaddr_storage remote_addr;

    while(true) {
        socklen_t addr_len = sizeof(sockaddr_storage);
        rc = recvfrom(
                m_ip_sockfd, m_recv_buffer, DRIVER_RECV_BUFFER_SIZE, 0, (struct sockaddr*)&remote_addr, &addr_len);
        if(rc == -1) {
            std::cerr << "recvfrom error\n";
        } else {
            parse_recv_buffer(rc);
        }
    }
}

void
linux_ip_socket_private_data::send(uint8_t* data, const size_t length)
{
    addrinfo* servinfo = nullptr;

    fill_addrinfo(&servinfo, m_ip_remote_device_configuration->address, m_ip_remote_device_configuration->port);

    addrinfo* p = nullptr;
    int rc = 0;
    for(p = servinfo; p != NULL; p = p->ai_next) {
        break;
    }

    if(p == NULL) {
        std::cerr << "Cannot create or bind socket\n";
        exit(3);
    }

    size_t send_buffer_index = 0;

    m_send_buffer[0] = START_BYTE;
    ++send_buffer_index;

    size_t index = 0;
    bool escape = false;
    while(index < length) {
        if(escape) {
            m_send_buffer[send_buffer_index] = data[index];
            ++send_buffer_index;
            ++index;
            escape = false;
        } else if(data[index] == START_BYTE) {
            m_send_buffer[send_buffer_index] = ESCAPE_BYTE;
            ++send_buffer_index;
            escape = true;
        } else if(data[index] == ESCAPE_BYTE) {
            m_send_buffer[send_buffer_index] = ESCAPE_BYTE;
            ++send_buffer_index;
            escape = true;
        } else if(data[index] == STOP_BYTE) {
            m_send_buffer[send_buffer_index] = ESCAPE_BYTE;
            ++send_buffer_index;
            escape = true;
        } else {
            m_send_buffer[send_buffer_index] = data[index];
            ++send_buffer_index;
            ++index;
        }

        if(send_buffer_index >= DRIVER_SEND_BUFFER_SIZE) {
            rc = sendto(m_ip_sockfd, m_send_buffer, DRIVER_SEND_BUFFER_SIZE, 0, p->ai_addr, p->ai_addrlen);
            if(rc == -1) {
                std::cerr << "sendto error\n";
                return;
            }
            send_buffer_index = 0;
        }
    }

    if(send_buffer_index >= DRIVER_SEND_BUFFER_SIZE) {
        rc = sendto(m_ip_sockfd, m_send_buffer, DRIVER_SEND_BUFFER_SIZE, 0, p->ai_addr, p->ai_addrlen);
        if(rc == -1) {
            std::cerr << "sendtoerror\n";
            return;
        }
        send_buffer_index = 0;
    }

    m_send_buffer[send_buffer_index] = STOP_BYTE;
    ++send_buffer_index;
    rc = sendto(m_ip_sockfd, m_send_buffer, send_buffer_index, 0, p->ai_addr, p->ai_addrlen);
    if(rc == -1) {
        std::cerr << "sendto error\n";
        return;
    }
    send_buffer_index = 0;
    freeaddrinfo(servinfo);
}

void
linux_ip_socket_private_data::fill_addrinfo(addrinfo** target, const char* address, unsigned int port)
{
    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    const std::string remote_host = std::string(address);
    const std::string remote_port = std::to_string(port);

    int rc = getaddrinfo(remote_host.c_str(), remote_port.c_str(), &hints, target);

    if(rc != 0) {
        std::cerr << "getaddrinfo returned an error: " << gai_strerror(rc) << std::endl;
        std::cerr << "Aborting." << std::endl;
        abort();
    }
}

void
linux_ip_socket_private_data::parse_recv_buffer(int length)
{
    // parse packet
    for(int i = 0; i < length; ++i) {
        switch(m_parse_state) {
            case STATE_WAIT:
                if(m_recv_buffer[i] == START_BYTE) {
                    m_parse_state = STATE_DATA_BYTE;
                }
                break;
            case STATE_DATA_BYTE:
                if(m_recv_buffer[i] == STOP_BYTE) {
                    Broker_receive_packet(m_message_buffer, m_message_buffer_index);
                    m_message_buffer_index = 0;
                    m_parse_state = STATE_WAIT;
                } else if(m_recv_buffer[i] == ESCAPE_BYTE) {
                    m_parse_state = STATE_ESCAPE_BYTE;
                } else if(m_recv_buffer[i] == START_BYTE) {
                    m_message_buffer_index = 0;
                    m_parse_state = STATE_DATA_BYTE;
                } else {
                    m_message_buffer[m_message_buffer_index] = m_recv_buffer[i];
                    ++m_message_buffer_index;
                }
                break;
            case STATE_ESCAPE_BYTE:
                m_message_buffer[m_message_buffer_index] = m_recv_buffer[i];
                ++m_message_buffer_index;
                m_parse_state = STATE_DATA_BYTE;

                break;
        }
    }
}

namespace taste {

void
LinuxIpSocketPoll(void* private_data)
{
    linux_ip_socket_private_data* self = reinterpret_cast<linux_ip_socket_private_data*>(private_data);
    self->poll();
}

void
LinuxIpSocketSend(void* private_data, uint8_t* data, const size_t length)
{
    linux_ip_socket_private_data* self = reinterpret_cast<linux_ip_socket_private_data*>(private_data);
    self->send(data, length);
}

void
LinuxIpSocketInit(void* private_data,
                  const enum SystemBus bus_id,
                  const enum SystemDevice device_id,
                  const Socket_IP_Conf_T* const device_configuration,
                  const Socket_IP_Conf_T* const remote_device_configuration)
{
    linux_ip_socket_private_data* self = reinterpret_cast<linux_ip_socket_private_data*>(private_data);
    self->init(bus_id, device_id, device_configuration, remote_device_configuration);
}
} // namespace taste
