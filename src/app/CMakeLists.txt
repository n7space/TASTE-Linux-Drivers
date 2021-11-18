add_executable(LinuxApp)
target_sources(LinuxApp
  PRIVATE    main.cpp)

target_include_directories(LinuxApp
  PRIVATE   ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/TASTE-Linux-Runtime/src)

target_link_libraries(LinuxApp
  PRIVATE   common_build_options
            TASTE::LinuxSerialCcsds
            TASTE::RuntimeMocks
            LinuxRuntime
            pthread
            TASTE::Escaper
  PUBLIC    TASTE::Broker)

add_format_target(LinuxApp)
