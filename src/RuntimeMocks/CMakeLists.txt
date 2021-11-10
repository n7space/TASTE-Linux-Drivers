add_library(RuntimeMocks STATIC)
target_sources(RuntimeMocks
  PRIVATE      system_spec.c
  PUBLIC       system_spec.h
               Broker.h
               drivers_config.h
			   Thread.h
			   request_size.h)

target_include_directories(RuntimeMocks
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(RuntimeMocks
  PRIVATE common_build_options)

add_format_target(RuntimeMocks)

add_library(TASTE::RuntimeMocks ALIAS RuntimeMocks)
