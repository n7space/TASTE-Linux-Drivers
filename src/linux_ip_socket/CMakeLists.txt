add_library(LinuxIpSocket STATIC)
target_sources(LinuxIpSocket
  PRIVATE   linux_ip_socket.cpp
  PUBLIC    linux_ip_socket.h)

target_include_directories(LinuxIpSocket
  PRIVATE   ${CMAKE_SOURCE_DIR}/TASTE-Linux-Runtime/src
            ${CMAKE_SOURCE_DIR}/src/RuntimeMocks)

target_link_libraries(LinuxIpSocket
  PRIVATE   common_build_options
            TASTE::RuntimeMocks
  PUBLIC    TASTE::Broker)

add_format_target(LinuxIpSocket)

add_library(TASTE::LinuxIpSocket ALIAS LinuxIpSocket)
