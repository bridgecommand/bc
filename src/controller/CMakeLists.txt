
add_subdirectory(libs/aisparser)

add_executable(bridgecommand-mc
    ControllerModel.cpp
    EventReceiver.cpp
    GUI.cpp
    main.cpp
    Network.cpp
    AISOverUDP.cpp
    ../IniFile.cpp
    ../Lang.cpp
    ../Utilities.cpp
    ../ScrollDial.cpp
)

set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads REQUIRED)

if (WIN32)
target_link_libraries(bridgecommand-mc
    bc-asio
    enet64
    irrlicht
    bc-aisparser
    Threads::Threads
    winmm
    ws2_32
)
else ()
target_link_libraries(bridgecommand-mc
    bc-asio
    enet
    bc-irrlicht
    bc-aisparser
    Threads::Threads
)
endif ()
