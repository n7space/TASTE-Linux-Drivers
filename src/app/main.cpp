#include "linux_serial_ccsds/linux_serial_ccsds.h"
#include <cstdio>
#include <unistd.h>

#include "Thread.h"

static constexpr uint8_t TEXT1[]{ "Hello\n\r" };
static constexpr uint8_t TEXT2[]{ "Goodbye\n\r" };

static constexpr int SEND_THREAD_PRIORITY1 = 1;
static constexpr int SEND_THREAD_STACK_SIZE1 = 65536;

static constexpr int SEND_THREAD_PRIORITY2 = 1;
static constexpr int SEND_THREAD_STACK_SIZE2 = 65536;

void* bus_to_driver_private_data[16];
void* bus_to_driver_send_function[16];
void* interface_to_deliver_function[16];

void sendThreadMethod1(void* args);
void sendThreadMethod2(void* args);

int
main()
{
    printf("\n\rDemo app started\n\r");

    linux_serial_ccsds_private_data serial1{};
    linux_serial_ccsds_private_data serial2{};

    taste::Thread sendThread1{ SEND_THREAD_PRIORITY1, SEND_THREAD_STACK_SIZE1 };
    taste::Thread sendThread2{ SEND_THREAD_PRIORITY2, SEND_THREAD_STACK_SIZE2 };

    Serial_CCSDS_Linux_Conf_T device1{ "/dev/ttyVCOM0", b115200, odd, 8, 0, {} };
    Serial_CCSDS_Linux_Conf_T device2{ "/dev/ttyVCOM1", b115200, odd, 8, 0, {} };

    serial1.driver_init(BUS_INVALID_ID, DEVICE_INVALID_ID, &device1, nullptr);
    serial2.driver_init(BUS_INVALID_ID, DEVICE_INVALID_ID, &device2, nullptr);

    linux_serial_ccsds_private_data* serials[2] = { &serial1, &serial2 };

    sendThread1.start(&sendThreadMethod1, serials);
    sendThread2.start(&sendThreadMethod2, &serial2);

    sendThread1.join();
    sendThread2.join();

    printf("\n\rDemo app finished");
    return 0;
}

void
sendThreadMethod1(void* args)
{
    linux_serial_ccsds_private_data** serial;
    serial = (linux_serial_ccsds_private_data**)args;
    static constexpr int numOfExe{ 10 };
    for(uint16_t i = 0; i < numOfExe; i++) {
        serial[0]->driver_send(TEXT1, sizeof(TEXT1));
        usleep(250000);
    }
}

void
sendThreadMethod2(void* args)
{
    linux_serial_ccsds_private_data* serial;
    serial = (linux_serial_ccsds_private_data*)args;
    static constexpr int numOfExe{ 20 };
    for(uint16_t i = 0; i < numOfExe; i++) {
        serial->driver_send(TEXT2, sizeof(TEXT2));
        usleep(250000);
    }
}
