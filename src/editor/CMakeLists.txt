
add_executable(bridgecommand-ed
    main.cpp
    ../IniFile.cpp
    ../Lang.cpp
    ../Utilities.cpp
    ../ScenarioDataStructure.cpp
    ControllerModel.cpp
    EventReceiver.cpp
    ImportExportGUI.cpp
    GUI.cpp
    StartupEventReceiver.cpp
)

if (WIN32)
target_link_libraries(bridgecommand-ed
    irrlicht
)
else ()
target_link_libraries(bridgecommand-ed
    bc-irrlicht
)
endif ()
