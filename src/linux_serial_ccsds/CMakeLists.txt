add_library(LinuxSerialCcsds STATIC)
target_sources(LinuxSerialCcsds
  PRIVATE   linux_serial_ccsds.cc
  PUBLIC    linux_serial_ccsds.h)

target_include_directories(LinuxSerialCcsds
  PRIVATE   ${CMAKE_CURRENT_SOURCE_DIR}/../
            ${CMAKE_CURRENT_SOURCE_DIR}/../../TASTE-Linux-Runtime/src
  PUBLIC    ${CMAKE_CURRENT_SOURCE_DIR}/../RuntimeMocks)

target_link_libraries(LinuxSerialCcsds
  PRIVATE   common_build_options
  PUBLIC    TASTE::Broker
            TASTE::Escaper)

add_format_target(LinuxSerialCcsds)

add_library(TASTE::LinuxSerialCcsds ALIAS LinuxSerialCcsds)
