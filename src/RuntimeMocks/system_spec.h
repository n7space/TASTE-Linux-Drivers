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

#ifndef SYSTEM_SPEC_H
#define SYSTEM_SPEC_H

enum RemoteInterface
{
    INTERFACE_INVALID_ID,
    INTERFACE_MAX_ID,
};

enum SystemPartition
{
    INVALID_PARTITION,
    X86_PARTITION,
};

enum SystemBus
{
    BUS_INVALID_ID,
};

#define SYSTEM_BUSES_NUMBER (0 + 1)

extern enum SystemBus port_to_bus_map[];
extern enum RemoteInterface bus_to_port_map[];

enum SystemDevice
{
    DEVICE_INVALID_ID,
};

#define SYSTEM_DEVICE_NUMBER (0 + 1)

extern enum SystemBus device_to_bus_map[SYSTEM_DEVICE_NUMBER];
extern void* device_configurations[SYSTEM_DEVICE_NUMBER];

#endif
