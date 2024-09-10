/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2016 James Packer

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

#include <iostream>
#include <chrono>

// Include the Irrlicht header
#include "irrlicht.h"
#include "../Utilities.hpp"
#include "../Constants.hpp"
#include "../IniFile.hpp"
#include "../ScenarioDataStructure.hpp"
#include "../Lang.hpp"
#include "ScenarioChoice.hpp"
#include "Network.hpp"
#include "ShipPositions.hpp"
#include "LinesData.hpp"
#include "EventReceiver.hpp"

#include <fstream> //To save to log

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h> // For GetSystemMetrics
#include <direct.h> //for windows _mkdir
#else
#include <sys/stat.h>
#endif // _WIN32


//Mac OS:
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#ifdef _MSC_VER
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

//Set up global for ini reader to have access to irrlicht logger if needed.
namespace IniFile {
    irr::ILogger* irrlichtLogger = 0;
}

std::string makeTimeString(uint64_t absoluteTime, uint64_t offsetTime, irr::f32 scenarioTime, irr::f32 accelerator)
{
    //timestamp (unix),
    //timestamp of start of first scenario day,
    //time since start of first scenario day (float),
    //accelerator#
    std::string timeString = Utilities::lexical_cast<std::string>(absoluteTime);
    timeString.append(",");
    timeString.append(Utilities::lexical_cast<std::string>(offsetTime));
    timeString.append(",");
    timeString.append(Utilities::lexical_cast<std::string>(scenarioTime));
    timeString.append(",");
    timeString.append(Utilities::lexical_cast<std::string>(accelerator));
    return timeString;
}

int main()
{

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
    //change up from BridgeCommand.app/Contents/MacOS/mh.app/Contents/MacOS to BridgeCommand.app/Contents/Resources
    exeFolderPath.append("/../../../../Resources");
    //change to this path now, so ini file is read
    chdir(exeFolderPath.c_str());
    //Note, we use this again after the createDevice call
	#endif

    //User read/write location - look in here first and the exe folder second for files
    std::string userFolder = Utilities::getUserDir();

    std::cout << "User folder is " << userFolder <<std::endl;

    /*Overview:
    Load scenario, including initial positions of each player
    Start as an enet client
    Connect to each multiplayer pc (each as a server)
    Send scenario information to each multiplayer pc (Tailored to include all other players as other ships)
    Send first update to each pc, based on initial positions
    Then loop:
        Get feedback from each pc for current position and heading
        Use this to update internal model
        Send out update to each pc, including other ship positions
    */

    //Read basic ini settings
    std::string iniFilename = "mph.ini";
    //Use local ini file if it exists
    if (Utilities::pathExists(userFolder + iniFilename)) {
        iniFilename = userFolder + iniFilename;
    }

    std::string modifier = IniFile::iniFileToString(iniFilename, "lang");
    if (modifier.length()==0) {
        modifier = "en"; //Default
    }
    std::string languageFile = "languageMultiplayer-";
    languageFile.append(modifier);
    languageFile.append(".txt");
    if (Utilities::pathExists(userFolder + languageFile)) {
        languageFile = userFolder + languageFile;
    }
    Lang language(languageFile);

    int fontSize = 12;
    float fontScale = IniFile::iniFileTof32(iniFilename, "font_scale");
    if (fontScale > 1) {
        fontSize = (int)(fontSize * fontScale + 0.5);
    } else {
	    fontScale = 1.0;
    }
    
    irr::u32 graphicsWidth = IniFile::iniFileTou32(iniFilename, "graphics_width");
    irr::u32 graphicsHeight = IniFile::iniFileTou32(iniFilename, "graphics_height");
    irr::u32 graphicsDepth = IniFile::iniFileTou32(iniFilename, "graphics_depth");
    int port = IniFile::iniFileTou32(iniFilename, "udp_send_port");

    // How long to pause between updates
    irr::u32 sleepTime = IniFile::iniFileTou32(iniFilename, "update_time");
    // Set defaults, and upper limit of 10s
    if (sleepTime==0) {
        sleepTime = 100;
    }
    if (sleepTime>10000) {
        sleepTime = 10000;
    }

    //Sensible defaults if not set
    irr::core::dimension2d<irr::u32> deskres;
    #ifdef _WIN32
    // Get the resolution (of the primary screen). Will be scaled as DPI unaware on Windows.
    deskres.Width=GetSystemMetrics(SM_CXSCREEN);
    deskres.Height=GetSystemMetrics(SM_CYSCREEN);
    #else
    // For other OSs, use Irrlicht's resolution call
    irr::IrrlichtDevice *nulldevice = irr::createDevice(irr::video::EDT_NULL);
    deskres = nulldevice->getVideoModeList()->getDesktopResolution();
    nulldevice->drop();
    #endif
    if (graphicsWidth==0) {
        graphicsWidth = 1200 * fontScale;
        if (graphicsWidth > deskres.Width*0.90) {
            graphicsWidth = deskres.Width*0.90;
        }
    }
    if (graphicsHeight==0) {
        graphicsHeight = 900 * fontScale;
        if (graphicsHeight > deskres.Height*0.90) {
            graphicsHeight = deskres.Height*0.90;
        }
    }
    if (graphicsDepth==0) {graphicsDepth=32;}
    if (port == 0) {port = 18304;}

    //Startup irrlicht
    //create device
    irr::SIrrlichtCreationParameters deviceParameters;
    deviceParameters.DriverType = irr::video::EDT_OPENGL;
    deviceParameters.WindowSize = irr::core::dimension2d<irr::u32>(graphicsWidth,graphicsHeight);
    deviceParameters.Bits = graphicsDepth;
    irr::IrrlichtDevice* device = irr::createDeviceEx(deviceParameters);
    device->setWindowCaption(L"Multiplayer Hub"); //Fixme - odd conversion from char* to wchar*!
    irr::video::IVideoDriver* driver = device->getVideoDriver();
    irr::scene::ISceneManager* smgr = device->getSceneManager();

    //Chdir back on OSX
    //Mac OS:
	#ifdef __APPLE__
    chdir(exeFolderPath.c_str());
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

    //Get user input for hostnames and scenario name
    std::string hostnames;
    std::string scenarioName;
    //Scenario path - default to user dir if it exists
    std::string scenarioPath = "Scenarios/";
    if (Utilities::pathExists(userFolder + scenarioPath)) {
        scenarioPath = userFolder + scenarioPath;
    }
    
    //Find default hostname if set in user directory (hostname-mp.txt)
    if (Utilities::pathExists(userFolder + "/hostname-mh.txt")) {
        hostnames=IniFile::iniFileToString(userFolder + "/hostname-mh.txt","hostname");
    }
    
    ScenarioChoice scenarioChoice(device,&language);
    scenarioChoice.chooseScenario(scenarioName,hostnames,scenarioPath);

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
    if (Utilities::pathExists(userFolder)) { 
        std::string hostnameFile = userFolder + "/hostname-mh.txt";
        std::ofstream file (hostnameFile.c_str());
        if (file.is_open()) {
            file << "hostname=" << hostnames << std::endl;
            file.close();
        }
    }


    Network network(port);
    network.connectToServer(hostnames);

    unsigned int numberOfPeers = network.getNumberOfPeers();

    std::cout << "Connected to " << numberOfPeers << " Bridge Command peers." << std::endl;


    //Load overall scenario information
    ScenarioData masterScenarioData = Utilities::getScenarioDataFromFile(scenarioPath + scenarioName,scenarioName);

    irr::u32 numberOfOtherShips; // This is the number of 'other' ships in each simulation. The total number of controllable ships is numberOfOtherShips+1
    if (masterScenarioData.otherShipsData.size() > 0) {
        numberOfOtherShips = masterScenarioData.otherShipsData.size()-1;
    } else {
        numberOfOtherShips = 0;
    }

    // These both use +1 because we are storing data for all ships. numberOfOtherShips is the number of 'other' ships in each simulation, so we need to add 1 for the 'own ship'
    ShipPositions shipPositionData(numberOfOtherShips+1);
    LinesData linesData(numberOfOtherShips+1);

    //Get time information and initialise
    irr::f32 scenarioTime; //Simulation internal time, starting at zero at 0000h on start day of simulation
    uint64_t scenarioOffsetTime; //Simulation day's start time from unix epoch (1 Jan 1970)
    uint64_t absoluteTime; //Unix timestamp for current time, including start day. Calculated from scenarioTime and scenarioOffsetTime

    std::chrono::time_point<std::chrono::system_clock> currentTime = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> previousTime = currentTime;

    //irr::u32 currentTime = millisecs(); //Computer clock time (ms)
    //irr::u32 previousTime = currentTime; //Computer clock time (ms)
    irr::f32 accelerator = 1.0;

    //Add some simple information to the GUI, so the user knows it's running
    //Add text, which will list connected peers, and current time.
    irr::u32 su = driver->getScreenSize().Width;
    irr::u32 sh = driver->getScreenSize().Height;
    irr::gui::IGUIStaticText* text = device->getGUIEnvironment()->addStaticText(L"",irr::core::rect<irr::s32>(0.01*su,0.01*sh,0.99*su,0.74*sh),true);

    // Add run and pause buttons
    irr::s32 runButtonID = 101;
    irr::s32 pauseButtonID = 102;
    irr::gui::IGUIButton* runButton = device->getGUIEnvironment()->addButton(irr::core::rect<irr::s32>(0.01*su,0.76*sh,0.49*su,0.99*sh), 0, runButtonID, language.translate("run").c_str());
    irr::gui::IGUIButton* pauseButton = device->getGUIEnvironment()->addButton(irr::core::rect<irr::s32>(0.51*su,0.76*sh,0.99*su,0.99*sh), 0, pauseButtonID, language.translate("pause").c_str());

    // Setup event receiver
    EventReceiver eventReceiver(pauseButtonID, runButtonID, accelerator);
    device->setEventReceiver(&eventReceiver);

    //Fixme: Think about time zone handling
    //Fixme: Note that if the time_t isn't long enough, 2038 problem exists
    scenarioOffsetTime = Utilities::dmyToTimestamp(masterScenarioData.startDay,masterScenarioData.startMonth,masterScenarioData.startYear);//Time in seconds to start of scenario day (unix timestamp for 0000h on day scenario starts)
    scenarioTime = masterScenarioData.startTime * SECONDS_IN_HOUR; //set internal scenario time to start
    absoluteTime = Utilities::round(scenarioTime) + scenarioOffsetTime;

    //for each peer, build basic scenario information (own ship, other ships, excluding this one)
    std::vector<ScenarioData> peerScenarioData;
    for(unsigned int thisPeer = 0; thisPeer<numberOfPeers; thisPeer++ ) {
        //Own ship data gets populated from other ship (including 1st leg if it exists
        if (masterScenarioData.otherShipsData.size() > thisPeer) {

            ScenarioData thisPeerData  = masterScenarioData;

            thisPeerData.ownShipData.ownShipName = thisPeerData.otherShipsData.at(thisPeer).shipName;
            thisPeerData.ownShipData.initialLat = thisPeerData.otherShipsData.at(thisPeer).initialLat;
            thisPeerData.ownShipData.initialLong = thisPeerData.otherShipsData.at(thisPeer).initialLong;
            if (thisPeerData.otherShipsData.at(thisPeer).legs.size()>0) {
                thisPeerData.ownShipData.initialSpeed = thisPeerData.otherShipsData.at(thisPeer).legs.at(0).speed;
                thisPeerData.ownShipData.initialBearing = thisPeerData.otherShipsData.at(thisPeer).legs.at(0).bearing;
            } else {
                thisPeerData.ownShipData.initialSpeed = 0;
                thisPeerData.ownShipData.initialBearing = 0;
            }
            //remove thisPeerData.otherShipsData.at(thisPeer)
            thisPeerData.otherShipsData.erase(thisPeerData.otherShipsData.begin()+thisPeer);

            //Send initial scenario information (reliable packet)
            network.sendString(thisPeerData.serialise(false),true,thisPeer);

            //Store the data for this peer
            peerScenarioData.push_back(thisPeerData);

        } else {
            std::cout << "More Bridge Command peers than ships available from scenario." << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    //Start main loop, listening for updates from PCs and sending out scenario update, including time handling
    while(device->run())
    {

        driver->beginScene(true, true, irr::video::SColor(0,128,128,128));

        // Pause, so we don't flood clients with data
        device->sleep(sleepTime);

        // Find current time acceleration
        accelerator = eventReceiver.getAccelerator();

        //Do time handling here.
        currentTime = std::chrono::system_clock::now();
        std::chrono::duration<float> elapsedTime = currentTime-previousTime;
        previousTime = currentTime;

        float deltaTime = accelerator*(std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime)).count()/1000.0;

        scenarioTime += deltaTime;
        absoluteTime = Utilities::round(scenarioTime) + scenarioOffsetTime;

        std::string timeString = makeTimeString(absoluteTime,scenarioOffsetTime,scenarioTime,accelerator);

        //for each peer
        for(unsigned int thisPeer = 0; thisPeer<numberOfPeers; thisPeer++ ) {

            std::string stringToSend = "BC";

            //0: Time info
            stringToSend.append(timeString);
            stringToSend.append("#");

            //1: Own ship info: Not used
            stringToSend.append("0#");

            //2: Number of other ships: Size of master other ships list -1, as we don't count the one being used as our own ship
            stringToSend.append(Utilities::lexical_cast<std::string>(numberOfOtherShips));
            stringToSend.append(",");
            stringToSend.append("0,0,"); //Number of buoys and MOB, values not used
            stringToSend.append(Utilities::lexical_cast<std::string>(linesData.getNumberOfOtherLines(thisPeer))); // Number of lines (mooring/towing)
            stringToSend.append("#"); //Terminate number info

            //3: Info on each other ship
            //For each Other, terminated with '#' at end of list
            //    PosX,PosZ,Heading,speed (kts),0(SART), 0 (Number of legs, 0 as we don't need leg info in multiplayer)|
            std::string otherShipsString;
            for(unsigned int i = 0; i < (numberOfOtherShips+1); i++) {
                if (i!=thisPeer) {
                    irr::f32 thisOtherShipX = 0;
                    irr::f32 thisOtherShipZ = 0;
                    irr::f32 thisOtherShipSpeed = 0;
                    irr::f32 thisOtherShipBearing = 0;
                    irr::f32 thisOtherShipRateOfTurn = 0;

                    shipPositionData.getShipPosition(i,
                                                     scenarioTime,
                                                     thisOtherShipX,
                                                     thisOtherShipZ,
                                                     thisOtherShipSpeed,
                                                     thisOtherShipBearing,
                                                     thisOtherShipRateOfTurn);

                    otherShipsString.append(Utilities::lexical_cast<std::string>(thisOtherShipX));
                    otherShipsString.append(",");
                    otherShipsString.append(Utilities::lexical_cast<std::string>(thisOtherShipZ));
                    otherShipsString.append(",");
                    otherShipsString.append(Utilities::lexical_cast<std::string>(thisOtherShipBearing));
                    otherShipsString.append(",");
                    otherShipsString.append(Utilities::lexical_cast<std::string>(thisOtherShipSpeed*MPS_TO_KTS));
                    otherShipsString.append(",");

                    // TODO: Send Rate of turn here
                    otherShipsString.append(Utilities::lexical_cast<std::string>(thisOtherShipRateOfTurn));
                    otherShipsString.append(",");

                    otherShipsString.append("0,0,0,0"); //SART enabled, MMSI, number of legs,leg info. TODO: Can we get MMSI
                    otherShipsString.append("|"); //End of other ship record
                }
            }
            //strip trailing '|' if present
            if(otherShipsString.length()>0) {
                otherShipsString = otherShipsString.substr(0,otherShipsString.length()-1);
            }
            stringToSend.append(otherShipsString);
            stringToSend.append("#");

            //Intermediate entries need to be present, but values aren't used
            stringToSend.append("4#5#6#7#8#9#10#");

            //11: Lines information (mooring/towing)
            std::string linesString = linesData.getLineDataString(thisPeer);
            //strip trailing '|' if present
            if(linesString.length()>0) {
                linesString = linesString.substr(0,linesString.length()-1);
            }
            stringToSend.append(linesString);

            //12: 13 basic records in data sent, entry 12 is engine and wheel data for secondary controls, not needed, so send blank entry
            stringToSend.append("12#");

            //std::cout << stringToSend << std::endl;

            network.sendString(stringToSend,false,thisPeer);

            //std::cout << "Sending to peer " << thisPeer << " Message:" << stringToSend << std::endl;

            /*
            For multiplayer, only actually uses info from records 0 (time), 2 (Number of entities) & 3 (Other ship info). BC Checks number of entries, so just need dummies
            Format is (with added newlines):
            BC

            (0) timestamp (unix), timestamp of start of first scenario day,
            time since start of first scenario day (float), accelerator#

            (1, own ship data not used in multiplayer, so can leave as 0#)
            Pos x, Pos z, heading, rate of turn, pitch, roll, SOG (knots), COG#

            (2) Number other, number buoys, number MOB (0)#

            (3) For each Other, terminated with '#' at end of list
                PosX,PosZ,Heading,speed (kts),0(SART), 0 (Number of legs, 0 as we don't need leg info in multiplayer)|

            Records 4 to 10 not used (separate with '#')

            (11) Lines information...


            */

            network.listenForMessages();
            std::string receivedMessage = network.getLatestMessage(thisPeer);
            if (receivedMessage.length() > 3 && receivedMessage.substr(0,3) == "MPF") { //Starts with 'MPF' for multiplayer feedback
                receivedMessage = receivedMessage.substr(3,receivedMessage.length()-3); //Strip 'MPF'
                std::vector<std::string> splitMessage = Utilities::split(receivedMessage,'#');
                //Store information
                if (splitMessage.size() == 7) {
                    irr::f32 thisOtherShipX = Utilities::lexical_cast<irr::f32>(splitMessage.at(0));
                    irr::f32 thisOtherShipZ = Utilities::lexical_cast<irr::f32>(splitMessage.at(1));
                    irr::f32 thisOtherShipBearing = Utilities::lexical_cast<irr::f32>(splitMessage.at(2));
                    irr::f32 thisOtherShipRateOfTurn = Utilities::lexical_cast<irr::f32>(splitMessage.at(3)); // deg/s
                    irr::f32 thisOtherShipSpeed = Utilities::lexical_cast<irr::f32>(splitMessage.at(4));
                    irr::f32 thisOtherShipTime = Utilities::lexical_cast<irr::f32>(splitMessage.at(5));
                    shipPositionData.setShipPosition(thisPeer,thisOtherShipTime,thisOtherShipX,thisOtherShipZ,thisOtherShipSpeed,thisOtherShipBearing,thisOtherShipRateOfTurn);

                    // Lines data, from splitMessage.at(6)
                    std::vector<std::string> linesDataString = Utilities::split(splitMessage.at(6),'|');

                    //std::cout << "Message in from " << thisPeer << ": " << splitMessage.at(6) << std::endl;
                    
                    // If necessary, add or delete line
                    linesData.setLineDataSize(thisPeer, linesDataString.size());
                    
                    // Update line information
                    for (int lineID = 0; lineID < linesDataString.size(); lineID++) {
                        std::vector<std::string> thisLineData = Utilities::split(linesDataString.at(lineID),',');
                        // Check number of elements for line data line
                        if (thisLineData.size() == 16) {
                            irr::f32 thisStartX = Utilities::lexical_cast<irr::f32>(thisLineData.at(0));
                            irr::f32 thisStartY = Utilities::lexical_cast<irr::f32>(thisLineData.at(1));
                            irr::f32 thisStartZ = Utilities::lexical_cast<irr::f32>(thisLineData.at(2));
                            irr::f32 thisEndX = Utilities::lexical_cast<irr::f32>(thisLineData.at(3));
                            irr::f32 thisEndY = Utilities::lexical_cast<irr::f32>(thisLineData.at(4));
                            irr::f32 thisEndZ = Utilities::lexical_cast<irr::f32>(thisLineData.at(5));
                            int thisStartType = Utilities::lexical_cast<int>(thisLineData.at(6));
                            int thisEndType = Utilities::lexical_cast<int>(thisLineData.at(7));
                            int thisStartID = Utilities::lexical_cast<int>(thisLineData.at(8));
                            int thisEndID = Utilities::lexical_cast<int>(thisLineData.at(9));
                            irr::f32 thisNominalLength = Utilities::lexical_cast<irr::f32>(thisLineData.at(10));
                            irr::f32 thisBreakingTension = Utilities::lexical_cast<irr::f32>(thisLineData.at(11));
                            irr::f32 thisBreakingStrain = Utilities::lexical_cast<irr::f32>(thisLineData.at(12));
                            irr::f32 thisNominalShipMass = Utilities::lexical_cast<irr::f32>(thisLineData.at(13));
                            int thisKeepSlack = Utilities::lexical_cast<int>(thisLineData.at(14));
                            int thisHeaveIn = Utilities::lexical_cast<int>(thisLineData.at(15));

                            // Modify start and end data to internal IDs (e.g. all ownShip will change to otherShip)
                            if (thisStartType == 1) {
                                // Own ship
                                thisStartType = 2;
                                thisStartID = thisPeer;
                            } else if (thisStartType == 2) {
                                // Other ship. Essential to use else if, to avoid accidentally changing own ship again
                                // Convert from local thisStartID (as used by thisPeer) to the overall list of other ships in the multiplayer scenario
                                if (thisStartID >= thisPeer) {
                                    thisStartID = thisStartID + 1;
                                }
                            }
                            if (thisEndType == 1) {
                                // Own ship
                                thisEndType = 2;
                                thisEndID = thisPeer;
                            } else if (thisEndType == 2) {
                                // Other ship. Essential to use else if, to avoid accidentally changing own ship again
                                // Convert from local thisStartID (as used by thisPeer) to the overall list of other ships in the multiplayer scenario
                                if (thisEndID >= thisPeer) {
                                    thisEndID = thisEndID + 1;
                                }
                            }

                            linesData.setLineData(thisPeer, lineID, 
                                                  thisStartType, thisEndType, thisStartID, thisEndID, 
                                                  thisKeepSlack,
                                                  thisHeaveIn, 
                                                  thisStartX, thisStartY, thisStartZ, 
                                                  thisEndX, thisEndY, thisEndZ, 
                                                  thisNominalLength, 
                                                  thisBreakingTension, 
                                                  thisBreakingStrain, 
                                                  thisNominalShipMass);
                        }
                    }
                    // End lines data
                } // Correct number of entries in MPF
            } // MPF record
        } //End of loop for each peer


        //TODO:
        //Update gui time here, using Utilities::timestampToString(absoluteTime)
        std::string displayTime = Utilities::timestampToString(absoluteTime);
        irr::core::stringw displayText = language.translate("time");
        displayText.append(L" ");
        displayText.append(irr::core::stringw(displayTime.c_str()));
        displayText.append(L"\n\n");
        for(unsigned int i = 0; i<numberOfPeers; i++ ) {
            irr::f32 thisOtherShipX = 0;
            irr::f32 thisOtherShipZ = 0;
            irr::f32 thisOtherShipSpeed = 0;
            irr::f32 thisOtherShipBearing = 0;
            irr::f32 thisOtherShipRateOfTurn = 0;

            shipPositionData.getShipPosition(i,scenarioTime,thisOtherShipX,thisOtherShipZ,thisOtherShipSpeed,thisOtherShipBearing,thisOtherShipRateOfTurn);
            std::string thisShipNumber = Utilities::lexical_cast<std::string>(i+1);
            std::string stringSpeed = Utilities::lexical_cast<std::string>(thisOtherShipSpeed*MPS_TO_KTS); //Should be in knots
            std::string stringHeading = Utilities::lexical_cast<std::string>(thisOtherShipBearing);
            irr::core::stringw thisShipInfo = language.translate("ship");
            thisShipInfo.append(L": ");
            thisShipInfo.append(irr::core::stringw(thisShipNumber.c_str()));
            thisShipInfo.append(L"\n");
            thisShipInfo.append(language.translate("speed"));
            thisShipInfo.append(L" ");
            thisShipInfo.append(irr::core::stringw(stringSpeed.c_str()));
            thisShipInfo.append(L" ");
            thisShipInfo.append(language.translate("knots"));
            thisShipInfo.append(L" ");
            thisShipInfo.append(language.translate("heading"));
            thisShipInfo.append(L" ");
            thisShipInfo.append(irr::core::stringw(stringHeading.c_str()));
            thisShipInfo.append(L"\n");

            displayText.append(thisShipInfo);
        }
        displayText.append(L"\n");
        displayText.append(irr::core::stringw(linesData.getNumberOfLines()));


        text->setText(displayText.c_str());

        smgr->drawAll();
        device->getGUIEnvironment()->drawAll();
        driver->endScene();

    } //End of main loop

    return(0);

}
