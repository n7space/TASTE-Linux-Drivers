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

typedef uint64_t asn1SccUint64;
typedef asn1SccUint64 asn1SccUint;
typedef bool flag;

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

typedef enum
{
    Serial_CCSDS_Linux_Baudrate_T_b9600 = 0,
    Serial_CCSDS_Linux_Baudrate_T_b19200 = 1,
    Serial_CCSDS_Linux_Baudrate_T_b38400 = 2,
    Serial_CCSDS_Linux_Baudrate_T_b57600 = 3,
    Serial_CCSDS_Linux_Baudrate_T_b115200 = 4,
    Serial_CCSDS_Linux_Baudrate_T_b230400 = 5
} Serial_CCSDS_Linux_Baudrate_T;

typedef enum
{
    Serial_CCSDS_Linux_Parity_T_even = 0,
    Serial_CCSDS_Linux_Parity_T_odd = 1
} Serial_CCSDS_Linux_Parity_T;

typedef char Serial_CCSDS_Linux_Conf_T_devname[25];
typedef asn1SccUint Serial_CCSDS_Linux_Conf_T_bits;

typedef flag Serial_CCSDS_Linux_Conf_T_use_paritybit;

typedef struct
{
    Serial_CCSDS_Linux_Conf_T_devname devname;
    Serial_CCSDS_Linux_Baudrate_T speed;
    Serial_CCSDS_Linux_Parity_T parity;
    Serial_CCSDS_Linux_Conf_T_bits bits;
    Serial_CCSDS_Linux_Conf_T_use_paritybit use_paritybit;

    struct
    {
        unsigned int speed : 1;
        unsigned int parity : 1;
        unsigned int bits : 1;
        unsigned int use_paritybit : 1;
    } exist;

} Serial_CCSDS_Linux_Conf_T;

#endif
