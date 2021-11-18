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

#ifndef LINUX_SERIAL_CCSDS_H
#define LINUX_SERIAL_CCSDS_H

/**
 * @file     linux_serial_ccsds.h
 * @brief    Driver for TASTE with uses UART for communication

 */

#include <cstddef>
#include <cstdint>

#include <Thread.h>
#include <Escaper.h>
#include <system_spec.h>

#include <drivers_config.h>

extern "C"
{
#include <Broker.h>
}

/**
 * @brief Structure for driver internal data.
 *
 * This structure is allocated by runtime and the pointer is passed to all driver functions.
 * The name of this structure shall match driver definition from ocarina_components.aadl
 * and has suffix '_private_data'.
 */
class linux_serial_ccsds_private_data final
{
  public:
    /**
     * @brief  Constructor.
     *
     * Construct empty object, which needs to be initialized using @link linux_serial_ccsds_private_data::init
     * before usage.
     */

    linux_serial_ccsds_private_data();

    /**
     * @brief  Destructor.
     *
     * Destruct created object
     */
    ~linux_serial_ccsds_private_data();

    /**
     * @brief Initialize driver.
     *
     * Driver needs to be initialized before start.
     *
     * @param bus_id         Identifier of the bus, which is used by driver
     * @param device_id      Identifier of the device
     * @param device_configuration Configuration of device
     * @param remote_device_configuration Configuration of remote device
     */
    void driver_init(const SystemBus bus_id,
                     const SystemDevice device_id,
                     const Serial_CCSDS_Linux_Conf_T* const device_configuration,
                     const Serial_CCSDS_Linux_Conf_T* const remote_device_configuration);
    /**
     * @brief Receive data from remote partitions.
     *
     * This function receives data from remote partition and sends it to the Broker.
     */
    void driver_poll();
    /**
     * @brief send data to remote partition.
     *
     * @param data           The Buffer which data to send to connected remote partition
     * @param length         The size of the buffer
     */
    void driver_send(const uint8_t* data, const size_t length);

  private:
    static constexpr int DRIVER_THREAD_PRIORITY = 1;
    static constexpr int DRIVER_THREAD_STACK_SIZE = 65536;
    static constexpr size_t DRIVER_RECV_BUFFER_SIZE = 8 * 1024;

    void driver_init_baudrate(const Serial_CCSDS_Linux_Conf_T* const device, int* cflags);
    void driver_init_character_size(const Serial_CCSDS_Linux_Conf_T* const device, int* cflags);
    void driver_init_parity(const Serial_CCSDS_Linux_Conf_T* const device, int* cflags);

    int serialFd{ -1 };
    enum SystemBus m_serial_device_bus_id;
    enum SystemDevice m_serial_device_id;
    const Serial_CCSDS_Linux_Conf_T* m_serial_device_configuration{};
    const Serial_CCSDS_Linux_Conf_T* m_serial_remote_device_configuration{};
    taste::Thread m_thread;

    uint8_t m_recv_buffer[DRIVER_RECV_BUFFER_SIZE];
    Escaper escaper{};
};

namespace taste {

/**
 * @brief Function which implements receiving data from remote partition.
 *
 * Functions works in separate thread, which is initialized by @link LinuxSerialCcsdsSend
 *
 * @param private_data   Driver private data, allocated by runtime
 */
void LinuxSerialCcsdsPoll(void* private_data);

/**
 * @brief Send data to remote partition.
 *
 * Function is used by runtime.
 *
 * @param private_data   Driver private data, allocated by runtime
 * @param data           The Buffer which data to send to connected remote partition
 * @param length         The size of the buffer
 */
void LinuxSerialCcsdsSend(void* private_data, const uint8_t* const data, const size_t length);

/**
 * @brief Initialize driver.
 *
 * Function is used by runtime to initialize the driver.
 *
 * @param private_data   Driver private data, allocated by runtime
 * @param bus_id         Identifier of the bus, which is used by driver
 * @param device_id      Identifier of the device
 * @param device_configuration Configuration of device
 * @param remote_device_configuration Configuration of remote device
 */
void LinuxSerialCcsdsInit(void* private_data,
                          const SystemBus bus_id,
                          const SystemDevice device_id,
                          const Serial_CCSDS_Linux_Conf_T* const device_configuration,
                          const Serial_CCSDS_Linux_Conf_T* const remote_device_configuration);
} // namespace taste

#endif
