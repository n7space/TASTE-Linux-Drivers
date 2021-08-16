add_library(IpSocketLinux STATIC)
target_sources(IpSocketLinux
    PRIVATE     ip_socket_linux.cpp
    PUBLIC      ip_socket_linux.h)

target_link_libraries(IpSocketLinux
  PRIVATE     common_build_options)

add_format_target(IpSocketLinux)

add_library(TASTE::IpSocketLinux ALIAS IpSocketLinux)
