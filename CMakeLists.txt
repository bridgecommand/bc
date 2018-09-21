cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(IRRLICHT_HOME "${CMAKE_CURRENT_SOURCE_DIR}/libs/Irrlicht/irrlicht-svn")

include(ExternalProject)
ExternalProject_Add(bc-irrlicht
    SOURCE_DIR "${IRRLICHT_HOME}/source/Irrlicht/"
    BINARY_DIR "${IRRLICHT_HOME}/source/Irrlicht/"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ${MAKE}
    INSTALL_COMMAND ""
)


include_directories(
    ./libs/enet/enet-1.3.11/include
    ./libs/asio/include
    "${IRRLICHT_HOME}/include/"
)

add_definitions(-DASIO_STANDALONE -DASIO_HAS_STD_THREAD)

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

    libs/enet/callbacks.c
    libs/enet/compress.c
    libs/enet/host.c
    libs/enet/list.c
    libs/enet/packet.c
    libs/enet/peer.c
    libs/enet/protocol.c
    libs/enet/unix.c
    libs/enet/win32.c

    libs/serial/src/impl/list_ports/list_ports_linux.cc
    libs/serial/src/impl/list_ports/list_ports_osx.cc
    libs/serial/src/impl/list_ports/list_ports_win.cc
    libs/serial/src/impl/unix.cc
    libs/serial/src/impl/win.cc
    libs/serial/src/serial.cc
)

add_executable(bridgecommand-bc
    ${BC_SOURCES}
)

add_dependencies(bridgecommand-bc
    bc-irrlicht
)

link_directories(
    /usr/X11R6/lib64 # or lib32
)

target_link_libraries(bridgecommand-bc PRIVATE
    "${IRRLICHT_HOME}/source/Irrlicht/libIrrlicht.a"
    GL
    Xxf86vm
    Xext
    X11
    Xcursor
    pthread
)
