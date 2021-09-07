#ifndef LINUX_IP_SOCKET_H
#define LINUX_IP_SOCKET_H

#include <cstddef>
#include <cstdint>

#include <Thread.h>
#include <system_spec.h>

#include <drivers_config.h>

// Linux_Ip_Socket

struct linux_ip_socket_private_data {
  static inline constexpr uint8_t START_BYTE = 0x00;
  static inline constexpr uint8_t STOP_BYTE = 0xFF;
  static inline constexpr uint8_t ESCAPE_BYTE = 0xFE;

  enum State {
    STATE_WAIT,
    STATE_DATA_BYTE,
    STATE_ESCAPE_BYTE,
  };
  linux_ip_socket_private_data();
  int ip_sockfd;
  enum SystemBus m_ip_device_bus_id;
  enum SystemDevice m_ip_device_id;
  const Socket_IP_Conf_T *m_ip_device_configuration;
  const Socket_IP_Conf_T *m_ip_remote_device_configuration;
  taste::Thread m_thread;
};

namespace taste {

void LinuxIpSocketPoll(void *private_data);

void LinuxIpSocketSend(void *private_data, uint8_t *data, size_t length);

void LinuxIpSocketInit(
    void *private_data, SystemBus bus_id, SystemDevice device_id,
    const Socket_IP_Conf_T *const device_configuration,
    const Socket_IP_Conf_T *const remote_device_configuration);
} // namespace taste

#endif
