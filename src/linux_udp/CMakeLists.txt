add_library(LinuxUdp STATIC)
target_sources(LinuxUdp
  PRIVATE   linux_udp.cc
  PUBLIC    linux_udp.h)

target_include_directories(LinuxUdp
  PRIVATE   ${CMAKE_SOURCE_DIR}/TASTE-Linux-Runtime/src
            ${CMAKE_SOURCE_DIR}/src/RuntimeMocks)

target_link_libraries(LinuxUdp
  PRIVATE   common_build_options
            TASTE::RuntimeMocks
  PUBLIC    TASTE::Broker
            TASTE::Escaper)

add_format_target(LinuxUdp)

add_library(TASTE::LinuxUdp ALIAS LinuxUdp)
