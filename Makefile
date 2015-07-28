# Bridge Command 5.0 Makefile, based on Makefiles for Irrlicht Examples
# It's usually sufficient to change just the target name and source file list
# and be sure that CXX is set to a valid compiler

# Name of the executable created (.exe will be added automatically if necessary)
Target := bridgecommand
# List of source files, separated by spaces
Sources := main.cpp Angles.cpp Buoy.cpp Buoys.cpp Camera.cpp GUIMain.cpp IniFile.cpp LandLights.cpp LandObject.cpp LandObjects.cpp Lang.cpp Light.cpp MyEventReceiver.cpp NavLight.cpp Network.cpp NetworkPrimary.cpp NetworkSecondary.cpp NMEA.cpp OtherShip.cpp OtherShips.cpp OwnShip.cpp RadarCalculation.cpp RadarScreen.cpp Rain.cpp RealisticWater.cpp ScenarioChoice.cpp Ship.cpp SimulationModel.cpp Sky.cpp StartupEventReceiver.cpp Terrain.cpp Tide.cpp Utilities.cpp Water.cpp libs/enet/callbacks.c libs/enet/compress.c libs/enet/host.c libs/enet/list.c libs/enet/packet.c libs/enet/peer.c libs/enet/protocol.c libs/enet/unix.c libs/enet/win32.c libs/serial/src/impl/list_ports/list_ports_linux.cc libs/serial/src/impl/list_ports/list_ports_osx.cc libs/serial/src/impl/list_ports/list_ports_win.cc libs/serial/src/impl/unix.cc libs/serial/src/impl/win.cc libs/serial/src/serial.cc
# Path to Irrlicht directory, should contain include/ and lib/
IrrlichtHome := ./libs/Irrlicht/irrlicht-1.8.1
# Path for the executable. Note that Irrlicht.dll should usually also be there for win32 systems
BinPath = .

# general compiler settings (might need to be set when compiling the lib, too)
# preprocessor flags, e.g. defines and include paths
USERCPPFLAGS = -I./libs/enet/enet-1.3.11/include
# compiler flags such as optimization flags
USERCXXFLAGS = -O3 -ffast-math
#USERCXXFLAGS = -g -Wall
# linker flags such as additional libraries and link paths
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
USERLDFLAGS =  -L/usr/X11R6/lib$(LIBSELECT) -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -L./libs/Irrlicht/irrlicht-1.8.1/source/Irrlicht/MacOSX/build/Release -lIrrlicht
else
USERLDFLAGS = -L/usr/X11R6/lib$(LIBSELECT) -lGL -lXxf86vm -lXext -lX11 -lXcursor -L$(IrrlichtHome)/lib/$(SYSTEM) -lIrrlicht
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
