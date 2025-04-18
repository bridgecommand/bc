file(GLOB SRC_FILES
    "*.cpp"
    "../IniFile.cpp"
    "../Lang.cpp"
    "../Utilities.cpp"
)
add_executable(bridgecommand ${SRC_FILES})

target_link_libraries(bridgecommand
    Irrlicht
)
