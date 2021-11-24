add_executable(LinuxApp)
target_sources(LinuxApp
  PRIVATE   main.cpp
            PacketSender.cpp
  PUBLIC    PacketSender.h)

target_include_directories(LinuxApp
  PRIVATE   ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/TASTE-Linux-Runtime/src)

target_link_libraries(LinuxApp
  PRIVATE   common_build_options
            TASTE::Packetizer
            TASTE::LinuxSerialCcsds
            LinuxRuntime
            Threads::Threads)

add_format_target(LinuxApp)
