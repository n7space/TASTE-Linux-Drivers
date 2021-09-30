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
