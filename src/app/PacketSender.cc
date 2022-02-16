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

#include <PacketSender.h>

#include "linux_serial_ccsds/linux_serial_ccsds.h"

#include <unistd.h>
#include <cstring>

uint8_t PacketSender::packetData2[PACKET_SIZE2]{};
uint8_t PacketSender::packetData1[PACKET_SIZE1]{};

void
PacketSender::Init()
{
    memcpy(&packetData1[SPACE_PACKET_PRIMARY_HEADER_SIZE], TEXT1, DATA_SIZE1);
    memcpy(&packetData2[SPACE_PACKET_PRIMARY_HEADER_SIZE], TEXT2, DATA_SIZE1);

    Packetizer_init(&packetizer);
    Packetizer_packetize(&packetizer,
                         Packetizer_PacketType_Telemetry,
                         DEVICE1_ID,
                         DEVICE2_ID,
                         packetData1,
                         SPACE_PACKET_PRIMARY_HEADER_SIZE,
                         DATA_SIZE1);

    Packetizer_init(&packetizer);
    Packetizer_packetize(&packetizer,
                         Packetizer_PacketType_Telemetry,
                         DEVICE2_ID,
                         DEVICE1_ID,
                         packetData2,
                         SPACE_PACKET_PRIMARY_HEADER_SIZE,
                         DATA_SIZE2);
}

void
PacketSender::SendThreadMethod1(void* args)
{
    linux_serial_ccsds_private_data* serial;
    serial = reinterpret_cast<linux_serial_ccsds_private_data*>(args);
    static constexpr int numOfExe{ 10 };
    for(uint16_t i = 0; i < numOfExe; i++) {
        serial->driver_send(PacketSender::packetData1, PACKET_SIZE1);
        usleep(250000);
    }
}

void
PacketSender::SendThreadMethod2(void* args)
{
    linux_serial_ccsds_private_data* serial;
    serial = reinterpret_cast<linux_serial_ccsds_private_data*>(args);
    static constexpr int numOfExe{ 20 };
    for(uint16_t i = 0; i < numOfExe; i++) {
        serial->driver_send(PacketSender::packetData2, PACKET_SIZE2);
        usleep(250000);
    }
}
