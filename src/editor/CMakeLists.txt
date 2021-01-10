
add_executable(bridgecommand-ed
    main.cpp
    ../IniFile.cpp
    ../Lang.cpp
    ../Utilities.cpp
    ControllerModel.cpp
    EventReceiver.cpp
    GUI.cpp
    StartupEventReceiver.cpp
)


target_link_libraries(bridgecommand-ed
    bc-irrlicht
)
