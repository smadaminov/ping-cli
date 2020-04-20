find_package(Threads)

add_executable(ping-cli
    main.cc
    ping_handler.cc
    signal_handler.cc
    ping_cli.cc
    checksum.cc
    icmp_recv.cc
    )

target_link_libraries(ping-cli
    ${CMAKE_THREAD_LIBS_INIT}
    )

target_include_directories(ping-cli PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)

install(TARGETS ping-cli RUNTIME DESTINATION bin)
