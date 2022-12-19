/**@file
 * This file is part of the TASTE C++ Linux Runtime.
 *
 * @copyright 2022 ESA / Maxime Perrotin
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

#include "linux_udp.h"

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

linux_udp_private_data::linux_udp_private_data()
    : m_thread(DRIVER_THREAD_PRIORITY, DRIVER_THREAD_STACK_SIZE)
{
    Escaper_init(&escaper,
                 m_encoded_packet_buffer,
                 ENCODED_PACKET_BUFFER_SIZE,
                 m_decoded_packet_buffer,
                 DECODED_PACKET_BUFFER_SIZE);
}

void
linux_udp_private_data::driver_init(const SystemBus bus_id,
                                          const SystemDevice device_id,
                                          const Socket_IP_Conf_T* const device_configuration,
                                          const Socket_IP_Conf_T* const remote_device_configuration)
{
    m_ip_device_bus_id = bus_id;
    m_ip_device_id = device_id;
    m_ip_device_configuration = device_configuration;
    m_ip_remote_device_configuration = remote_device_configuration;
    m_thread.start(&taste::LinuxUdpPoll, this);
}

void
linux_udp_private_data::driver_poll()
{
    prepare_listen_socket(); // create the socket file descriptor

    // only one active connection
    const int connected_descriptors_table_size = 1;
    struct pollfd connected_descriptors_table[connected_descriptors_table_size];

    while(true) {
        Escaper_start_decoder(&escaper);
        read_data_or_disconnect(connected_descriptors_table);
    }
}

void
linux_udp_private_data::driver_send(const uint8_t* const data, const size_t length)
{
    static int sockfd = INVALID_SOCKET_ID;

    if(INVALID_SOCKET_ID == sockfd) {
        sockfd = connect_to_remote_driver();
        if (INVALID_SOCKET_ID == sockfd) return;
    }
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(m_ip_remote_device_configuration->port);
    servaddr.sin_addr.s_addr = inet_addr(m_ip_remote_device_configuration->address);

    size_t index = 0;

    Escaper_start_encoder(&escaper);
    while(index < length) {
        size_t packet_length = Escaper_encode_packet(&escaper, data, length, &index);
        sendto(sockfd, m_encoded_packet_buffer, packet_length, MSG_CONFIRM,
                (const struct sockaddr *) &servaddr, sizeof(servaddr));
    }
}

int
linux_udp_private_data::connect_to_remote_driver()
{
   const int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

   if(sockfd == INVALID_SOCKET_ID) {
       std::cerr << "socket() returned an error: " << strerror(errno) << std::endl;
       return INVALID_SOCKET_ID;
    }
   else {
       return sockfd;
   }
}

void
linux_udp_private_data::prepare_listen_socket()
{
    struct sockaddr_in servaddr;
    // Creating UDP socket file descriptor
    if ((m_listen_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        std::cerr << "socket() returned an error: " << strerror(errno) << std::endl;
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(m_ip_device_configuration->address);
    servaddr.sin_port = htons(m_ip_device_configuration->port);
    if (bind(m_listen_sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        std::cerr << "bind() returned an error: " << strerror(errno) << std::endl;
    }
}


bool
linux_udp_private_data::read_data_or_disconnect(pollfd* table)
{
    struct sockaddr_in cliaddr;
    memset(&cliaddr, 0, sizeof(cliaddr));
    unsigned int len = sizeof(cliaddr);

    const int recv_result = recvfrom(m_listen_sockfd, m_recv_buffer, DRIVER_RECV_BUFFER_SIZE, MSG_WAITALL, (struct sockaddr *) &cliaddr, &len);
    if(recv_result == RECV_ERROR) {
        std::cerr << "recv() returned an error: " << std::strerror(errno) << std::endl;
        close(table[0].fd);
        table[0].fd = 0;
        table[0].events = 0;
        return false;
    } else if(recv_result == RECV_CONNECTION_SHUTDOWN) {
        close(table[0].fd);
        table[0].fd = 0;
        table[0].events = 0;
        return false;
    } else {
        const size_t length = static_cast<const size_t>(recv_result);
        Escaper_decode_packet(&escaper, m_ip_device_bus_id, m_recv_buffer, length, Broker_receive_packet);
        return true;
    }
}

namespace taste {

void
LinuxUdpPoll(void* private_data)
{
    linux_udp_private_data* self = reinterpret_cast<linux_udp_private_data*>(private_data);
    self->driver_poll();
}

void
LinuxUdpSend(void* private_data, const uint8_t* const data, const size_t length)
{
    linux_udp_private_data* self = reinterpret_cast<linux_udp_private_data*>(private_data);
    self->driver_send(data, length);
}

void
LinuxUdpInit(void* private_data,
                  const enum SystemBus bus_id,
                  const enum SystemDevice device_id,
                  const Socket_IP_Conf_T* const device_configuration,
                  const Socket_IP_Conf_T* const remote_device_configuration)
{
    linux_udp_private_data* self = reinterpret_cast<linux_udp_private_data*>(private_data);
    self->driver_init(bus_id, device_id, device_configuration, remote_device_configuration);
}
} // namespace taste
