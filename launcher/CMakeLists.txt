
add_executable(bridgecommand EXCLUDE_FROM_ALL
    main.cpp
    ../IniFile.cpp
    ../Lang.cpp
    ../Utilities.cpp
)

target_link_libraries(bridgecommand
    bc-irrlicht
)
