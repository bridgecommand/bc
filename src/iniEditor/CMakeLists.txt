
add_executable(aegirstyra-ini
    main.cpp
    ../IniFile.cpp
    ../Lang.cpp
    ../Utilities.cpp
)

target_link_libraries(aegirstyra-ini
    bc-irrlicht
)
