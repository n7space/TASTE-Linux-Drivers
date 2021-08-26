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

#include <thread.h>

#include <drivers_config.h>

extern "C" {
#include <Broker.h>
}

linux_ip_socket_private_data::linux_ip_socket_private_data()
    : m_thread(1, 65536) {}

namespace taste {

void LinuxIpSocketPoll(void *private_data) {
  linux_ip_socket_private_data *self =
      reinterpret_cast<linux_ip_socket_private_data *>(private_data);
  std::cout << "Creating driver thread\n" << std::endl;
  struct addrinfo hints;
  struct addrinfo *servinfo;
  struct addrinfo *p;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  std::string primary_host = self->m_ip_device_configuration->address;
  std::string primary_port =
      std::to_string(self->m_ip_device_configuration->port);

  std::cout << "Bind at " << primary_host << " : " << primary_port << std::endl;
  int rc = getaddrinfo(primary_host.c_str(), primary_port.c_str(), &hints,
                       &servinfo);
  if (rc != 0) {
    std::cerr << "getaddrinfo\n";
    exit(3);
  }

  for (p = servinfo; p != NULL; p = p->ai_next) {
    self->ip_sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (self->ip_sockfd == -1) {
      std::cerr << "socket\n";
      continue;
    }
    rc = bind(self->ip_sockfd, p->ai_addr, p->ai_addrlen);
    if (rc == -1) {
      std::cerr << "bind\n";
      continue;
    }

    break;
  }

  if (p == NULL) {
    std::cerr << "Cannot create or bind socket\n";
    exit(3);
  }

  freeaddrinfo(servinfo);

  struct sockaddr_storage their_addr;

  static uint8_t buffer[BROKER_BUFFER_SIZE];
  while (true) {
    socklen_t addr_len;
    rc = recvfrom(self->ip_sockfd, buffer, BROKER_BUFFER_SIZE, 0,
                  (struct sockaddr *)&their_addr, &addr_len);
    if (rc == -1) {
      std::cerr << "recvfrom";
    } else {
      std::cout << "received " << rc << " bytes" << std::endl;
      Broker_receive_packet(buffer, rc);
    }
  }
}

void LinuxIpSocketSend(void *private_data, uint8_t *data, size_t length) {
  linux_ip_socket_private_data *self =
      reinterpret_cast<linux_ip_socket_private_data *>(private_data);

  struct addrinfo hints;
  struct addrinfo *servinfo;
  struct addrinfo *p;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  std::string remote_host = self->m_ip_remote_device_configuration->address;
  std::string remote_port =
      std::to_string(self->m_ip_remote_device_configuration->port);

  std::cout << "Sending to " << remote_host << " : " << remote_port
            << std::endl;
  int rc =
      getaddrinfo(remote_host.c_str(), remote_port.c_str(), &hints, &servinfo);

  if (rc != 0) {
    std::cerr << "getaddrinfo\n";
    exit(3);
  }

  for (p = servinfo; p != NULL; p = p->ai_next) {
    break;
  }

  if (p == NULL) {
    std::cerr << "Cannot create or bind socket\n";
    exit(3);
  }

  freeaddrinfo(servinfo);

  std::cout << "Sending " << length << " bytes\n";
  rc = sendto(self->ip_sockfd, data, length, 0, p->ai_addr, p->ai_addrlen);
  if (rc == -1) {
    std::cerr << "sendto\n";
  } else {
    std::cout << rc << " bytes sent" << std::endl;
  }
}

void LinuxIpSocketInit(
    void *private_data, enum SystemBus bus_id, enum SystemDevice device_id,
    const Socket_IP_Conf_T *const device_configuration,
    const Socket_IP_Conf_T *const remote_device_configuration) {

  linux_ip_socket_private_data *self =
      reinterpret_cast<linux_ip_socket_private_data *>(private_data);
  self->m_ip_device_bus_id = bus_id;
  self->m_ip_device_id = device_id;
  self->m_ip_device_configuration = device_configuration;
  self->m_ip_remote_device_configuration = remote_device_configuration;
  self->m_thread.start(&LinuxIpSocketPoll, private_data);
}
} // namespace taste
