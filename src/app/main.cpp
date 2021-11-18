#include "linux_serial_ccsds/linux_serial_ccsds.h"
#include <cstdio>
#include <unistd.h>

static constexpr uint8_t TEXT1[]{ "Hello\n\r" };
static constexpr uint8_t TEXT2[]{ "Goodbye\n\r" };

static constexpr int SEND_THREAD_PRIORITY1 = 2;
static constexpr int SEND_THREAD_STACK_SIZE1 = 65536;

static constexpr int SEND_THREAD_PRIORITY2 = 1;
static constexpr int SEND_THREAD_STACK_SIZE2 = 65536;

linux_serial_ccsds_private_data serial1{};
linux_serial_ccsds_private_data serial2{};

void sendThreadMethod1(void* args);
void sendThreadMethod2(void* args);

int
main()
{
    printf("\n\rDemo app started\n\r");

    taste::Thread sendThread1{ SEND_THREAD_PRIORITY1, SEND_THREAD_STACK_SIZE1 };
    taste::Thread sendThread2{ SEND_THREAD_PRIORITY2, SEND_THREAD_STACK_SIZE2 };

    Serial_CCSDS_Linux_Conf_T device1{
        "/dev/ttyVCOM0", b115200, odd, 8, 0,
    };
    Serial_CCSDS_Linux_Conf_T device2{
        "/dev/ttyVCOM1", b115200, odd, 8, 0,
    };

    serial1.driver_init(BUS_INVALID_ID, DEVICE_INVALID_ID, &device1, nullptr);
    serial2.driver_init(BUS_INVALID_ID, DEVICE_INVALID_ID, &device2, nullptr);

    int someargs1;
    int someargs2;
    sendThread1.start(&sendThreadMethod1, &someargs1);
    // sendThread2.start(&sendThreadMethod2, &someargs2);

    sendThread1.join();
    // sendThread2.join();

    printf("\n\r The end");
    return 0;
}

void
sendThreadMethod1(void* args)
{
    (void*)args;
    static constexpr int numOfExe{ 10 };
    for(uint16_t i = 0; i < numOfExe; i++) {
        printf("\n\rSender1:");
        serial1.driver_send(TEXT1, sizeof(TEXT1));
        serial2.driver_send(TEXT2, sizeof(TEXT2));
        usleep(500000);
    }
}

void
sendThreadMethod2(void* args)
{
    (void*)args;
    static constexpr int numOfExe{ 20 };
    for(uint16_t i = 0; i < numOfExe; i++) {
        printf("\n\rSender2");
        serial2.driver_send(TEXT2, sizeof(TEXT2));
        usleep(250000);
    }
}
