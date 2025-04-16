
include_directories(${CMAKE_SOURCE_DIR}/libs/asio/include/)

file(GLOB SRC_FILES
    "*.cpp"
    "../IniFile.cpp"
    "../Lang.cpp"
    "../Utilities.cpp"
    "../HeadingIndicator.cpp"
)
add_executable(bridgecommand-rp ${SRC_FILES})

set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads REQUIRED)

target_link_libraries(bridgecommand-rp
    bc-asio
    enet
    Irrlicht
    Threads::Threads
)
