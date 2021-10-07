#include "irrlicht.h"
#include <iostream>
#include <cstdio>
#include <vector>
#include <asio.hpp>
#include "PositionDataStruct.hpp"
#include "ShipDataStruct.hpp"
#include "Network.hpp"
#include "ControllerModel.hpp"
#include "GUI.hpp"
#include "EventReceiver.hpp"

#include "../IniFile.hpp"
#include "../Lang.hpp"
#include "../Utilities.hpp"

//Mac OS:
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#ifdef _MSC_VER
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

// Irrlicht Namespaces
//using namespace irr;

//Set up global for ini reader to have access to irrlicht logger if needed.
namespace IniFile {
    irr::ILogger* irrlichtLogger = 0;
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

#ifdef _WIN32
//From https://superkogito.github.io/blog/LoopMonitorsDetailsInCplusplus.html
// Structure that includes all screen hanldes and rectangles
struct cMonitorsVec
{
      std::vector<int>       iMonitors;
      std::vector<HMONITOR>  hMonitors;
      std::vector<HDC>       hdcMonitors;
      std::vector<RECT>      rcMonitors;

      static BOOL CALLBACK MonitorEnum(HMONITOR hMon, HDC hdc, LPRECT lprcMonitor, LPARAM pData)
      {
              cMonitorsVec* pThis = reinterpret_cast<cMonitorsVec*>(pData);

              pThis->hMonitors.push_back(hMon);
              pThis->hdcMonitors.push_back(hdc);
              pThis->rcMonitors.push_back(*lprcMonitor);
              pThis->iMonitors.push_back(pThis->hdcMonitors.size());
              return TRUE;
      }

      cMonitorsVec()
      {
              EnumDisplayMonitors(0, 0, MonitorEnum, (LPARAM)this);
      }
};
#endif // _WIN32

int main (int argc, char ** argv)
{

    #ifdef FOR_DEB
    chdir("/usr/share/bridgecommand");
    #endif // FOR_DEB

    //Mac OS:
    //Find starting folder
	#ifdef __APPLE__
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
	//change up from BridgeCommand.app/Contents/MacOS/mc.app/Contents/MacOS to BridgeCommand.app/Contents/Resources
    exeFolderPath.append("/../../../../Resources");
    //change to this path now, so ini file is read
    chdir(exeFolderPath.c_str());
    //Note, we use this again after the createDevice call
	#endif

    //User read/write location - look in here first and the exe folder second for files
    std::string userFolder = Utilities::getUserDir();

    std::string iniFilename = "repeater.ini";
    //Use local ini file if it exists
    if (Utilities::pathExists(userFolder + iniFilename)) {
        iniFilename = userFolder + iniFilename;
    }

    if ((argc>2)&&(strcmp(argv[1],"-c")==0)) {
        iniFilename = std::string(argv[2]); //TODO: Check this for sanity?
        std::cout << "Using Ini file >" << iniFilename << "<" << std::endl;
    }

    irr::u32 graphicsWidth = IniFile::iniFileTou32(iniFilename, "graphics_width");
    irr::u32 graphicsHeight = IniFile::iniFileTou32(iniFilename, "graphics_height");
    irr::u32 graphicsDepth = IniFile::iniFileTou32(iniFilename, "graphics_depth");
    bool fullScreen = (IniFile::iniFileTou32(iniFilename, "graphics_mode")==1); //1 for full screen
	bool fakeFullScreen = (IniFile::iniFileTou32(iniFilename, "graphics_mode") == 3); //3 for no border
	if (fakeFullScreen) {
		fullScreen = true; //Fall back for non-windows
	}

	int fontSize = 13;
    float fontScale = IniFile::iniFileTof32(iniFilename, "font_scale");
    fontSize = (int)(fontSize * fontScale + 0.5);

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
                if (graphicsWidth > deskres.Width*0.9) {
                    graphicsWidth = deskres.Width*0.9;
                }
			}
		}
		if (graphicsHeight == 0) {
			if (fullScreen) {
				graphicsHeight = deskres.Height;
			}
			else {
				graphicsHeight = 900 * fontScale; // deskres.Height*0.8;
                if (graphicsHeight > deskres.Height*0.9) {
                    graphicsHeight = deskres.Height*0.9;
                }
			}
		}
	}

	if (graphicsDepth == 0) { graphicsDepth = 32; }
    //Load UDP network settings
    irr::u32 udpPort = IniFile::iniFileTou32(iniFilename, "udp_send_port");
    if (udpPort == 0) {
        udpPort = 18304;
    }

	//load language
	std::string modifier = IniFile::iniFileToString(iniFilename, "lang");
	if (modifier.length() == 0) {
		modifier = "en"; //Default
	}
	std::string languageFile = "languageRepeater-";
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

        int requestedMonitor = IniFile::iniFileTou32(iniFilename, "monitor")-1; //0 indexed, -1 will indicate default

        DWORD style = WS_VISIBLE | WS_POPUP;
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

        cMonitorsVec Monitors; //The constructor for this initialises it with a list of the monitors

        if (requestedMonitor>-1 && Monitors.iMonitors.size() > requestedMonitor) {
            //The user has requested a specific monitor

            //Set to fill requested monitor
            int x = Monitors.rcMonitors[requestedMonitor].left;
            int y = Monitors.rcMonitors[requestedMonitor].top;
            graphicsWidth = Monitors.rcMonitors[requestedMonitor].right - Monitors.rcMonitors[requestedMonitor].left;
            graphicsHeight = Monitors.rcMonitors[requestedMonitor].bottom - Monitors.rcMonitors[requestedMonitor].top;

            hWnd = CreateWindowA(Win32ClassName, "Bridge Command",
                style, x, y, graphicsWidth, graphicsHeight,
                NULL, NULL, hInstance, NULL);

            deviceParameters.WindowId = hWnd; //Tell irrlicht about the window to use


        } else {
            //Get user to move a dialog, so their mouse is positioned on the monitor they want
            if (GetSystemMetrics(SM_CMONITORS) > 1) {
                irr::core::stringw locationMessageW = language.translate("moveMessage");

                std::wstring wlocationMessage = std::wstring(locationMessageW.c_str());
                std::string slocationMessage(wlocationMessage.begin(), wlocationMessage.end());

                MessageBoxA(nullptr, slocationMessage.c_str(), "Multi monitor", MB_OK);
            }

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

            hWnd = CreateWindowA(Win32ClassName, "Bridge Command",
                style, x, y, graphicsWidth, graphicsHeight,
                NULL, NULL, hInstance, NULL);

            deviceParameters.WindowId = hWnd; //Tell irrlicht about the window to use
        }

	}
#endif

    //IrrlichtDevice* device = createDevice(video::EDT_OPENGL, irr::core::dimension2d<irr::u32>(graphicsWidth,graphicsHeight),graphicsDepth,fullScreen,false,false,0);

	deviceParameters.DriverType = irr::video::EDT_OPENGL;
	deviceParameters.WindowSize = irr::core::dimension2d<irr::u32>(graphicsWidth, graphicsHeight);
	deviceParameters.Bits = graphicsDepth;
	deviceParameters.Fullscreen = fullScreen;

	irr::IrrlichtDevice* device = createDeviceEx(deviceParameters);
	if (device == 0) {
		std::cerr << "Could not start - please check your graphics options." << std::endl;
		exit(EXIT_FAILURE); //Could not get file system
	}

	irr::video::IVideoDriver* driver = device->getVideoDriver();
    //scene::ISceneManager* smgr = device->getSceneManager();


    #ifdef __APPLE__
    //Mac OS - cd back to original dir - seems to be changed during createDevice
	irr::io::IFileSystem* fileSystem = device->getFileSystem();
    if (fileSystem==0) {
        exit(EXIT_FAILURE); //Could not get file system TODO: Message for user
        std::cout << "Could not get filesystem" << std::endl;
    }
    fileSystem->changeWorkingDirectoryTo(exeFolderPath.c_str());
    #endif

    std::string fontName = IniFile::iniFileToString(iniFilename, "font");
    std::string fontPath = "media/fonts/" + fontName + "/" + fontName + "-" + std::to_string(fontSize) + ".xml";
    irr::gui::IGUIFont *font = device->getGUIEnvironment()->getFont(fontPath.c_str());
    if (font == NULL) {
        std::cout << "Could not load font, using fallback" << std::endl;
    } else {
        //set skin default font
        device->getGUIEnvironment()->getSkin()->setFont(font);
    }

    //Classes:  Network and Controller share data with shared data structures (passed by ref). Controller then pushes data to the GUI
    //Network class
    Network network(udpPort);

    //Show user the hostname etc
    std::string ourHostName = asio::ip::host_name();

    irr::core::stringw patienceMessage = irr::core::stringw(ourHostName.c_str());
    patienceMessage.append(L":");
    patienceMessage.append(irr::core::stringw(network.getPort()));


    //GUI class
    GUIMain guiMain(device, &language, patienceMessage);
    //Main model
    ControllerModel controller(device, &guiMain);

    //Create data structures to hold own ship, other ship and buoy data
    irr::f32 time = 0; //Time since start of day 1 of the scenario
    irr::f32 weather = 0; //(0-12)
    irr::f32 rain = 0; //(0-10)
    irr::f32 visibility = 10.1; //(0.1-10.1)
    ShipData ownShipData;

    //create event receiver, linked to model
    EventReceiver receiver(device, &controller, &guiMain, &network);
    device->setEventReceiver(&receiver);

    irr::u32 timer = device->getTimer()->getRealTime();

    //See if user has specified a mode to start with
    if (IniFile::iniFileTou32(iniFilename, "rudder")==1) {
        guiMain.setMode(true);
    } else if (IniFile::iniFileTou32(iniFilename, "heading")==1) {
        guiMain.setMode(false);
    }


    while(device->run()) {

        driver->beginScene(true, false, irr::video::SColor(0,200,200,200)); //Don't need to clear Z buffer

        //Read in data from network
        network.update(time, ownShipData);

        //Update the internal model, and call the gui
        controller.update(time, ownShipData);

        driver->endScene();

        //Pause if faster than 30 fps (33ms)
        irr::u32 newTimer = device->getTimer()->getRealTime();
        if (newTimer-timer<33) {
            device->sleep(33-(newTimer-timer));
        }
        timer = newTimer;
    }

    return(0);
}
