cmake_minimum_required(VERSION 3.8)
project(bridgecommand)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(libs/asio)
add_subdirectory(libs/enet-1.3.14)
add_subdirectory(libs/Irrlicht)
add_subdirectory(libs/serial)

include_directories("libs/enet-1.3.14/include")
#add_definitions(-DWITH_SOUND)

# optional tools
add_subdirectory(controller)
add_subdirectory(editor)
add_subdirectory(iniEditor)
add_subdirectory(launcher)
add_subdirectory(multiplayerHub)
add_subdirectory(repeater)

set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads REQUIRED)

set(BC_SOURCES
    main.cpp
    Angles.cpp
    Buoy.cpp
    Buoys.cpp
    Camera.cpp
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
    Light.cpp
    ManOverboard.cpp
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
    StartupEventReceiver.cpp
    Terrain.cpp
    Tide.cpp
    Utilities.cpp
    Water.cpp
)

add_executable(bridgecommand-bc
    ${BC_SOURCES}
)

target_link_libraries(bridgecommand-bc PRIVATE
    bc-asio
    enet
    bc-serial
    bc-irrlicht
    Threads::Threads
    #sndfile
    #portaudio
    #asound
)
