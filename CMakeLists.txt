cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_subdirectory(libs/asio)
add_subdirectory(libs/enet)
add_subdirectory(libs/Irrlicht)
add_subdirectory(libs/serial)

# optional tools
add_subdirectory(controller)
add_subdirectory(editor)
add_subdirectory(iniEditor)
add_subdirectory(launcher)
add_subdirectory(multiplayerHub)
add_subdirectory(repeater)

set(THREADS_PREFER_PTHREAD_FLAG ON)
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
    bc-enet
    bc-serial
    bc-irrlicht
    Threads::Threads
)
