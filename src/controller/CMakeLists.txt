
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
)

set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads REQUIRED)

target_link_libraries(bridgecommand-mc
    bc-asio
    enet
    bc-irrlicht
    bc-aisparser
    Threads::Threads
)
