
include_directories(${CMAKE_SOURCE_DIR}/libs/asio/include/)

add_executable(aegirstyra-rp
    main.cpp
    ../IniFile.cpp
    ../Lang.cpp
    ../Utilities.cpp
    ../HeadingIndicator.cpp
    ControllerModel.cpp
    EventReceiver.cpp
    GUI.cpp
    Network.cpp
)

set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads REQUIRED)

target_link_libraries(aegirstyra-rp
    bc-asio
    enet
    bc-irrlicht
    Threads::Threads
)
