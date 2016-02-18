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
#include "../ScenarioDataStructure.hpp"
#include "Network.hpp"
#include "ShipPositions.hpp"


//Mac OS:
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

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



    std::string hostnames;
    std::cout << "Please enter comma separated list of multiplayer PC hostnames:" << std::endl;
    std::cin >> hostnames;

    int port = 18304; //TODO: Read in from ini file

    Network network(port);
    network.connectToServer(hostnames);

    unsigned int numberOfPeers = network.getNumberOfPeers();

    std::cout << "Connected to " << numberOfPeers << " Bridge Command peers." << std::endl;

    //Choose scenario
    std::string scenarioName = "";
    //Scenario path - default to user dir if it exists
    std::string scenarioPath = "Scenarios/";
    if (Utilities::pathExists(userFolder + scenarioPath)) {
        scenarioPath = userFolder + scenarioPath;
    }

    std::cout << "Please enter scenario name:" << std::endl;
    std::cin >> scenarioName;

    //Load overall scenario information
    ScenarioData masterScenarioData = Utilities::getScenarioDataFromFile(scenarioPath + scenarioName,scenarioName);

    irr::u32 numberOfOtherShips;
    if (masterScenarioData.otherShipsData.size() > 0) {
        numberOfOtherShips = masterScenarioData.otherShipsData.size()-1;
    } else {
        numberOfOtherShips = 0;
    }

    ShipPositions shipPositionData(numberOfOtherShips+1);

    //Get time information and initialise
    irr::f32 scenarioTime; //Simulation internal time, starting at zero at 0000h on start day of simulation
    uint64_t scenarioOffsetTime; //Simulation day's start time from unix epoch (1 Jan 1970)
    uint64_t absoluteTime; //Unix timestamp for current time, including start day. Calculated from scenarioTime and scenarioOffsetTime

    std::chrono::time_point<std::chrono::system_clock> currentTime = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> previousTime = currentTime;

    //irr::u32 currentTime = millisecs(); //Computer clock time (ms)
    //irr::u32 previousTime = currentTime; //Computer clock time (ms)
    irr::f32 accelerator = 1.0;

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
            network.sendString(thisPeerData.serialise(),true,thisPeer);

            //Store the data for this peer
            peerScenarioData.push_back(thisPeerData);

        } else {
            std::cout << "More Bridge Command peers than ships available from scenario." << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    //Start main loop, listening for updates from PCs and sending out scenario update, including time handling
    while(true) {

        //Do time handling here.
        currentTime = std::chrono::system_clock::now();
        std::chrono::duration<float> elapsedTime = currentTime-previousTime;
        previousTime = currentTime;

        float deltaTime = accelerator*(std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime)).count()/1000.0;

        scenarioTime += deltaTime;
        absoluteTime = Utilities::round(scenarioTime) + scenarioOffsetTime;

        std::cout << "Time: " << absoluteTime << std::endl;

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
            stringToSend.append("0,0#"); //Number of buoys and MOB, values not used

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
                    shipPositionData.getShipPosition(i,scenarioTime,thisOtherShipX,thisOtherShipZ,thisOtherShipSpeed,thisOtherShipBearing);
                    otherShipsString.append(Utilities::lexical_cast<std::string>(thisOtherShipX));
                    otherShipsString.append(",");
                    otherShipsString.append(Utilities::lexical_cast<std::string>(thisOtherShipZ));
                    otherShipsString.append(",");
                    otherShipsString.append(Utilities::lexical_cast<std::string>(thisOtherShipBearing));
                    otherShipsString.append(",");
                    otherShipsString.append(Utilities::lexical_cast<std::string>(thisOtherShipSpeed));
                    otherShipsString.append(",");
                    otherShipsString.append("0,0,0"); //SART enabled, number of legs,leg info
                    otherShipsString.append("|"); //End of other ship record
                }
            }
            //strip trailing '|' if present
            if(otherShipsString.length()>0) {
                otherShipsString = otherShipsString.substr(0,otherShipsString.length()-1);
            }
            stringToSend.append(otherShipsString);
            stringToSend.append("#");

            //Remaining entries need to be present, but values aren't used
            stringToSend.append("4#5#6#7#8#9#10");

            std::cout << stringToSend << std::endl;

            network.sendString(stringToSend,false,thisPeer);

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


            */

            network.listenForMessages();
            std::string receivedMessage = network.getLatestMessage(thisPeer);
            if (!receivedMessage.empty()) {
                //Do stuff with received message
            }
        } //End of loop for each peer
    } //End of main loop

    return(0);

}
