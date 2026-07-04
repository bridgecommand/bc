
include_directories(${CMAKE_SOURCE_DIR}/libs/asio/include/)

add_executable(bridgecommand-rp
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

if (WIN32)
target_link_libraries(bridgecommand-rp
    bc-asio
    enet64
    irrlicht
    Threads::Threads
    winmm
    ws2_32
)
else ()
target_link_libraries(bridgecommand-rp
    bc-asio
    enet
    bc-irrlicht
    Threads::Threads
)
endif ()

