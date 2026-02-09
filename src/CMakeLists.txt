cmake_minimum_required(VERSION 3.8)
project(bridgecommand)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if (APPLE)
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++17 -stdlib=libc++ -mmacosx-version-min=10.13")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13")
    set(CMAKE_OSX_ARCHITECTURES "arm64")
endif (APPLE)

add_subdirectory(libs/asio)
add_subdirectory(libs/enet-1.3.14)
add_subdirectory(libs/Irrlicht)
add_subdirectory(libs/serial)

include_directories("libs/enet-1.3.14/include")
add_definitions(-DWITH_SOUND)
# add_definitions(-DWITH_PROFILING)
if (NOT APPLE)
    #add_definitions(-DFOR_DEB)
endif (NOT APPLE)

# unit tests
add_subdirectory(tests)

# optional tools
add_subdirectory(chartConverter)
add_subdirectory(controller)
add_subdirectory(editor)
add_subdirectory(iniEditor)
add_subdirectory(launcher)
add_subdirectory(multiplayerHub)
add_subdirectory(repeater)

set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads REQUIRED)
if (NOT APPLE)
find_package(OpenXR REQUIRED)
endif(NOT APPLE)

# Optional: OpenAL for 3D spatial audio
find_package(OpenAL)
if(OPENAL_FOUND)
    message(STATUS "OpenAL found: ${OPENAL_INCLUDE_DIR}")
    add_definitions(-DWITH_OPENAL)
else()
    message(STATUS "OpenAL not found - using PortAudio fallback")
endif()

# Optional: GDAL for S-57 chart integration
find_package(GDAL)
if(GDAL_FOUND)
    message(STATUS "GDAL found: ${GDAL_VERSION}")
    add_definitions(-DWITH_GDAL)
else()
    message(STATUS "GDAL not found - chart integration disabled")
endif()

set(BC_SOURCES
    iprof.cpp
    main.cpp
    AIS.cpp
    Angles.cpp
    Autopilot.cpp
    AzimuthDial.cpp
    Buoy.cpp
    Buoys.cpp
    Camera.cpp
    ControlVisualiser.cpp
    DefaultEventReceiver.cpp
    FFTWave.cpp
    GUIMain.cpp
    GUIRectangle.cpp
    HeadingIndicator.cpp
    IniFile.cpp
    LandLights.cpp
    LandObject.cpp
    LandObjects.cpp
    Lang.cpp
    LegacyPhysicsModel.cpp
    Light.cpp
    Line.cpp
    Lines.cpp
    ManOverboard.cpp
    MMGPhysicsModel.cpp
    MovingWater.cpp
    MyEventReceiver.cpp
    NMEA.cpp
    NavLight.cpp
    Network.cpp
    NetworkPrimary.cpp
    NetworkSecondary.cpp
    NumberToImage.cpp
    OtherShip.cpp
    OtherShips.cpp
    OutlineScrollBar.cpp
    OwnShip.cpp
    RadarCalculation.cpp
    RadarScreen.cpp
    Rain.cpp
    ScenarioChoice.cpp
    ScenarioDataStructure.cpp
    ScrollDial.cpp
    Ship.cpp
    SimulationModel.cpp
    Sky.cpp
    Sound.cpp
    SoundOpenAL.cpp
    StartupEventReceiver.cpp
    Terrain.cpp
    Tide.cpp
    Utilities.cpp
    VRInterface.cpp
	ExitMessage.cpp
    Water.cpp
    BCTerrainSceneNode.cpp
    BCTerrainTriangleSelector.cpp
)

add_executable(bridgecommand-bc
    ${BC_SOURCES}
)

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

    find_library(VORBISENC_LIB vorbisenc HINTS /opt/homebrew/lib)
    find_library(VORBIS_LIB vorbis HINTS /opt/homebrew/lib)
    find_library(OGG_LIB ogg HINTS /opt/homebrew/lib)
    find_library(FLAC_LIB FLAC HINTS /opt/homebrew/lib)
    find_library(OPUS_LIB opus HINTS /opt/homebrew/lib)
    find_library(MPG123_LIB mpg123 HINTS /opt/homebrew/lib)
    find_library(LAME_LIB mp3lame HINTS /opt/homebrew/lib)

    target_link_libraries(bridgecommand-bc PRIVATE
        bc-asio
        enet
        bc-serial
        bc-irrlicht
        Threads::Threads
        sndfile
        portaudio
        ${VORBISENC_LIB}
        ${VORBIS_LIB}
        ${OGG_LIB}
        ${FLAC_LIB}
        ${OPUS_LIB}
        ${MPG123_LIB}
        ${LAME_LIB}
        "-framework AudioUnit"
	    "-framework AudioToolbox"
	    "-framework CoreAudio"
    )
    if(OPENAL_FOUND)
        target_link_libraries(bridgecommand-bc PRIVATE "-framework OpenAL")
    endif()
else (APPLE)
    target_link_libraries(bridgecommand-bc PRIVATE
        bc-asio
        enet
        bc-serial
        bc-irrlicht
        Threads::Threads
        sndfile
        portaudio
        asound
        GL
        OpenXR::openxr_loader
    )
    if(OPENAL_FOUND)
        target_link_libraries(bridgecommand-bc PRIVATE ${OPENAL_LIBRARY})
        target_include_directories(bridgecommand-bc PRIVATE ${OPENAL_INCLUDE_DIR})
    endif()
endif (APPLE)
