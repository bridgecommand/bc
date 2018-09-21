
add_executable(bridgecommand-mc EXCLUDE_FROM_ALL
    ControllerModel.cpp
    EventReceiver.cpp
    GUI.cpp
    main.cpp
    Network.cpp
    ../IniFile.cpp
    ../Lang.cpp
    ../Utilities.cpp
)

target_link_libraries(bridgecommand-mc
    bc-asio
    bc-enet
    bc-irrlicht
)
