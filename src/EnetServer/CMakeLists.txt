file(GLOB SRC_FILES
    "*.cpp"
    "../IniFile.cpp"
    "../Utilities.cpp"
)
add_executable(bridgecommand-es ${SRC_FILES})

set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads REQUIRED)

target_link_libraries(bridgecommand-es
    enet
    Irrlicht
    Threads::Threads
)

