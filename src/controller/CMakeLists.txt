
add_subdirectory(libs/aisparser)

file(GLOB SRC_FILES
    "*.cpp"
    "../IniFile.cpp"
    "../Lang.cpp"
    "../Utilities.cpp"
    "../ScrollDial.cpp"
)
add_executable(bridgecommand-mc ${SRC_FILES})

set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads REQUIRED)

target_link_libraries(bridgecommand-mc
    bc-asio
    enet
    Irrlicht
    bc-aisparser
    Threads::Threads
)
