
cmake_minimum_required(VERSION 3.10)
project(bridgecommand)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if (APPLE)
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -mmacosx-version-min=10.7")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.7")
endif (APPLE)

find_package(X11 REQUIRED)

add_subdirectory(libs/asio)
add_subdirectory(libs/serial)

set(IRRLICHT_INCLUDE_DIR "/usr/local/include/irrlicht")
include_directories(${IRRLICHT_INCLUDE_DIR})

add_definitions(-DWITH_SOUND)
# add_definitions(-DWITH_PROFILING)
if (NOT APPLE)
    #add_definitions(-DFOR_DEB)
endif (NOT APPLE)

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
if (NOT APPLE)
find_package(OpenXR REQUIRED)
endif(NOT APPLE)

file(GLOB SRC_FILES
    "*.cpp"
)
add_executable(bridgecommand-bc ${SRC_FILES})

if (APPLE)
    
    add_library(sndfile STATIC IMPORTED) # or STATIC instead of SHARED
    set_target_properties(sndfile PROPERTIES
      IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/libs/libsndfile/lib/libsndfile.a"
      INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/libs/libsndfile/include"
    )
    add_library(portaudio STATIC IMPORTED) # or STATIC instead of SHARED
    set_target_properties(portaudio PROPERTIES
      IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/libs/portaudio/lib/libportaudio.a"
      INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/libs/portaudio/include"
    )

    target_link_libraries(bridgecommand-bc PRIVATE
        bc-asio
        enet
        bc-serial
        bc-irrlicht
        Threads::Threads
        sndfile
        portaudio
        "-framework AudioUnit"
	    "-framework AudioToolbox"
	    "-framework CoreAudio"
    )
else (APPLE)
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
    )
endif (APPLE)
