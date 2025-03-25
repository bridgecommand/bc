
add_executable(aegirstyra
    main.cpp
    ../IniFile.cpp
    ../Lang.cpp
    ../Utilities.cpp
)

target_link_libraries(aegirstyra
    bc-irrlicht
)
