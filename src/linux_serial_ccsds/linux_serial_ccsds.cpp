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

#include "linux_serial_ccsds.h"

linux_serial_ccsds_private_data::linux_serial_ccsds_private_data() {}

void
linux_serial_ccsds_private_data::driver_init(const SystemBus bus_id,
                                             const SystemDevice device_id,
                                             const Serial_CCSDS_Linux_Conf_T* const device_configuration,
                                             const Serial_CCSDS_Linux_Conf_T* const remote_device_configuration)
{
}

void
linux_serial_ccsds_private_data::driver_poll()
{
}

void
linux_serial_ccsds_private_data::driver_send(const uint8_t* const data, const size_t length)
{
}

namespace taste {

void
LinuxSerialCcsdsPoll(void* private_data)
{
    linux_serial_ccsds_private_data* self = reinterpret_cast<linux_serial_ccsds_private_data*>(private_data);
    self->driver_poll();
}

void
LinuxSerialCcsdsSend(void* private_data, const uint8_t* const data, const size_t length)
{
    linux_serial_ccsds_private_data* self = reinterpret_cast<linux_serial_ccsds_private_data*>(private_data);
    self->driver_send(data, length);
}

void
LinuxSerialCcsdsInit(void* private_data,
                     const enum SystemBus bus_id,
                     const enum SystemDevice device_id,
                     const Serial_CCSDS_Linux_Conf_T* const device_configuration,
                     const Serial_CCSDS_Linux_Conf_T* const remote_device_configuration)
{
    linux_serial_ccsds_private_data* self = reinterpret_cast<linux_serial_ccsds_private_data*>(private_data);
    self->driver_init(bus_id, device_id, device_configuration, remote_device_configuration);
}
} // namespace taste
