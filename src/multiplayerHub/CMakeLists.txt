
add_executable(aegirstyra-mh
    main.cpp
    ../IniFile.cpp
    ../Lang.cpp
    ../Utilities.cpp
    ../ScenarioDataStructure.cpp
    LinesData.cpp
    Network.cpp
    ScenarioChoice.cpp
    ShipPositions.cpp
    StartupEventReceiver.cpp
    EventReceiver.cpp
)

target_link_libraries(aegirstyra-mh
    enet
    bc-irrlicht
)
