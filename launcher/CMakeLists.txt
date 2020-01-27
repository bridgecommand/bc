
add_executable(bridgecommand
    main.cpp
    ../IniFile.cpp
    ../Lang.cpp
    ../Utilities.cpp
)

target_link_libraries(bridgecommand
    bc-irrlicht
)
