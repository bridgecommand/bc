
add_executable(bridgecommand-es
    main.cpp
    com.cpp
    comstatus.cpp
    fsm.cpp
    message.cpp
    thread.cpp
)

set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads REQUIRED)


target_link_libraries(bridgecommand-es
    enet
    Threads::Threads
)

