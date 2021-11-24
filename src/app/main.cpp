#include "linux_serial_ccsds/linux_serial_ccsds.h"

#include <cstdio>
#include <unistd.h>
#include <cstring>

#include "Thread.h"

extern "C"
{
#include <Packetizer.h>
}

static constexpr uint8_t TEXT1[]{ "Hello\n\r" };
static constexpr uint8_t TEXT2[]{ "Goodbye\n\r" };

static constexpr int SEND_THREAD_PRIORITY1 = 1;
static constexpr int SEND_THREAD_STACK_SIZE1 = 65536;

static constexpr int SEND_THREAD_PRIORITY2 = 1;
static constexpr int SEND_THREAD_STACK_SIZE2 = 65536;

static constexpr uint16_t DEVICE1_ID = 0;
static constexpr uint16_t DEVICE2_ID = 1;

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
void sendThreadMethod1(void* args);
void sendThreadMethod2(void* args);

Packetizer packetizer{};

static constexpr size_t DATA_SIZE1 = sizeof(TEXT1);
static constexpr size_t PACKET_SIZE1 = SPACE_PACKET_PRIMARY_HEADER_SIZE + DATA_SIZE1 + SPACE_PACKET_ERROR_CONTROL_SIZE;
uint8_t packetData1[PACKET_SIZE1];

static constexpr size_t DATA_SIZE2 = sizeof(TEXT2);
static constexpr size_t PACKET_SIZE2 = SPACE_PACKET_PRIMARY_HEADER_SIZE + DATA_SIZE2 + SPACE_PACKET_ERROR_CONTROL_SIZE;
uint8_t packetData2[PACKET_SIZE2];

int
main()
{
    printf("\n\rDemo app started\n\r");

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

    linux_serial_ccsds_private_data serial1{};
    linux_serial_ccsds_private_data serial2{};

    taste::Thread sendThread1{ SEND_THREAD_PRIORITY1, SEND_THREAD_STACK_SIZE1 };
    taste::Thread sendThread2{ SEND_THREAD_PRIORITY2, SEND_THREAD_STACK_SIZE2 };

    Serial_CCSDS_Linux_Conf_T device1{ "/tmp/ttyVCOM0", b115200, odd, 8, 0, {} };
    Serial_CCSDS_Linux_Conf_T device2{ "/tmp/ttyVCOM1", b115200, odd, 8, 0, {} };

    serial1.driver_init(BUS_INVALID_ID, DEVICE_INVALID_ID, &device1, nullptr);
    serial2.driver_init(BUS_INVALID_ID, DEVICE_INVALID_ID, &device2, nullptr);

    sendThread1.start(&sendThreadMethod1, &serial1);
    sendThread2.start(&sendThreadMethod2, &serial2);

    sendThread1.join();
    sendThread2.join();

    printf("\n\rDemo app finished");
    return 0;
}

void
sendThreadMethod1(void* args)
{
    linux_serial_ccsds_private_data* serial;
    serial = reinterpret_cast<linux_serial_ccsds_private_data*>(args);
    static constexpr int numOfExe{ 10 };
    for(uint16_t i = 0; i < numOfExe; i++) {
        serial->driver_send(packetData1, PACKET_SIZE1);
        usleep(250000);
    }
}

void
sendThreadMethod2(void* args)
{
    linux_serial_ccsds_private_data* serial;
    serial = reinterpret_cast<linux_serial_ccsds_private_data*>(args);
    static constexpr int numOfExe{ 20 };
    for(uint16_t i = 0; i < numOfExe; i++) {
        serial->driver_send(packetData2, PACKET_SIZE2);
        usleep(250000);
    }
}
