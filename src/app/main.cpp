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
#include "linux_serial_ccsds/linux_serial_ccsds.h"

#include <cstdio>

#include "Thread.h"

#include "PacketSender.h"

static constexpr int SEND_THREAD_PRIORITY1 = 1;
static constexpr int SEND_THREAD_STACK_SIZE1 = 65536;

static constexpr int SEND_THREAD_PRIORITY2 = 1;
static constexpr int SEND_THREAD_STACK_SIZE2 = 65536;

static constexpr size_t NUMBER_OF_DEVICES = 2;

void
device1_interface_deliver_function(const uint8_t* const data, const size_t data_size)
{
    (void)data_size;
    printf("Serial1 received: %s", data);
}

void
device2_interface_deliver_function(const uint8_t* const data, const size_t data_size)
{
    (void)data_size;
    printf("Serial2 received: %s", data);
}

void* bus_to_driver_private_data[NUMBER_OF_DEVICES];
void* bus_to_driver_send_function[NUMBER_OF_DEVICES];
void* interface_to_deliver_function[NUMBER_OF_DEVICES]{ reinterpret_cast<void*>(device1_interface_deliver_function),
                                                        reinterpret_cast<void*>(device2_interface_deliver_function) };

int
main()
{
    printf("\n\rDemo app started\n\r");

    PacketSender packetSender;
    packetSender.Init();

    linux_serial_ccsds_private_data serial1{};
    linux_serial_ccsds_private_data serial2{};

    taste::Thread sendThread1{ SEND_THREAD_PRIORITY1, SEND_THREAD_STACK_SIZE1 };
    taste::Thread sendThread2{ SEND_THREAD_PRIORITY2, SEND_THREAD_STACK_SIZE2 };

    Serial_CCSDS_Linux_Conf_T device1{ "/tmp/ttyVCOM0", b115200, odd, 8, 0, {} };
    Serial_CCSDS_Linux_Conf_T device2{ "/tmp/ttyVCOM1", b115200, odd, 8, 0, {} };

    serial1.driver_init(BUS_INVALID_ID, DEVICE_INVALID_ID, &device1, nullptr);
    serial2.driver_init(BUS_INVALID_ID, DEVICE_INVALID_ID, &device2, nullptr);

    sendThread1.start(&PacketSender::SendThreadMethod1, &serial1);
    sendThread2.start(&PacketSender::SendThreadMethod2, &serial2);

    sendThread1.join();
    sendThread2.join();

    printf("\n\rDemo app finished");
    return 0;
}
