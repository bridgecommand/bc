file(GLOB SRC_FILES
    "*.cpp"
    "../IniFile.cpp"
    "../Lang.cpp"
    "../Utilities.cpp"
    "../ScenarioDataStructure.cpp"
)

add_executable(bridgecommand-mh ${SRC_FILES})

target_link_libraries(bridgecommand-mh
    enet
    Irrlicht
)

