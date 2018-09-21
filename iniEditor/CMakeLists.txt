
add_executable(bridgecommand-ini EXCLUDE_FROM_ALL
    main.cpp
    ../IniFile.cpp
    ../Lang.cpp
    ../Utilities.cpp
)

target_link_libraries(bridgecommand-ini
    bc-irrlicht
)
