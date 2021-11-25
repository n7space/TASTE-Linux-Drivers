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

#ifndef PACKET_SENDER_H
#define PACKET_SENDER_H

extern "C"
{
#include <Packetizer.h>
}

class PacketSender
{
  public:
    void Init();

    static void SendThreadMethod1(void* args);
    static void SendThreadMethod2(void* args);

  private:
    static constexpr uint16_t DEVICE1_ID = 0;
    static constexpr uint16_t DEVICE2_ID = 1;

    static constexpr char const* TEXT1{ "Hello\n\r" };
    static constexpr char const* TEXT2{ "Goodbye\n\r" };

    static constexpr size_t DATA_SIZE1 = sizeof(TEXT1);
    static constexpr size_t PACKET_SIZE1 =
            SPACE_PACKET_PRIMARY_HEADER_SIZE + DATA_SIZE1 + SPACE_PACKET_ERROR_CONTROL_SIZE;

    static constexpr size_t DATA_SIZE2 = sizeof(TEXT2);
    static constexpr size_t PACKET_SIZE2 =
            SPACE_PACKET_PRIMARY_HEADER_SIZE + DATA_SIZE2 + SPACE_PACKET_ERROR_CONTROL_SIZE;

  private:
    Packetizer packetizer{};
    static uint8_t packetData1[PACKET_SIZE1];
    static uint8_t packetData2[PACKET_SIZE2];
};

#endif
