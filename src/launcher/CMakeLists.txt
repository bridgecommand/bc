
add_executable(bridgecommand
    main.cpp
    ../IniFile.cpp
    ../Lang.cpp
    ../Utilities.cpp
)

if (WIN32)
target_link_libraries(bridgecommand
    irrlicht
)
else ()
target_link_libraries(bridgecommand
    bc-irrlicht
)
endif ()
