add_executable(LinuxApp)
target_sources(LinuxApp
 PRIVATE main.cpp)

target_include_directories(LinuxApp
  PRIVATE ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/RuntimeMocks
  )

target_link_libraries(LinuxApp
  PRIVATE     common_build_options
			  TASTE::LinuxSerialCcsds
              LinuxRuntime
              pthread)

add_format_target(LinuxApp)
