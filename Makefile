# Bridge Command 5.0 Makefile, based on Makefiles for Irrlicht Examples
# It's usually sufficient to change just the target name and source file list
# and be sure that CXX is set to a valid compiler

# Name of the executable created (.exe will be added automatically if necessary)
Target := bridgecommand-bc
# List of source files, separated by spaces
Sources := main.cpp Angles.cpp Buoy.cpp Buoys.cpp Camera.cpp DefaultEventReceiver.cpp FFTWave.cpp GUIMain.cpp GUIRectangle.cpp HeadingIndicator.cpp IniFile.cpp LandLights.cpp LandObject.cpp LandObjects.cpp Lang.cpp Light.cpp ManOverboard.cpp MovingWater.cpp MyEventReceiver.cpp NMEA.cpp NavLight.cpp Network.cpp NetworkPrimary.cpp NetworkSecondary.cpp NumberToImage.cpp OtherShip.cpp OtherShips.cpp OutlineScrollBar.cpp OwnShip.cpp RadarCalculation.cpp RadarScreen.cpp Rain.cpp ScenarioChoice.cpp ScenarioDataStructure.cpp ScrollDial.cpp Ship.cpp SimulationModel.cpp Sky.cpp Sound.cpp StartupEventReceiver.cpp Terrain.cpp Tide.cpp Utilities.cpp Water.cpp libs/enet/callbacks.c libs/enet/compress.c libs/enet/host.c libs/enet/list.c libs/enet/packet.c libs/enet/peer.c libs/enet/protocol.c libs/enet/unix.c libs/enet/win32.c libs/serial/src/impl/list_ports/list_ports_linux.cc libs/serial/src/impl/list_ports/list_ports_osx.cc libs/serial/src/impl/list_ports/list_ports_win.cc libs/serial/src/impl/unix.cc libs/serial/src/impl/win.cc libs/serial/src/serial.cc
# Path to Irrlicht directory, should contain include/ and lib/
IrrlichtHome := ./libs/Irrlicht/irrlicht-svn
# Path for the executable. Note that Irrlicht.dll should usually also be there for win32 systems
BinPath = .

# general compiler settings (might need to be set when compiling the lib, too)
# preprocessor flags, e.g. defines and include paths
UNAME_S := $(shell uname -s)
USERCPPFLAGS = -std=c++11 -I./libs/enet/enet-1.3.11/include -I./libs/asio/include -DASIO_STANDALONE -DASIO_HAS_STD_THREAD
# compiler flags such as optimization flags
ifeq ($(UNAME_S),Darwin)
USERCXXFLAGS = -O3 -ffast-math -mmacosx-version-min=10.7
else
USERCXXFLAGS = -O3 -ffast-math
endif
# linker flags such as additional libraries and link paths
ifeq ($(UNAME_S),Darwin)
USERLDFLAGS = -stdlib=libc++ -L./libs/Irrlicht/irrlicht-svn/lib/OSX -lIrrlicht -L/usr/X11R6/lib$(LIBSELECT) -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
else
USERLDFLAGS = -L$(IrrlichtHome)/lib/Linux -lIrrlicht -L/usr/X11R6/lib$(LIBSELECT) -lGL -lXxf86vm -lXext -lX11 -lXcursor -lpthread
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
	$(warning Building...)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(Sources) -o $(DESTPATH) $(LDFLAGS)
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
	cp -a shaders BridgeCommand.app/Contents/Resources/shaders
	cp -a World BridgeCommand.app/Contents/Resources/World
	cp -a bc5.ini BridgeCommand.app/Contents/Resources/bc5.ini
	cp -a map.ini BridgeCommand.app/Contents/Resources/map.ini
	cp -a mph.ini BridgeCommand.app/Contents/Resources/mph.ini
	cp -a repeater.ini BridgeCommand.app/Contents/Resources/repeater.ini
	cp -a language.txt BridgeCommand.app/Contents/Resources/language.txt
	cp -a languageController.txt BridgeCommand.app/Contents/Resources/languageController.txt
	cp -a languageMultiplayer.txt BridgeCommand.app/Contents/Resources/languageMultiplayer.txt 
	cp -a languageLauncher.txt BridgeCommand.app/Contents/Resources/languageLauncher.txt
	cp -a languageIniEditor.txt BridgeCommand.app/Contents/Resources/languageIniEditor.txt
	cp -a languageRepeater.txt BridgeCommand.app/Contents/Resources/languageRepeater.txt
endif

clean:
	$(warning Cleaning...)
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
