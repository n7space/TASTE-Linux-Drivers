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
#include <poll.h>
#include <unistd.h>

linux_ip_socket_private_data::linux_ip_socket_private_data()
    : m_thread(DRIVER_THREAD_PRIORITY, DRIVER_THREAD_STACK_SIZE)
    , m_message_buffer_index(0)
{
}

void
linux_ip_socket_private_data::driver_init(const SystemBus bus_id,
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
linux_ip_socket_private_data::driver_poll()
{
    prepare_listen_socket();

    const int descriptors_table_size = DRIVER_MAX_CONNECTIONS + 1;
    struct pollfd descriptors_table[descriptors_table_size];
    int descriptors_table_count = 0;
    descriptors_table[0].fd = m_listen_sockfd;
    descriptors_table[0].events = POLLIN;
    ++descriptors_table_count;

    while(true) {
        const int poll_result = ::poll(descriptors_table, descriptors_table_count, -1);
        if(poll_result == -1) {
            std::cerr << "poll() returned an error: " << strerror(errno) << std::endl;
            std::cerr << "aborting." << std::endl;
            abort();
        }

        for(int descriptor_index = 0; descriptor_index < descriptors_table_count; ++descriptor_index) {
            if(descriptors_table[descriptor_index].revents & POLLIN) {
                if(descriptor_index == 0) {
                    accept_connection(descriptors_table, descriptors_table_count);
                } else {
                    read_data_or_disconnect(descriptor_index, descriptors_table, descriptors_table_count);
                }
            }
        }
    }
}

void
linux_ip_socket_private_data::driver_send(const uint8_t* const data, const size_t length)
{
    const int sockfd = connect_to_remote_driver();
    if(sockfd == INVALID_SOCKET_ID) {
        return;
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
            send_packet(sockfd, DRIVER_SEND_BUFFER_SIZE);
            send_buffer_index = 0;
        }
    }

    m_send_buffer[send_buffer_index] = STOP_BYTE;
    ++send_buffer_index;
    send_packet(sockfd, send_buffer_index);
    send_buffer_index = 0;
    close(sockfd);
}

void
linux_ip_socket_private_data::find_addresses(addrinfo** target, const char* address, const unsigned int port)
{
    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    const std::string remote_port = std::to_string(port);

    const int getaddrinfo_result = getaddrinfo(address, remote_port.c_str(), &hints, target);

    if(getaddrinfo_result != 0) {
        std::cerr << "getaddrinfo returned an error: " << gai_strerror(getaddrinfo_result) << std::endl;
        std::cerr << "Aborting." << std::endl;
        abort();
    }
}

void
linux_ip_socket_private_data::parse_recv_buffer(const size_t length)
{
    for(size_t i = 0; i < length; ++i) {
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

void
linux_ip_socket_private_data::send_packet(const int sockfd, const size_t buffer_length)
{
    const int send_result = send(sockfd, m_send_buffer, buffer_length, 0);
    if(send_result == -1) {
        std::cerr << "sendto error\n";
        return;
    }
}

int
linux_ip_socket_private_data::connect_to_remote_driver()
{
    addrinfo* address_array = nullptr;

    find_addresses(&address_array, m_ip_remote_device_configuration->address, m_ip_remote_device_configuration->port);

    addrinfo* connect_address = address_array;

    if(connect_address == nullptr) {
        std::cerr << "Cannot find remote address." << std::endl;
        return INVALID_SOCKET_ID;
    }

    const int sockfd = socket(connect_address->ai_family, connect_address->ai_socktype, connect_address->ai_protocol);
    if(sockfd == -1) {
        std::cerr << "socket() returned an error: " << strerror(errno) << std::endl;
        freeaddrinfo(address_array);
        return INVALID_SOCKET_ID;
    }
    const int connect_result = connect(sockfd, connect_address->ai_addr, connect_address->ai_addrlen);
    if(connect_result == -1) {
        std::cerr << "connect() returned an error: " << strerror(errno) << std::endl;
        freeaddrinfo(address_array);
        return INVALID_SOCKET_ID;
    }

    freeaddrinfo(address_array);
    return sockfd;
}

void
linux_ip_socket_private_data::prepare_listen_socket()
{
    addrinfo* address_array = nullptr;
    find_addresses(&address_array, m_ip_device_configuration->address, m_ip_device_configuration->port);

    addrinfo* listen_address = nullptr;
    for(listen_address = address_array; listen_address != nullptr; listen_address = listen_address->ai_next) {
        m_listen_sockfd = socket(listen_address->ai_family, listen_address->ai_socktype, listen_address->ai_protocol);
        if(m_listen_sockfd == -1) {
            std::cerr << "socket() returned an error: " << strerror(errno) << std::endl;
            continue;
        }
        const int bind_result = bind(m_listen_sockfd, listen_address->ai_addr, listen_address->ai_addrlen);
        if(bind_result == -1) {
            std::cerr << "bind() returned an error: " << strerror(errno) << std::endl;
            continue;
        }

        break;
    }

    if(listen_address == nullptr) {
        std::cerr << "Cannot create or bind socket\n";
        abort();
    }

    freeaddrinfo(address_array);

    const int listen_result = listen(m_listen_sockfd, DRIVER_MAX_CONNECTIONS);
    if(listen_result == -1) {
        std::cerr << "Cannot listen on socket\n";
        abort();
    }
}

void
linux_ip_socket_private_data::initialize_packet_parser()
{
    m_parse_state = STATE_WAIT;
}

void
linux_ip_socket_private_data::accept_connection(pollfd* table, int& table_size)
{
    sockaddr_storage remote_addr;
    socklen_t remote_addr_size = sizeof(sockaddr_storage);
    const int new_sockfd = accept(m_listen_sockfd, (struct sockaddr*)&remote_addr, &remote_addr_size);
    if(new_sockfd == -1) {
        std::cerr << "accept() returned an error: " << std::strerror(errno) << std::endl;
    } else {
        initialize_packet_parser();
        table[table_size].fd = new_sockfd;
        table[table_size].events = POLLIN;
        ++table_size;
    }
}

void
linux_ip_socket_private_data::read_data_or_disconnect(const int table_index, pollfd* table, int& table_size)
{
    const int recv_result = recv(table[table_index].fd, m_recv_buffer, DRIVER_RECV_BUFFER_SIZE, 0);
    if(recv_result == -1) {
        std::cerr << "recv() returned an error: " << std::strerror(errno) << std::endl;
    } else if(recv_result == 0) {
        --table_size;
        close(table[table_size].fd);
    } else {
        parse_recv_buffer(recv_result);
    }
}

namespace taste {

void
LinuxIpSocketPoll(void* private_data)
{
    linux_ip_socket_private_data* self = reinterpret_cast<linux_ip_socket_private_data*>(private_data);
    self->driver_poll();
}

void
LinuxIpSocketSend(void* private_data, const uint8_t* const data, const size_t length)
{
    linux_ip_socket_private_data* self = reinterpret_cast<linux_ip_socket_private_data*>(private_data);
    self->driver_send(data, length);
}

void
LinuxIpSocketInit(void* private_data,
                  const enum SystemBus bus_id,
                  const enum SystemDevice device_id,
                  const Socket_IP_Conf_T* const device_configuration,
                  const Socket_IP_Conf_T* const remote_device_configuration)
{
    linux_ip_socket_private_data* self = reinterpret_cast<linux_ip_socket_private_data*>(private_data);
    self->driver_init(bus_id, device_id, device_configuration, remote_device_configuration);
}
} // namespace taste
