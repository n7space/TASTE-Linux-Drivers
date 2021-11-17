add_library(LinuxSerialCcsds STATIC)
target_sources(LinuxSerialCcsds
  PRIVATE linux_serial_ccsds.cpp
  PUBLIC  linux_serial_ccsds.h)

target_link_libraries(LinuxSerialCcsds
  PRIVATE     common_build_options
  			  TASTE::RuntimeMocks)

add_format_target(LinuxSerialCcsds)

add_library(TASTE::LinuxSerialCcsds ALIAS LinuxSerialCcsds)
