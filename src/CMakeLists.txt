add_executable(ping-cli
    main.cc
    ping_handler.cc
    signal_handler.cc
    ping_cli.cc
    checksum.cc
    )

target_include_directories(ping-cli PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)

install(TARGETS ping-cli RUNTIME DESTINATION bin)
