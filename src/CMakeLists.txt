enable_testing()

add_executable(ping-cli
    main.cc
    ping_handler.cc
    signal_handler.cc
    ping_cli.cc
    )

target_include_directories(ping-cli PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)

add_test(
  NAME
    ping-cli
  COMMAND
    ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/demo
  )

install(TARGETS ping-cli RUNTIME DESTINATION bin)
