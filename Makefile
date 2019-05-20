# Bridge Command 5.0 Makefile, based on Makefiles for Irrlicht Examples
# It's usually sufficient to change just the target name and source file list
# and be sure that CXX is set to a valid compiler

# Name of the executable created (.exe will be added automatically if necessary)
Target := bridgecommand-bc

# List of source files
Sources += main.cpp
Sources += Angles.cpp
Sources += Buoy.cpp
Sources += Buoys.cpp
Sources += Camera.cpp
Sources += DefaultEventReceiver.cpp
Sources += FFTWave.cpp
Sources += GUIMain.cpp
Sources += GUIRectangle.cpp
Sources += HeadingIndicator.cpp
Sources += IniFile.cpp
Sources += LandLights.cpp
Sources += LandObject.cpp
Sources += LandObjects.cpp
Sources += Lang.cpp
Sources += Light.cpp
Sources += ManOverboard.cpp
Sources += MovingWater.cpp
Sources += MyEventReceiver.cpp
Sources += NMEA.cpp
Sources += NavLight.cpp
Sources += Network.cpp
Sources += NetworkPrimary.cpp
Sources += NetworkSecondary.cpp
Sources += NumberToImage.cpp
Sources += OtherShip.cpp
Sources += OtherShips.cpp
Sources += OutlineScrollBar.cpp
Sources += OwnShip.cpp
Sources += RadarCalculation.cpp
Sources += RadarScreen.cpp
Sources += Rain.cpp
Sources += ScenarioChoice.cpp
Sources += ScenarioDataStructure.cpp
Sources += ScrollDial.cpp
Sources += Ship.cpp
Sources += SimulationModel.cpp
Sources += Sky.cpp
Sources += Sound.cpp
Sources += StartupEventReceiver.cpp
Sources += Terrain.cpp
Sources += Tide.cpp
Sources += Utilities.cpp
Sources += Water.cpp

Sources += libs/enet/callbacks.c
Sources += libs/enet/compress.c
Sources += libs/enet/host.c
Sources += libs/enet/list.c
Sources += libs/enet/packet.c
Sources += libs/enet/peer.c
Sources += libs/enet/protocol.c
Sources += libs/enet/unix.c
Sources += libs/enet/win32.c

Sources += libs/serial/src/impl/list_ports/list_ports_linux.cc
Sources += libs/serial/src/impl/list_ports/list_ports_osx.cc
Sources += libs/serial/src/impl/list_ports/list_ports_win.cc
Sources += libs/serial/src/impl/unix.cc
Sources += libs/serial/src/impl/win.cc
Sources += libs/serial/src/serial.cc

# Path to Irrlicht directory, should contain include/ and lib/
IrrlichtHome := ./libs/Irrlicht/irrlicht-svn

# Path for the executable. Note that Irrlicht.dll should usually also be there for win32 systems
BinPath = .

# general compiler settings (might need to be set when compiling the lib, too)
# preprocessor flags, e.g. defines and include paths
UNAME_S := $(shell uname -s)
USERCPPFLAGS = -std=c++11 \
	-I./libs/enet/enet-1.3.11/include \
	-I./libs/asio/include \
	-DASIO_STANDALONE \
	-DASIO_HAS_STD_THREAD

# compiler flags such as optimization flags
ifeq ($(UNAME_S),Darwin)
USERCXXFLAGS = -O3 -ffast-math -mmacosx-version-min=10.7
else
USERCXXFLAGS = -O3 -ffast-math
endif
# linker flags such as additional libraries and link paths
ifeq ($(UNAME_S),Darwin)
USERLDFLAGS = -stdlib=libc++ \
	-L./libs/Irrlicht/irrlicht-svn/lib/OSX \
	-lIrrlicht \
	-L/usr/X11R6/lib$(LIBSELECT) \
	-framework OpenGL \
	-framework Cocoa \
	-framework IOKit \
	-framework CoreVideo
else
USERLDFLAGS = -L$(IrrlichtHome)/lib/Linux \
	-lIrrlicht \
	-L/usr/X11R6/lib$(LIBSELECT) \
	-lGL \
	-lXxf86vm \
	-lXext \
	-lX11 \
	-lXcursor \
	-lpthread
endif

####
#no changes necessary below this line
####

CPPFLAGS = -I$(IrrlichtHome)/include -I/usr/X11R6/include $(USERCPPFLAGS)
CXXFLAGS = $(USERCXXFLAGS)
LDFLAGS = $(USERLDFLAGS)

# name of the binary - only valid for targets which set SYSTEM
DESTPATH = $(BinPath)/$(Target)$(SUF)

#default target is Linux
all:
	$(info Building...)
#Build Irrlicht (different command on OSX)
ifeq ($(UNAME_S),Darwin)
	xcodebuild -project libs/Irrlicht/irrlicht-svn/source/Irrlicht/Irrlicht.xcodeproj
else
	$(MAKE) -C $(IrrlichtHome)/source/Irrlicht/ all
endif
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(Sources) -o $(DESTPATH) $(LDFLAGS)
#Build additional programs
	$(MAKE) -C launcher/ all
	$(MAKE) -C controller/ all
	$(MAKE) -C editor/ all
	$(MAKE) -C iniEditor/ all
	$(MAKE) -C multiplayerHub/ all
	$(MAKE) -C repeater/ all
ifeq ($(UNAME_S),Darwin)
	cp $(DESTPATH) BridgeCommand.app/Contents/MacOS/bc.app/Contents/MacOS/bc
	rm -f BridgeCommand.app/Contents/MacOS/bc.app/Contents/MacOS/.gitignore
	rm -rf BridgeCommand.app/Contents/Resources/media
	rm -rf BridgeCommand.app/Contents/Resources/Models
	rm -rf BridgeCommand.app/Contents/Resources/Scenarios
	rm -rf BridgeCommand.app/Contents/Resources/shaders
	rm -rf BridgeCommand.app/Contents/Resources/World
	cp -a doc BridgeCommand.app/Contents/Resources/doc
	cp -a media BridgeCommand.app/Contents/Resources/media
	cp -a Models BridgeCommand.app/Contents/Resources/Models
	cp -a Scenarios BridgeCommand.app/Contents/Resources/Scenarios
	mkdir BridgeCommand.app/Contents/Resources/shaders
	cat shaders/Water_ps.glsl | sed -E 's/#version 130/#version 120/' > BridgeCommand.app/Contents/Resources/shaders/Water_ps.glsl
	cat shaders/Water_vs.glsl | sed -E 's/#version 130/#version 120/' > BridgeCommand.app/Contents/Resources/shaders/Water_vs.glsl
	cp -a World BridgeCommand.app/Contents/Resources/World
	cp -a bc5.ini BridgeCommand.app/Contents/Resources/bc5.ini
	cp -a map.ini BridgeCommand.app/Contents/Resources/map.ini
	cp -a mph.ini BridgeCommand.app/Contents/Resources/mph.ini
	cp -a repeater.ini BridgeCommand.app/Contents/Resources/repeater.ini
	cp -a language-en.txt BridgeCommand.app/Contents/Resources/language-en.txt
	cp -a languageController-en.txt BridgeCommand.app/Contents/Resources/languageController-en.txt
	cp -a languageMultiplayer-en.txt BridgeCommand.app/Contents/Resources/languageMultiplayer-en.txt
	cp -a languageLauncher-en.txt BridgeCommand.app/Contents/Resources/languageLauncher-en.txt
	cp -a languageIniEditor-en.txt BridgeCommand.app/Contents/Resources/languageIniEditor-en.txt
	cp -a languageRepeater-en.txt BridgeCommand.app/Contents/Resources/languageRepeater-en.txt
endif

clean:
	$(info Cleaning...)
ifeq ($(UNAME_S),Darwin)
	xcodebuild -project libs/Irrlicht/irrlicht-svn/source/Irrlicht/Irrlicht.xcodeproj clean
else
	$(MAKE) -C $(IrrlichtHome)/source/Irrlicht/ clean
endif
	$(MAKE) -C launcher/ clean
	$(MAKE) -C controller/ clean
	$(MAKE) -C editor/ clean
	$(MAKE) -C iniEditor/ clean
	$(MAKE) -C multiplayerHub/ clean
	$(MAKE) -C repeater/ clean
	@$(RM) $(DESTPATH)

.PHONY: all

#multilib handling
ifeq ($(HOSTTYPE), x86_64)
LIBSELECT=64
endif
#solaris real-time features
ifeq ($(HOSTTYPE), sun4)
LDFLAGS += -lrt
endif

