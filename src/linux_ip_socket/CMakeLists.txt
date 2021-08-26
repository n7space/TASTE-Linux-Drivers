add_library(LinuxIpSocket STATIC)
target_sources(LinuxIpSocket
 PRIVATE linux_ip_socket.cpp
 PUBLIC  linux_ip_socket.h)

#target_link_libraries(LinuxIpSocket
#  PRIVATE     common_build_options)

add_format_target(LinuxIpSocket)

#add_library(TASTE::LinuxIpSocket ALIAS LinuxIpSocket)
