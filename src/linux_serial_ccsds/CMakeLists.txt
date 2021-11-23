add_library(LinuxSerialCcsds STATIC)
target_sources(LinuxSerialCcsds
  PRIVATE   linux_serial_ccsds.cpp
  PUBLIC    linux_serial_ccsds.h)

target_include_directories(LinuxSerialCcsds
  PRIVATE   ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/TASTE-Linux-Runtime/src
  PUBLIC    ${CMAKE_SOURCE_DIR}/src/RuntimeMocks)

target_link_libraries(LinuxSerialCcsds
  PRIVATE   common_build_options
  PUBLIC    TASTE::Broker
            TASTE::Escaper)

add_format_target(LinuxSerialCcsds)

add_library(TASTE::LinuxSerialCcsds ALIAS LinuxSerialCcsds)
