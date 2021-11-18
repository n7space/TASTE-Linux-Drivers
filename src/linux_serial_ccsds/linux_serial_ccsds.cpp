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

#include <fcntl.h>
#include <cassert>
#include <termios.h>
#include <unistd.h>
#include <iostream>

linux_serial_ccsds_private_data::linux_serial_ccsds_private_data()
    : m_thread(DRIVER_THREAD_PRIORITY, DRIVER_THREAD_STACK_SIZE)
{
}

linux_serial_ccsds_private_data::~linux_serial_ccsds_private_data()
{
    if(serialFd != -1) {
        close(serialFd);
    } else {
    }
}

inline void
linux_serial_ccsds_private_data::driver_init_baudrate(const Serial_CCSDS_Linux_Conf_T* const device, int* cflags)
{
    switch(device->speed) {
        case b9600:
            *cflags |= B9600;
            break;
        case b19200:
            *cflags |= B19200;
            break;
        case b38400:
            *cflags |= B38400;
            break;
        case b57600:
            *cflags |= B57600;
            break;
        case b115200:
            *cflags |= B115200;
            break;
        case b230400:
            *cflags |= B230400;
            break;
        default:
            *cflags |= B115200;
            std::cerr << "Not supported baudrate value, defaulting to 115200\r\n";
    }
}

inline void
linux_serial_ccsds_private_data::driver_init_character_size(const Serial_CCSDS_Linux_Conf_T* const device, int* cflags)
{
    switch(device->bits) {
        case 5:
            *cflags |= CS5;
            break;
        case 6:
            *cflags |= CS6;
            break;
        case 7:
            *cflags |= CS7;
            break;
        case 8:
            *cflags |= CS8;
            break;
        default:
            *cflags |= CS8;
            std::cerr << "Not supported character size, defaulting to 8 bits\r\n";
    }
}

inline void
linux_serial_ccsds_private_data::driver_init_parity(const Serial_CCSDS_Linux_Conf_T* const device, int* cflags)
{
    if(device->use_paritybit) {
        *cflags |= PARENB;
        switch(device->parity) {
            case odd:
                *cflags |= PARODD;
                break;
            case even:
                *cflags &= ~PARODD;
                break;
            default:
                *cflags &= ~PARENB;
                std::cerr << "Not supported parity type deaulting to no parity";
        }
    } else {
        *cflags &= ~PARENB;
    }
}

void
linux_serial_ccsds_private_data::driver_init(const SystemBus bus_id,
                                             const SystemDevice device_id,
                                             const Serial_CCSDS_Linux_Conf_T* const device_configuration,
                                             const Serial_CCSDS_Linux_Conf_T* const remote_device_configuration)
{
    m_serial_device_bus_id = bus_id;
    m_serial_device_id = device_id;
    m_serial_device_configuration = device_configuration;
    m_serial_remote_device_configuration = remote_device_configuration;

    /// Open UART device
    /**
     * Access mode		O_RDWR - read write access mode
     * Blocking mode	O_NDELAY - non blocking mode
     * 					O_NOCTTY - pathname will refer to tty
     */
    serialFd = open(device_configuration->devname, O_RDWR | O_NOCTTY); //| O_NDELAY);
    if(serialFd == -1) {
        std::cerr << "Error while opening a file \n\r";
        exit(EXIT_FAILURE);
    }

    /// Configure UART
    struct termios options;

    int cflags = 0;

    driver_init_baudrate(device_configuration, &cflags);
    driver_init_character_size(device_configuration, &cflags);
    driver_init_parity(device_configuration, &cflags);

    tcgetattr(serialFd, &options);
    options.c_cflag = cflags | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(serialFd, TCIFLUSH);
    tcsetattr(serialFd, TCSANOW, &options);

    m_thread.start(&taste::LinuxSerialCcsdsPoll, this);
}

void
linux_serial_ccsds_private_data::driver_poll()
{
    ssize_t numOfRecvBytes{ 0 };
    while(1) {
        if(serialFd != -1) {
            numOfRecvBytes = read(serialFd, m_recv_buffer, DRIVER_RECV_BUFFER_SIZE);
            if(numOfRecvBytes > 0) {
                printf("\nReceived:\n");
                for(ssize_t i = 0; i < numOfRecvBytes; i++) {
                    putchar(m_recv_buffer[i]);
                }
                // todo process character here using extracted escaping module
            } else {
                std::cerr << "Error while polling. Cannot read.\n\r";
                exit(EXIT_FAILURE);
            }
        } else {
            std::cerr << "Error while polling. Wrong file descriptor\n\r";
            exit(EXIT_FAILURE);
        }
    }
}

void
linux_serial_ccsds_private_data::driver_send(const uint8_t* const data, const size_t length)
{
    if(serialFd != -1) {
        Escaper_init_encode(&escaper);
        size_t index = 0;
        size_t packetLength = 0;

        while(!Escaper_encode_packet(&escaper, data, length, &index, &packetLength)) {
            int count = write(serialFd, escaper.m_send_packet_buffer, packetLength);
            if(count < 0) {
                std::cerr << "Serial write error\n\r";
            }
        }

    } else {
        std::cerr << "Error while sending. Wrong file descriptor\n\r";
        exit(EXIT_FAILURE);
    }
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
