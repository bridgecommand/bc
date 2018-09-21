
add_executable(bridgecommand-mh EXCLUDE_FROM_ALL
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
    bc-enet
    bc-irrlicht
)
