
add_subdirectory(libs/aisparser)

add_executable(aegirstyra-mc
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

target_link_libraries(aegirstyra-mc
    bc-asio
    enet
    bc-irrlicht
    bc-aisparser
    Threads::Threads
)
