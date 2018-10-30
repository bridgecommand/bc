
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

target_link_libraries(bridgecommand-rp
    bc-enet
    bc-irrlicht
)
