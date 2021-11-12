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

#ifndef DRIVERS_CONFIG_H
#define DRIVERS_CONFIG_H

#include <cstdint>

typedef uint32_t Port_T;

typedef enum
{
    ipv4 = 0,
    ipv6 = 1
} Version_T;

#define Version_T_ipv4 ipv4
#define Version_T_ipv6 ipv6

typedef char Socket_IP_Conf_T_devname[21];
typedef char Socket_IP_Conf_T_address[41];

typedef struct
{
    Socket_IP_Conf_T_devname devname;
    Socket_IP_Conf_T_address address;
    Version_T version;
    Port_T port;

    struct
    {
        unsigned int version : 1;
    } exist;

} Socket_IP_Conf_T;

#endif
