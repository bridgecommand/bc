
add_executable(bridgecommand-mh
    main.cpp
    ../IniFile.cpp
    ../Lang.cpp
    ../Utilities.cpp
    ../ScenarioDataStructure.cpp
    Network.cpp
    ScenarioChoice.cpp
    ShipPositions.cpp
    StartupEventReceiver.cpp
)

target_link_libraries(bridgecommand-mh
    enet
    bc-irrlicht
)
