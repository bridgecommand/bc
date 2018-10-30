
add_library(bc-enet
    callbacks.c
    compress.c
    host.c
    list.c
    packet.c
    peer.c
    protocol.c
    unix.c
    win32.c
)

target_include_directories(bc-enet PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/enet-1.3.11/include"
)

if (MSVC)
    target_link_libraries(bc-enet
        winmm
        Ws2_32
    )
endif (MSVC)
