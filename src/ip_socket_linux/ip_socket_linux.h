#ifndef LINUX_IP_SOCKET_H
#define LINUX_IP_SOCKET_H

#include <cstdint>
#include <cstddef>

#include <system_spec.h>

#include <drivers_config.h>

// Linux_Ip_Socket

namespace taste
{
  void LinuxIpSocketPoll();

  void LinuxIpSocketSend(uint8_t * data, size_t length);

  void LinuxIpSocketInit(SystemBus bus_id,
						 SystemDevice device_id,
						 const Socket_IP_Conf_T *const device_configuration,
						 const Socket_IP_Conf_T *const remote_device_configuration);
}

#endif
