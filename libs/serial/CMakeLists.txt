
add_library(bc-serial
    src/impl/list_ports/list_ports_linux.cc
    src/impl/list_ports/list_ports_osx.cc
    src/impl/list_ports/list_ports_win.cc
    src/impl/unix.cc
    src/impl/win.cc
    src/serial.cc
)

target_include_directories(bc-serial PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
