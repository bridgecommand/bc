
add_executable(bridgecommand-mh
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

if (WIN32)
target_link_libraries(bridgecommand-mh
    enet64
    irrlicht
    winmm
    ws2_32
)
else ()
target_link_libraries(bridgecommand-mh
    enet
    bc-irrlicht
)
endif ()

