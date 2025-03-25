
add_executable(aegirstyra-ed
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


target_link_libraries(aegirstyra-ed
    bc-irrlicht
)
