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

#include <drivers_config.h>

extern "C"
{
#include <Broker.h>
}

linux_ip_socket_private_data::linux_ip_socket_private_data()
    : m_thread(DRIVER_THREAD_PRIORITY, DRIVER_THREAD_STACK_SIZE)
{
}

void
linux_ip_socket_private_data::init(SystemBus bus_id,
                                   SystemDevice device_id,
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
    std::cout << "Creating driver thread\n" << std::endl;
    addrinfo* servinfo = nullptr;
    addrinfo* p = nullptr;
    int rc;

    fill_addrinfo(&servinfo, m_ip_device_configuration->address, m_ip_device_configuration->port);

    for(p = servinfo; p != NULL; p = p->ai_next) {
        m_ip_sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(m_ip_sockfd == -1) {
            std::cerr << "socket\n";
            continue;
        }
        rc = bind(m_ip_sockfd, p->ai_addr, p->ai_addrlen);
        if(rc == -1) {
            std::cerr << "bind\n";
            continue;
        }

        break;
    }

    if(p == NULL) {
        std::cerr << "Cannot create or bind socket\n";
        exit(3);
    }

    freeaddrinfo(servinfo);

    struct sockaddr_storage their_addr;

    static uint8_t message_buffer[BROKER_BUFFER_SIZE];
    size_t message_buffer_index = 0;

    linux_ip_socket_private_data::State state = linux_ip_socket_private_data::STATE_WAIT;

    static uint8_t buffer[256];
    while(true) {
        socklen_t addr_len = sizeof(sockaddr_storage);
        rc = recvfrom(m_ip_sockfd, buffer, 256, 0, (struct sockaddr*)&their_addr, &addr_len);
        if(rc == -1) {
            std::cerr << "recvfrom";
        } else {
            // parse packet
            for(int i = 0; i < rc; ++i) {
                switch(state) {
                    case linux_ip_socket_private_data::STATE_WAIT:
                        if(buffer[i] == linux_ip_socket_private_data::START_BYTE) {
                            state = linux_ip_socket_private_data::STATE_DATA_BYTE;
                        }
                        break;
                    case linux_ip_socket_private_data::STATE_DATA_BYTE:
                        if(buffer[i] == linux_ip_socket_private_data::STOP_BYTE) {
                            Broker_receive_packet(message_buffer, message_buffer_index);
                            message_buffer_index = 0;
                            state = linux_ip_socket_private_data::STATE_WAIT;
                        } else if(buffer[i] == linux_ip_socket_private_data::ESCAPE_BYTE) {
                            state = linux_ip_socket_private_data::STATE_ESCAPE_BYTE;
                        } else if(buffer[i] == linux_ip_socket_private_data::START_BYTE) {
                            message_buffer_index = 0;
                            state = linux_ip_socket_private_data::STATE_DATA_BYTE;
                        } else {
                            message_buffer[message_buffer_index] = buffer[i];
                            ++message_buffer_index;
                        }
                        break;
                    case linux_ip_socket_private_data::STATE_ESCAPE_BYTE:
                        message_buffer[message_buffer_index] = buffer[i];
                        ++message_buffer_index;
                        state = linux_ip_socket_private_data::STATE_DATA_BYTE;

                        break;
                }
            }
        }
    }
}

void
linux_ip_socket_private_data::send(uint8_t* data, size_t length)
{
    addrinfo* servinfo = nullptr;
    addrinfo* p = nullptr;
    int rc;

    fill_addrinfo(&servinfo, m_ip_remote_device_configuration->address, m_ip_remote_device_configuration->port);

    for(p = servinfo; p != NULL; p = p->ai_next) {
        break;
    }

    if(p == NULL) {
        std::cerr << "Cannot create or bind socket\n";
        exit(3);
    }

    static uint8_t send_buffer[256];
    size_t send_buffer_index = 0;

    send_buffer[0] = linux_ip_socket_private_data::START_BYTE;
    ++send_buffer_index;

    size_t index = 0;
    bool escape = false;
    while(index < length) {
        if(escape) {
            send_buffer[send_buffer_index] = data[index];
            ++send_buffer_index;
            ++index;
            escape = false;
        } else if(data[index] == linux_ip_socket_private_data::START_BYTE) {
            send_buffer[send_buffer_index] = linux_ip_socket_private_data::ESCAPE_BYTE;
            ++send_buffer_index;
            escape = true;
        } else if(data[index] == linux_ip_socket_private_data::ESCAPE_BYTE) {
            send_buffer[send_buffer_index] = linux_ip_socket_private_data::ESCAPE_BYTE;
            ++send_buffer_index;
            escape = true;
        } else if(data[index] == linux_ip_socket_private_data::STOP_BYTE) {
            send_buffer[send_buffer_index] = linux_ip_socket_private_data::ESCAPE_BYTE;
            ++send_buffer_index;
            escape = true;
        } else {
            send_buffer[send_buffer_index] = data[index];
            ++send_buffer_index;
            ++index;
        }

        if(send_buffer_index >= 256) {
            rc = sendto(m_ip_sockfd, send_buffer, 256, 0, p->ai_addr, p->ai_addrlen);
            if(rc == -1) {
                std::cerr << "sendto\n";
                return;
            }
            send_buffer_index = 0;
        }
    }

    if(send_buffer_index >= 256) {
        rc = sendto(m_ip_sockfd, send_buffer, 256, 0, p->ai_addr, p->ai_addrlen);
        if(rc == -1) {
            std::cerr << "sendto\n";
            return;
        }
        send_buffer_index = 0;
    }

    send_buffer[send_buffer_index] = linux_ip_socket_private_data::STOP_BYTE;
    ++send_buffer_index;
    rc = sendto(m_ip_sockfd, send_buffer, send_buffer_index, 0, p->ai_addr, p->ai_addrlen);
    if(rc == -1) {
        std::cerr << "sendto\n";
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

namespace taste {

void
LinuxIpSocketPoll(void* private_data)
{
    linux_ip_socket_private_data* self = reinterpret_cast<linux_ip_socket_private_data*>(private_data);
    self->poll();
}

void
LinuxIpSocketSend(void* private_data, uint8_t* data, size_t length)
{
    linux_ip_socket_private_data* self = reinterpret_cast<linux_ip_socket_private_data*>(private_data);
    self->send(data, length);
}

void
LinuxIpSocketInit(void* private_data,
                  enum SystemBus bus_id,
                  enum SystemDevice device_id,
                  const Socket_IP_Conf_T* const device_configuration,
                  const Socket_IP_Conf_T* const remote_device_configuration)
{
    linux_ip_socket_private_data* self = reinterpret_cast<linux_ip_socket_private_data*>(private_data);
    self->init(bus_id, device_id, device_configuration, remote_device_configuration);
}
} // namespace taste
