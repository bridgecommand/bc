
add_executable(bridgecommand-ini
    main.cpp
    ../IniFile.cpp
    ../Lang.cpp
    ../Utilities.cpp
)

if (WIN32)
target_link_libraries(bridgecommand-ini
    irrlicht
)
else ()
target_link_libraries(bridgecommand-ini
    bc-irrlicht
)
endif ()
