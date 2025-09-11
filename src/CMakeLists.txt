
cmake_minimum_required(VERSION 3.10)
project(bridgecommand)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(X11 REQUIRED)

add_subdirectory(../lib/asio ../../lib/asio)
add_subdirectory(../lib/serial ../../lib/serial) 

include_directories("../lib/asio/include")
set(IRRLICHT_INCLUDE_DIR "/usr/local/include/irrlicht")
include_directories(${IRRLICHT_INCLUDE_DIR})

add_definitions(-DWITH_SOUND)
# add_definitions(-DWITH_PROFILING)

# optional tools
add_subdirectory(controller)
add_subdirectory(editor)
add_subdirectory(iniEditor)
add_subdirectory(launcher)
add_subdirectory(multiplayerHub)
add_subdirectory(repeater)
add_subdirectory(EnetServer)

set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads REQUIRED)
find_package(OpenXR REQUIRED)

file(GLOB SRC_FILES
    "*.cpp"
)
add_executable(bridgecommand-bc ${SRC_FILES})

target_link_libraries(bridgecommand-bc PRIVATE
        bc-asio
        enet
        bc-serial
        Irrlicht
        Threads::Threads
	${X11_LIBRARIES}
        sndfile
        portaudio
        asound
        GL
        OpenXR::openxr_loader
	netcdf
    )
