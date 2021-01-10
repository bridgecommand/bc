/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2014 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY Or FITNESS For A PARTICULAR PURPOSE.  See the
     GNU General Public License For more details.

     You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

// main.cpp

#ifdef WITH_PROFILING
#include "iprof.hpp"
#else
#define IPROF(a) //intentionally empty placeholder
#endif

// Include the Irrlicht header
#include "irrlicht.h"

#include "DefaultEventReceiver.hpp"
#include "GUIMain.hpp"
#include "ScenarioDataStructure.hpp"
#include "SimulationModel.hpp"
#include "ScenarioChoice.hpp"
#include "MyEventReceiver.hpp"
#include "Network.hpp"
#include "IniFile.hpp"
#include "Constants.hpp"
#include "Lang.hpp"
#include "NMEA.hpp"
#include "Sound.hpp"
#include "Utilities.hpp"
#include "OperatingModeEnum.hpp"

#include <cstdlib> //For rand(), srand()
#include <vector>
#include <sstream>
#include <fstream> //To save to log
#include <asio.hpp> //To display hostname

#ifdef _WIN32
#include <direct.h> //for windows _mkdir
#else
#include <sys/stat.h>
#endif // _WIN32

#include "profile.hpp"

//Mac OS:
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif


#ifdef _MSC_VER
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif


//Global definition for ini logger
namespace IniFile {
    irr::ILogger* irrlichtLogger = 0;
}

// Irrlicht Namespaces
//using namespace irr;

irr::core::stringw getCredits(){

    irr::core::stringw creditsString(L"NO DATA SUPPLIED WITH THIS PROGRAM, OR DERIVED FROM IT IS TO BE USED FOR NAVIGATION.\n\n");
    creditsString.append(L"Bridge Command is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 2 as published by the Free Software Foundation.\n\n");
    creditsString.append(L"Bridge Command  is distributed  in the  hope that  it will  be useful, but WITHOUT ANY WARRANTY; without even the implied  warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.\n\n");
    creditsString.append(L"In memory of Sergio Fuentes, who provided many useful suggestions for the program's development.\n\n");
    creditsString.append(L"Many thanks to those who have made their models available for use in Bridge Command:\n");
    creditsString.append(L"> Juergen Klemp\n");
    creditsString.append(L"> Simon D Richardson\n");
    creditsString.append(L"> Jason Simpson\n");
    creditsString.append(L"> Ragnar\n");
    creditsString.append(L"> Thierry Videlaine\n");
    creditsString.append(L"> NETC (Naval Education and Training Command)\n");
    creditsString.append(L"> Sky image from 0ptikz\n\n");
    creditsString.append(L"Many thanks to Ken Trethewey for making his images of the Eddystone lighthouse available.\n\n");

	creditsString.append(L"Bridge Command uses the Irrlicht Engine, the ENet networking library, ASIO, PortAudio, water based on Keith Lantz FFT water implementation, RealisticWaterSceneNode by elvman, AIS Parser by Brian C. Lane, and the Serial library by William Woodall. Bridge Command depends on libsndfile, which is released under the GNU Lesser General Public License version 2.1 or 3.\n\n");

    creditsString.append(L"The Irrlicht Engine is based in part on the work of the Independent JPEG Group, the zlib, and libpng.");

    return creditsString;
}

#ifdef _WIN32
static LRESULT CALLBACK CustomWndProc(HWND hWnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		HWND hwndCtl = (HWND)lParam;
		int code = HIWORD(wParam);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
#endif

int main()
{

    #ifdef WITH_PROFILING
    IPROF_FUNC;
    #endif

    #ifdef FOR_DEB
    chdir("/usr/share/bridgecommand");
    #endif // FOR_DEB

    //Mac OS:
	#ifdef __APPLE__
    //Find starting folder
    char exePath[1024];
    uint32_t pathSize = sizeof(exePath);
    std::string exeFolderPath = "";
    if (_NSGetExecutablePath(exePath, &pathSize) == 0) {
        std::string exePathString(exePath);
        size_t pos = exePathString.find_last_of("\\/");
        if (std::string::npos != pos) {
            exeFolderPath = exePathString.substr(0, pos);
        }
    }
    //change up from BridgeCommand.app/Contents/MacOS/bc.app/Contents/MacOS to BridgeCommand.app/Contents/Resources
    exeFolderPath.append("/../../../../Resources");
    //change to this path now, so ini file is read
    chdir(exeFolderPath.c_str());
    //Note, we use this again after the createDevice call
	#endif

    //User read/write location - look in here first and the exe folder second for files
    std::string userFolder = Utilities::getUserDir();

    //Read basic ini settings
    std::string iniFilename = "bc5.ini";
    //Use local ini file if it exists
    if (Utilities::pathExists(userFolder + iniFilename)) {
        iniFilename = userFolder + iniFilename;
    }

    #ifdef __arm__
    if (IniFile::iniFileTou32(iniFilename, "PA_ALSA_PLUGHW") == 1) {
        setenv("PA_ALSA_PLUGHW", "1", true);
    }
    #endif

    irr::u32 graphicsWidth = IniFile::iniFileTou32(iniFilename, "graphics_width");
    irr::u32 graphicsHeight = IniFile::iniFileTou32(iniFilename, "graphics_height");
    irr::u32 graphicsDepth = IniFile::iniFileTou32(iniFilename, "graphics_depth");
    bool fullScreen = (IniFile::iniFileTou32(iniFilename, "graphics_mode")==1); //1 for full screen
	bool fakeFullScreen = (IniFile::iniFileTou32(iniFilename, "graphics_mode") == 3); //3 for no border
	#ifdef __APPLE__
	if (fakeFullScreen) {
		fullScreen = true; //Fall back for mac
	}
	#endif
	irr::u32 antiAlias = IniFile::iniFileTou32(iniFilename, "anti_alias"); // 0 or 1 for disabled, 2,4,6,8 etc for FSAA
    irr::u32 directX = IniFile::iniFileTou32(iniFilename, "use_directX"); // 0 for openGl, 1 for directX (if available)
	irr::u32 disableShaders = IniFile::iniFileTou32(iniFilename, "disable_shaders"); // 0 for normal, 1 for no shaders
	if (directX == 1) {
		disableShaders = 1; //FIXME: Hardcoded for no directX shaders
	}
	irr::u32 waterSegments = IniFile::iniFileTou32(iniFilename, "water_segments"); // power of 2
	if (waterSegments == 0) {
		waterSegments = 32;
	}
    //Initial view configuration
    irr::f32 viewAngle = IniFile::iniFileTof32(iniFilename, "view_angle"); //Horizontal field of view
    irr::f32 lookAngle = IniFile::iniFileTof32(iniFilename, "look_angle"); //Initial look angle
    if (viewAngle <= 0) {
        viewAngle = 90;
    }

    irr::f32 cameraMinDistance = IniFile::iniFileTof32(iniFilename, "minimum_distance");
    irr::f32 cameraMaxDistance = IniFile::iniFileTof32(iniFilename, "maximum_distance");
    if (cameraMinDistance<=0) {
        cameraMinDistance = 1;
    }
    if (cameraMaxDistance<=0) {
        cameraMaxDistance = 6*M_IN_NM;
    }

    //Load joystick settings, subtract 1 as first axis is 0 internally (not 1)
    JoystickSetup joystickSetup;
    joystickSetup.portJoystickAxis = IniFile::iniFileTou32(iniFilename, "port_throttle_channel")-1;
    joystickSetup.stbdJoystickAxis = IniFile::iniFileTou32(iniFilename, "stbd_throttle_channel")-1;
    joystickSetup.rudderJoystickAxis = IniFile::iniFileTou32(iniFilename, "rudder_channel")-1;
    joystickSetup.bowThrusterJoystickAxis = IniFile::iniFileTou32(iniFilename, "bow_thruster_channel")-1;
    joystickSetup.sternThrusterJoystickAxis = IniFile::iniFileTou32(iniFilename, "stern_thruster_channel")-1;
    //Which joystick number
    joystickSetup.portJoystickNo = IniFile::iniFileTou32(iniFilename, "joystick_no_port"); //TODO: Note that these have changed after 5.0b4 to be consistent with BC4.7
    joystickSetup.stbdJoystickNo = IniFile::iniFileTou32(iniFilename, "joystick_no_stbd");
    joystickSetup.rudderJoystickNo = IniFile::iniFileTou32(iniFilename, "joystick_no_rudder");
    joystickSetup.bowThrusterJoystickNo = IniFile::iniFileTou32(iniFilename, "joystick_no_bow_thruster");
    joystickSetup.sternThrusterJoystickNo = IniFile::iniFileTou32(iniFilename, "joystick_no_stern_thruster");

    //Joystick mapping
    irr::u32 numberOfJoystickPoints = IniFile::iniFileTou32(iniFilename, "joystick_map_points");
    if (numberOfJoystickPoints > 0) {
        for (irr::u32 i = 1; i < numberOfJoystickPoints+1; i++) {
            joystickSetup.inputPoints.push_back(IniFile::iniFileTof32(iniFilename, IniFile::enumerate2("joystick_map",i,1)));
            joystickSetup.outputPoints.push_back(IniFile::iniFileTof32(iniFilename, IniFile::enumerate2("joystick_map",i,2)));
        }
    }
    //Default linear mapping if not set
    if (joystickSetup.inputPoints.size()<2) {
        joystickSetup.inputPoints.clear();
        joystickSetup.outputPoints.clear();
        joystickSetup.inputPoints.push_back(-1.0);
        joystickSetup.inputPoints.push_back(1.0);
        joystickSetup.outputPoints.push_back(-1.0);
        joystickSetup.outputPoints.push_back(1.0);
    }

    //Load NMEA settings
    std::string nmeaSerialPortName = IniFile::iniFileToString(iniFilename, "NMEA_ComPort");
    std::string nmeaUDPAddressName = IniFile::iniFileToString(iniFilename, "NMEA_UDPAddress");
    std::string nmeaUDPPortName = IniFile::iniFileToString(iniFilename, "NMEA_UDPPort");

    //Load UDP network settings
    irr::u32 udpPort = IniFile::iniFileTou32(iniFilename, "udp_send_port");
    if (udpPort == 0) {
        udpPort = 18304;
    }

    int fontSize = 12;
    float fontScale = IniFile::iniFileTof32(iniFilename, "font_scale");
    if (fontScale > 1) {
        fontSize = (int)(fontSize * fontScale + 0.5);
    } else {
	    fontScale = 1.0;
    }

    //Sensible defaults if not set
	if (graphicsWidth == 0 || graphicsHeight == 0) {
		irr::IrrlichtDevice *nulldevice = irr::createDevice(irr::video::EDT_NULL);
		irr::core::dimension2d<irr::u32> deskres = nulldevice->getVideoModeList()->getDesktopResolution();
		nulldevice->drop();
		if (graphicsWidth == 0) {
			if (fullScreen || fakeFullScreen) {
				graphicsWidth = deskres.Width;
			} else {
				graphicsWidth = 1200 * fontScale; // deskres.Width*0.8;
			}
		}
		if (graphicsHeight == 0) {
			if (fullScreen) {
				graphicsHeight = deskres.Height;
			}
			else {
				graphicsHeight = 900 * fontScale; // deskres.Height*0.8;
			}
		}
	}

	if (graphicsDepth == 0) { graphicsDepth = 32; }

    //load language
    std::string modifier = IniFile::iniFileToString(iniFilename, "lang");
    if (modifier.length()==0) {
        modifier = "en"; //Default
    }
    std::string languageFile = "language-";
    languageFile.append(modifier);
    languageFile.append(".txt");
    if (Utilities::pathExists(userFolder + languageFile)) {
        languageFile = userFolder + languageFile;
    }
    Lang language(languageFile);

	irr::SIrrlichtCreationParameters deviceParameters;

#ifdef _WIN32

	HWND hWnd;
	HINSTANCE hInstance = 0;
	// create dialog
	const char* Win32ClassName = "CIrrlichtWindowsTestDialog";

	WNDCLASSEX wcex;

	if (fakeFullScreen) {

		if (GetSystemMetrics(SM_CMONITORS) > 1) {
			irr::core::stringw locationMessageW = language.translate("moveMessage");

			std::wstring wlocationMessage = std::wstring(locationMessageW.c_str());
			std::string slocationMessage(wlocationMessage.begin(), wlocationMessage.end());

			MessageBoxA(nullptr, slocationMessage.c_str(), "Multi monitor", MB_OK);
		}

		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = (WNDPROC)CustomWndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = DLGWINDOWEXTRA;
		wcex.hInstance = hInstance;
		wcex.hIcon = NULL;
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
		wcex.lpszMenuName = 0;
		wcex.lpszClassName = Win32ClassName;
		wcex.hIconSm = 0;

		RegisterClassEx(&wcex);

		//Find location of mouse cursor
		POINT p;
		int x = 0;
		int y = 0;
		if (GetCursorPos(&p))
		{
			//Find monitor this is on
			HMONITOR monitor = MonitorFromPoint(p, MONITOR_DEFAULTTOPRIMARY);
			MONITORINFO mi;
			RECT        rc;

			mi.cbSize = sizeof(mi);
			GetMonitorInfo(monitor, &mi);
			rc = mi.rcMonitor;

			//Set to fill current monitor
			x = rc.left;
			y = rc.top;
			graphicsWidth = rc.right - rc.left;
			graphicsHeight = rc.bottom - rc.top;

		}


		DWORD style = WS_VISIBLE | WS_POPUP;

		hWnd = CreateWindowA(Win32ClassName, "Bridge Command",
			style, x, y, graphicsWidth, graphicsHeight,
			NULL, NULL, hInstance, NULL);

		deviceParameters.WindowId = hWnd; //Tell irrlicht about the window to use

	}
#endif

    //Use an extra SIrrlichtCreationParameters parameter, added to our version of the Irrlicht source, to request a borderless X11 window if requested
    #ifdef __linux__
    if (fakeFullScreen) {
	deviceParameters.X11borderless=true; //Has an effect on X11 only
    }
    #endif

    //create device
    deviceParameters.DriverType = irr::video::EDT_OPENGL;
	//Allow optional directX if available
	if (directX==1) {
        if (irr::IrrlichtDevice::isDriverSupported(irr::video::EDT_DIRECT3D9)) {
            deviceParameters.DriverType = irr::video::EDT_DIRECT3D9;
        } else {
            std::cerr << "DirectX 9 requested but not available.\nThis may be because Bridge Command has been compiled without DirectX support,\nor your system does not support DirectX.\nTrying OpenGL" << std::endl << std::endl;
        }
	}

    deviceParameters.WindowSize = irr::core::dimension2d<irr::u32>(graphicsWidth,graphicsHeight);
    deviceParameters.Bits = graphicsDepth;
    deviceParameters.Fullscreen = fullScreen;
    deviceParameters.AntiAlias = antiAlias;

    irr::IrrlichtDevice* device = irr::createDeviceEx(deviceParameters);

	//On Windows, redirect console stderr to log file
	std::string userLog = userFolder + "log.txt";
	std::cout << "User log file is " << userLog << std::endl;
	/*
	FILE * stream = 0;
	#ifdef _WIN32
	errno_t success = freopen_s(&stream, userLog.c_str(), "w", stderr);
	#endif // _WIN32
    */
	if (device == 0) {
		std::cerr << "Could not start - please check your graphics options." << std::endl;
		return(EXIT_FAILURE); //Could not get file system
	}

    device->setWindowCaption(irr::core::stringw(LONGNAME.c_str()).c_str()); //Note: Odd conversion from char* to wchar*!

    irr::video::IVideoDriver* driver = device->getVideoDriver();
    irr::scene::ISceneManager* smgr = device->getSceneManager();

    std::vector<std::string> logMessages;
    DefaultEventReceiver defReceiver(&logMessages, device);
    device->setEventReceiver(&defReceiver);

    //Tell the Ini routine the logger address
    IniFile::irrlichtLogger = device->getLogger();

    device->getLogger()->log("User folder is:");
    device->getLogger()->log(userFolder.c_str());

    smgr->getParameters()->setAttribute(irr::scene::ALLOW_ZWRITE_ON_TRANSPARENT, true);

    #ifdef __APPLE__
    //Bring window to front
    //NSWindow* window = reinterpret_cast<NSWindow>(device->getVideoDriver()->getExposedVideoData().HWnd);
    //Mac OS - cd back to original dir - seems to be changed during createDevice
    irr::io::IFileSystem* fileSystem = device->getFileSystem();
    if (fileSystem==0) {
        std::cerr << "Could not get filesystem:" << std::endl;
        return(EXIT_FAILURE); //Could not get file system

    }
    fileSystem->changeWorkingDirectoryTo(exeFolderPath.c_str());
    #endif

    //set gui skin and 'flatten' this
    irr::gui::IGUISkin* newskin = device->getGUIEnvironment()->createSkin(irr::gui::EGST_WINDOWS_METALLIC   );

    device->getGUIEnvironment()->setSkin(newskin);
    newskin->drop();

	irr::u32 su = driver->getScreenSize().Width;
	irr::u32 sh = driver->getScreenSize().Height;

	//set size of camera window, based on actual window
	graphicsWidth = su;
	graphicsHeight = sh;
	irr::u32 graphicsWidth3d = su;
	irr::u32 graphicsHeight3d = sh * 0.6;
	irr::f32 aspect = (irr::f32)su / (irr::f32)sh;
	irr::f32 aspect3d = (irr::f32)graphicsWidth3d / (irr::f32)graphicsHeight3d;

	std::cout << "graphicsWidth: "<< graphicsWidth << " graphicsHeight: " << graphicsHeight << std::endl;

    std::string fontName = IniFile::iniFileToString(iniFilename, "font");
    std::string fontPath = "media/fonts/" + fontName + "/" + fontName + "-" + std::to_string(fontSize) + ".xml";
    irr::gui::IGUIFont *font = device->getGUIEnvironment()->getFont(fontPath.c_str());
    if (font == NULL) {
        std::cout << "Could not load font, using fallback" << std::endl;
    } else {
        //set skin default font
        device->getGUIEnvironment()->getSkin()->setFont(font);
    }

    //Choose scenario
    std::string scenarioName = "";
    std::string hostname = "";
    //Scenario path - default to user dir if it exists
    std::string scenarioPath = "Scenarios/";
    if (Utilities::pathExists(userFolder + scenarioPath)) {
        scenarioPath = userFolder + scenarioPath;
    }

    //Find default hostname if set in user directory (hostname.txt)
    if (Utilities::pathExists(userFolder + "/hostname.txt")) {
        hostname=IniFile::iniFileToString(userFolder + "/hostname.txt","hostname");
    }

	//Start sound
	Sound sound;

    OperatingMode::Mode mode = OperatingMode::Normal;
    ScenarioChoice scenarioChoice(device,&language);
    scenarioChoice.chooseScenario(scenarioName, hostname, udpPort, mode, scenarioPath);

    //Save hostname in user directory (hostname.txt). Check first that the location exists
    if (!Utilities::pathExists(Utilities::getUserDirBase())) {
        std::string pathToMake = Utilities::getUserDirBase();
        if (pathToMake.size() > 1) {pathToMake.erase(pathToMake.size()-1);} //Remove trailing slash
        #ifdef _WIN32
        _mkdir(pathToMake.c_str());
        #else
        mkdir(pathToMake.c_str(),0755);
        #endif // _WIN32
    }
    if (!Utilities::pathExists(Utilities::getUserDir())) {
        std::string pathToMake = Utilities::getUserDir();
        if (pathToMake.size() > 1) {pathToMake.erase(pathToMake.size()-1);} //Remove trailing slash
        #ifdef _WIN32
        _mkdir(pathToMake.c_str());
        #else
        mkdir(pathToMake.c_str(),0755);
        #endif // _WIN32
    }

    if (Utilities::pathExists(userFolder)) { //TODO: Should we make this if it doesn't exist?
        std::string hostnameFile = userFolder + "/hostname.txt";
        std::ofstream file (hostnameFile.c_str());
        if (file.is_open()) {
            file << "hostname=" << hostname << std::endl;
            file.close();
        }
    }

	//Show loading message

	irr::u32 creditsStartTime = device->getTimer()->getRealTime();
    irr::core::stringw creditsText = language.translate("loadingmsg");
    creditsText.append(L"\n\n");
    creditsText.append(getCredits());
    irr::gui::IGUIStaticText* loadingMessage = device->getGUIEnvironment()->addStaticText(creditsText.c_str(), irr::core::rect<irr::s32>(0.05*su,0.05*sh,0.95*su,0.95*sh),true);
    device->run();
    driver->beginScene(irr::video::ECBF_COLOR|irr::video::ECBF_DEPTH, irr::video::SColor(0,200,200,200));
    device->getGUIEnvironment()->drawAll();
    driver->endScene();

    //seed random number generator
    std::srand(device->getTimer()->getTime());

    //create GUI
    GUIMain guiMain;

    //Set up networking (this will get a pointer to the model later)
    //Create networking, linked to model, choosing whether to use main or secondary network mode
    Network* network = Network::createNetwork(mode, udpPort, device);
    //Network network(&model);
    network->connectToServer(hostname);

    //Read in scenario data (work in progress)
    ScenarioData scenarioData;
    if (mode == OperatingMode::Normal) {
        scenarioData = Utilities::getScenarioDataFromFile(scenarioPath + scenarioName, scenarioName);
    } else {
        //If in secondary mode, get scenario information from the server
        //Tell user what we're doing
        irr::core::stringw portMessage = language.translate("secondaryWait");
        portMessage.append(L" ");
        std::string ourHostName = asio::ip::host_name();
        portMessage.append(irr::core::stringw(ourHostName.c_str()));
        portMessage.append(L":");
        portMessage.append(irr::core::stringw(network->getPort()));
        loadingMessage->setText(portMessage.c_str());
        device->run();
        driver->beginScene(irr::video::ECBF_COLOR|irr::video::ECBF_DEPTH, irr::video::SColor(0,200,200,200));
        device->getGUIEnvironment()->drawAll();
        driver->endScene();
        //Get the data
        std::string receivedSerialisedScenarioData;
        while (device->run() && receivedSerialisedScenarioData.empty()) {
            network->getScenarioFromNetwork(receivedSerialisedScenarioData);
        }
        scenarioData.deserialise(receivedSerialisedScenarioData);
    }
    std::string serialisedScenarioData = scenarioData.serialise();

    loadingMessage->remove(); loadingMessage = 0;

    //Note: We could use this serialised format as a scenario import/export format or for online distribution


    //Create simulation model
    SimulationModel model(device, smgr, &guiMain, &sound, scenarioData, mode, viewAngle, lookAngle, cameraMinDistance, cameraMaxDistance, disableShaders, waterSegments);

    //Load the gui
    bool hideEngineAndRudder=false;
    if (mode==OperatingMode::Secondary) {
        hideEngineAndRudder=true;
    }
    guiMain.load(device, &language, &logMessages, model.isSingleEngine(),hideEngineAndRudder,model.hasDepthSounder(),model.getMaxSounderDepth(),model.hasGPS(), model.hasBowThruster(), model.hasSternThruster(), model.hasTurnIndicator());

    //Give the network class a pointer to the model
    network->setModel(&model);

    //load realistic water
    //RealisticWaterSceneNode* realisticWater = new RealisticWaterSceneNode(smgr, 4000, 4000, "./",irr::core::dimension2du(512, 512),smgr->getRootSceneNode());

    //create event receiver, linked to model
    MyEventReceiver receiver(device, &model, &guiMain, joystickSetup, &logMessages);
    device->setEventReceiver(&receiver);

    //create NMEA serial port and UDP, linked to model
    NMEA nmea(&model, nmeaSerialPortName, nmeaUDPAddressName, nmeaUDPPortName, device);

	//Load sound files
	sound.load(model.getOwnShipEngineSound(), model.getOwnShipWaveSound(), model.getOwnShipHornSound());

    //check enough time has elapsed to show the credits screen (5s)
    while(device->getTimer()->getRealTime() - creditsStartTime < 5000) {
        device->run();
    }
    //remove credits here
    //loadingMessage->remove(); loadingMessage = 0;

    //set up timing for NMEA
    const irr::u32 NMEA_UPDATE_MS = 250;
    irr::u32 nextNMEATime = device->getTimer()->getTime()+NMEA_UPDATE_MS;

//    Profiling
//    Profiler networkProfile("Network");
//    Profiler nmeaProfile("NMEA");
//    Profiler modelProfile("Model");
//    Profiler renderSetupProfile("Render setup");
//    Profiler renderRadarProfile("Render radar");
//    Profiler renderProfile("3d render");
//    Profiler guiProfile("GUI render");
//    Profiler renderFinishProfile("Render finish");

	sound.StartSound();

    //main loop
    while(device->run())
    {

        { IPROF("Network");
//        networkProfile.tic();
        network->update();
//        networkProfile.toc();

        //Check if time has elapsed, so we send data once per NMEA_UPDATE_MS.
//        nmeaProfile.tic();
        }{ IPROF("NMEA");
        if (device->getTimer()->getTime() >= nextNMEATime) {

            if (!nmeaSerialPortName.empty() || (!nmeaUDPAddressName.empty() && !nmeaUDPPortName.empty())) {
                nmea.updateNMEA();

                if (!nmeaSerialPortName.empty()) {
                    nmea.sendNMEASerial();
                }

                if (!nmeaUDPAddressName.empty() && !nmeaUDPPortName.empty()) {
                    nmea.sendNMEAUDP();
                }

            nextNMEATime = device->getTimer()->getTime()+NMEA_UPDATE_MS;
            }
        }
//        nmeaProfile.toc();

//        modelProfile.tic();
        }{ IPROF("Model");
        model.update();
//        modelProfile.toc();


        //Set up

//        renderSetupProfile.tic();
        }{ IPROF("Render setup");
        driver->setViewPort(irr::core::rect<irr::s32>(0,0,graphicsWidth,graphicsHeight)); //Full screen before beginScene
        driver->beginScene(irr::video::ECBF_COLOR|irr::video::ECBF_DEPTH, irr::video::SColor(0,128,128,128));
//        renderSetupProfile.toc();

//        renderRadarProfile.tic();

        }
        bool fullScreenRadar = guiMain.getLargeRadar();
        { IPROF("Render radar");
        if (model.isRadarOn()) {
            //radar view portion
            if (graphicsHeight>graphicsHeight3d && (guiMain.getShowInterface() || fullScreenRadar)) {
                model.setWaterVisible(false); //Hide the reflecting water, as this updates itself on drawAll()
                if (fullScreenRadar) {
                    driver->setViewPort(guiMain.getLargeRadarRect());
                } else {
                    driver->setViewPort(irr::core::rect<irr::s32>(graphicsWidth-(graphicsHeight-graphicsHeight3d),graphicsHeight3d,graphicsWidth,graphicsHeight));
                }
                model.setRadarCameraActive();
                smgr->drawAll();
                model.setWaterVisible(true); //Re-show the water
            }
        }

 //       renderRadarProfile.toc();

 //       renderProfile.tic();
        }{ IPROF("Render");

        //3d view portion
        model.setMainCameraActive(); //Note that the NavLights expect the main camera to be active, so they know where they're being viewed from
        if (!fullScreenRadar) {
            if (guiMain.getShowInterface()) {
                driver->setViewPort(irr::core::rect<irr::s32>(0,0,graphicsWidth3d,graphicsHeight3d));
                model.updateViewport(aspect3d);
            } else {
                driver->setViewPort(irr::core::rect<irr::s32>(0,0,graphicsWidth,graphicsHeight));
                model.updateViewport(aspect);
            }
            //drawAll3dProfile.tic();
            smgr->drawAll();
            //drawAll3dProfile.toc();
        }

 //       renderProfile.toc();

 //       guiProfile.tic();
        }{ IPROF("GUI");
        //gui
        driver->setViewPort(irr::core::rect<irr::s32>(0,0,graphicsWidth,graphicsHeight)); //Full screen for gui
        guiMain.drawGUI();
 //       guiProfile.toc();
        }{ IPROF("End scene");
 //       renderFinishProfile.tic();
        driver->endScene();
 //       renderFinishProfile.toc();
        }

    }

    #ifdef WITH_PROFILING
    InternalProfiler::aggregateEntries();

    std::cout << "\nThe profiler stats so far:\n"
           "WHAT: AVG_TIME (TOTAL_TIME / TIMES_EXECUTED)"
           "\nAll times in micro seconds\n"
        << InternalProfiler::stats << std::endl;
    #endif

    //networking should be stopped (presumably with destructor when it goes out of scope?)
    device->getLogger()->log("About to stop network");
    delete network;

    device->drop();

    //Save log messages out
	//Note that stderr has also been redirected to this file on windows, so it will contain anything from cerr, as well as these log messages
	//Save log messages to user directory, into log.txt, overwrite old file with that name
	std::ofstream logFile;
	/*
	if (stream) {
		fclose(stream);
		logFile.open(userLog, std::ofstream::app); //Append
	}
	else {
	*/
		logFile.open(userLog); //Overwrite
	/*
	}
	*/


	for (unsigned int i=0;i<logMessages.size();i++) {
        if (logFile.good()) {
            //Check we're not creating an excessively long file
            if (i<=1000 && logMessages.at(i).length() <=1000) {
                logFile << "Log: " << logMessages.at(i) << std::endl;
            }
        }
    }

    //End
    return(0);
}
