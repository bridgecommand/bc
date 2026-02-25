
cmake_minimum_required(VERSION 3.10)
project(bridgecommand-somos)

set(CMAKE_CXX_STANDARD 17)
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
include_directories("/usr/include/eigen3/")
include_directories("/usr/include/SDL2/")


find_package(PkgConfig)
pkg_check_modules(JSONCPP jsoncpp)
include_directories(${JSONCPP_INCLUDE_DIRS})

pkg_search_module(LIBEVDEV REQUIRED libevdev)
include_directories(${LIBEVDEV_INCLUDE_DIRS})

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
find_package(SDL2 REQUIRED)

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
	${JSONCPP_LIBRARIES}
	SDL2
	${LIBEVDEV_LIBRARIES}
    )
