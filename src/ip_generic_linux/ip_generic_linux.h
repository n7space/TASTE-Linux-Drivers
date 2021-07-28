#ifndef IP_GENERIC_LINUX_H
#define IP_GENERIC_LINUX_H

#include <system_spec.h>
#include <drivers_config.h>
#include <cstdint>
#include <cstddef>

void generic_ip_driver_linux_poll();

void generic_ip_driver_linux_send(uint8_t* data, size_t length);

void generic_ip_driver_linux_init(enum SystemBus bus_id,
								  enum SystemDevice device_id,
								  const Generic_IP_Conf_T* const device_configuration,
								  const Generic_IP_Conf_T* const remote_device_configuration);

#endif
