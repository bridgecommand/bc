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

#include <iostream>
#include <cstdio>
#include <vector>

#include "NetworkSecondary.hpp"
#include "SimulationModel.hpp"
#include "Utilities.hpp"
#include "Constants.hpp"

NetworkSecondary::NetworkSecondary(int port, OperatingMode::Mode mode, irr::IrrlichtDevice* dev)
{
    server = 0;
    device = dev;

    ENetAddress address;
    this->mode = mode;
    model=0; //Not linked at the moment

    accelAdjustment = 0;
    previousTimeError = 0;

    if (enet_initialize () != 0)
    {
        std::cerr << "An error occurred while initializing ENet.\n";
        exit(EXIT_FAILURE);
    }

    /* Bind the server to the default localhost. */
    /* A specific host address can be specified by */
    /* enet_address_set_host (& address, "x.x.x.x"); */
    address.host = ENET_HOST_ANY;
    /* Bind the server to port XXXXX. */

    int tries=0;

    while (server==NULL && tries < 10) {
        address.port = port;
        server = enet_host_create (& address /* the address to bind the server host to */,
        32 /* allow up to 32 clients and/or outgoing connections */,
        0 /* allow maximum number of channels */,
        0 /* assume any amount of incoming bandwidth */,
        0 /* assume any amount of outgoing bandwidth */);

        //Update for next attempt if needed
        if (server==NULL) {
            tries++;
            port++;
        }
    }

    if (server == NULL)
    {
        std::cerr << "An error occurred while trying to create an ENet server host." << std::endl;
		enet_deinitialize();
		exit (EXIT_FAILURE);
    }


}

NetworkSecondary::~NetworkSecondary()
{
    enet_host_destroy(server);
    enet_deinitialize();
}

void NetworkSecondary::connectToServer(std::string hostnames)
{
    //Don't need to do anything
}

void NetworkSecondary::getScenarioFromNetwork(std::string& dataString) //Not used by primary
{
     if (enet_host_service (server, & event, 10) > 0) { //Wait 10ms for event
        if (event.type ==ENET_EVENT_TYPE_RECEIVE) {

            //receive it
            char tempString[8192]; //Fixme: Think if this is long enough
            snprintf(tempString,8192,"%s",event.packet -> data);
            std::string receivedString(tempString);

            //Basic checks
            if (receivedString.length() > 4) { //Check if more than 4 chars long, ie we have at least some data
                if (receivedString.substr(0,4).compare("SCN1") == 0 ) { //Check if it starts with SCN1
                    //If valid, use this string
                    dataString = receivedString;
                }
            }
        }
        /* Clean up the packet now that we're done using it. */
        enet_packet_destroy (event.packet);
     }
}

void NetworkSecondary::setModel(SimulationModel* model) //This MUST be called before update()
{
    this->model = model;
}

int NetworkSecondary::getPort()
{
    if (server) {
        return server->address.port;
    }
    return 0;
}

void NetworkSecondary::update()
{

    if (model==0) {
        std::cerr << "Network not linked to model" << std::endl;
        return;
    }

    /* Wait up to 10 milliseconds for an event. */
    while (enet_host_service (server, & event, 10) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                printf ("A new client connected from %x:%u.\n",
                    event.peer->address.host,
                    event.peer->address.port);
                /* Store any relevant client information here. */
                //event.peer -> data = "Client information";
                break;
            case ENET_EVENT_TYPE_RECEIVE:

                receiveMessage(); //Process and use the received message

                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy (event.packet);

                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                printf ("%s disconected.\n", (char*)event.peer -> data);
                /* Reset the peer's client information. */
                event.peer -> data = NULL;
                break;
            default:
                break;
        }
    }
}

void NetworkSecondary::receiveMessage()
{
    //receive it
    char tempString[8192]; //Fixme: Think if this is long enough
    snprintf(tempString,8192,"%s",event.packet -> data);
    std::string receivedString(tempString);

    //std::cout << "Received data:" << receivedStrings << std::endl;

    //Basic checks
    if (receivedString.length() > 2) { //Check if more than 2 chars long, ie we have at least some data
        if (receivedString.substr(0,2).compare("BC") == 0 ) { //Check if it starts with BC
            //Strip 'BC'
            receivedString = receivedString.substr(2,receivedString.length()-2);

            //Split into main parts
            std::vector<std::string> receivedData = Utilities::split(receivedString,'#');

            //Check number of elements
            if (receivedData.size() == 13) { //13 basic records in data sent

                irr::f32 timeError;

                //Get time info from record 0
                std::vector<std::string> timeData = Utilities::split(receivedData.at(0),',');
                //Time since start of scenario day 1 is record 2
                if (timeData.size() > 2) {
                    timeError = Utilities::lexical_cast<irr::f32>(timeData.at(2)) - model->getTimeDelta(); //How far we are behind the master
                    irr::f32 baseAccelerator = Utilities::lexical_cast<irr::f32>(timeData.at(3)); //The master accelerator setting
                    if (fabs(timeError) > 1) {
                        //Big time difference, so reset
                        model->setTimeDelta(Utilities::lexical_cast<irr::f32>(timeData.at(2)));
                        accelAdjustment = 0;
                        device->getLogger()->log("Resetting time alignment");
                    } else { //Adjust accelerator to maintain time alignment
                        accelAdjustment += timeError*0.01; //Integral only at the moment
                        //Check for zero crossing, and reset
                        if (previousTimeError * timeError < 0) {
                            accelAdjustment = 0;
                        }
                        if (baseAccelerator + accelAdjustment < 0) {accelAdjustment= -1*baseAccelerator;}//Saturate at zero
                    }
                    model->setAccelerator(baseAccelerator + accelAdjustment);
                    previousTimeError = timeError; //Store for next time
                }

                //Get own ship position info from record 1, if in secondary mode (not used in multiplayer)
                if (mode==OperatingMode::Secondary) {
                    std::vector<std::string> positionData = Utilities::split(receivedData.at(1),',');
                    if (positionData.size() == 9) { //9 elements in position data sent
                        model->setPos(Utilities::lexical_cast<irr::f32>(positionData.at(0)),
                                    Utilities::lexical_cast<irr::f32>(positionData.at(1)));
                        model->setHeading(Utilities::lexical_cast<irr::f32>(positionData.at(2)));
                        model->setRateOfTurn(Utilities::lexical_cast<irr::f32>(positionData.at(3)));
                        model->setSpeed(Utilities::lexical_cast<irr::f32>(positionData.at(6))/MPS_TO_KTS);
                    }
                }

                //Start
                //Get other ship info from records 2 and 3
                //Numbers of objects in record 2 (Others, buoys, MOBs, lines)
                std::vector<std::string> numberData = Utilities::split(receivedData.at(2),',');
                if (numberData.size() == 4) {
                    irr::u32 numberOthers = Utilities::lexical_cast<irr::u32>(numberData.at(0));

                    //Update other ship data
                    std::vector<std::string> otherShipsDataString = Utilities::split(receivedData.at(3),'|');
                    if (numberOthers == otherShipsDataString.size()) {
                        for (irr::u32 i=0; i<otherShipsDataString.size(); i++) {
                            std::vector<std::string> thisShipData = Utilities::split(otherShipsDataString.at(i),',');
                            if (thisShipData.size() == 9) { //9 elements for each ship
                                //Update data
                                model->setOtherShipHeading(i,Utilities::lexical_cast<irr::f32>(thisShipData.at(2)));
                                model->setOtherShipSpeed(i,Utilities::lexical_cast<irr::f32>(thisShipData.at(3))/MPS_TO_KTS);

                                // Set other ship rate of turn from thisShipData.at(4) in deg/s (only used in multiplayer)
                                model->setOtherShipRateOfTurn(i,Utilities::lexical_cast<irr::f32>(thisShipData.at(4)));

                                irr::f32 receivedPosX = Utilities::lexical_cast<irr::f32>(thisShipData.at(0));
                                irr::f32 receivedPosZ = Utilities::lexical_cast<irr::f32>(thisShipData.at(1));
                                model->setOtherShipPos(i,receivedPosX,receivedPosZ);
                                //Todo: Think about using timeError to extrapolate position to get more accurately.
                                //Todo: use SART etc
                            }
                        }
                    }

                    //Update MOB data
                    irr::u32 numberMOB = Utilities::lexical_cast<irr::u32>(numberData.at(2));
                    if (numberMOB==1) {
                        //MOB should be visible, find if we have an MOB position record with two items (record 5)
                        //TODO: TEST!
                        std::vector<std::string> mobData = Utilities::split(receivedData.at(5),',');
                        if (mobData.size()==2) {
                            model->setManOverboardVisible(true);
                            model->setManOverboardPos(  Utilities::lexical_cast<irr::f32>(mobData.at(0)),
                                                        Utilities::lexical_cast<irr::f32>(mobData.at(1)));
                        }
                    } else if (numberMOB==0) {
                        model->setManOverboardVisible(false);
                    }

                    irr::u32 numberLines = Utilities::lexical_cast<irr::u32>(numberData.at(3));
                    // Update lines information
                    if (numberLines > 0) {

                        // Check if numberLines is different from the number of lines being shown (on network lines list)
                        if (numberLines > model->getLines()->getNumberOfLines(true)) {
                            // Need to add lines, initially undefined
                            while (numberLines > model->getLines()->getNumberOfLines(true)) {
                                model->getLines()->addLine(true);
                            }
                        } else if (numberLines < model->getLines()->getNumberOfLines(true)) {
                            // Need to remove lines
                            while (numberLines < model->getLines()->getNumberOfLines(true)) {
                                // We don't really know which to remove, so just remove from start, and update as needed later
                                model->getLines()->removeLine(0, true);
                            }
                        }

                        // For each line, check that data is correct, and if not, update it
                        std::vector<std::string> linesDataString = Utilities::split(receivedData.at(11),'|');
                        // Additional check, that the number of lines matches the number of data entries
                        if (numberLines == linesDataString.size()) {

                            for (irr::u32 i=0; i<linesDataString.size(); i++) {
                                std::vector<std::string> thisLineData = Utilities::split(linesDataString.at(i),',');
                                if (thisLineData.size() == 16) { //16 elements for each line

                                    irr::f32 lineStartX = Utilities::lexical_cast<irr::f32>(thisLineData.at(0));
                                    irr::f32 lineStartY = Utilities::lexical_cast<irr::f32>(thisLineData.at(1));
                                    irr::f32 lineStartZ = Utilities::lexical_cast<irr::f32>(thisLineData.at(2));
                                    irr::f32 lineEndX = Utilities::lexical_cast<irr::f32>(thisLineData.at(3));
                                    irr::f32 lineEndY = Utilities::lexical_cast<irr::f32>(thisLineData.at(4));
                                    irr::f32 lineEndZ = Utilities::lexical_cast<irr::f32>(thisLineData.at(5));

                                    int lineStartType = Utilities::lexical_cast<int>(thisLineData.at(6));
                                    int lineEndType = Utilities::lexical_cast<int>(thisLineData.at(7));
                                    int lineStartID = Utilities::lexical_cast<int>(thisLineData.at(8));
                                    int lineEndID = Utilities::lexical_cast<int>(thisLineData.at(9));

                                    irr::f32 lineNominalLength = Utilities::lexical_cast<irr::f32>(thisLineData.at(10));
                                    irr::f32 lineBreakingTension = Utilities::lexical_cast<irr::f32>(thisLineData.at(11));
                                    irr::f32 lineBreakingStrain = Utilities::lexical_cast<irr::f32>(thisLineData.at(12));
                                    irr::f32 lineNominalShipMass = Utilities::lexical_cast<irr::f32>(thisLineData.at(13));

                                    int lineKeepSlackInt = Utilities::lexical_cast<int>(thisLineData.at(14));
                                    int lineHeaveInInt = Utilities::lexical_cast<int>(thisLineData.at(15));

                                    bool lineKeepSlack;
                                    if (lineKeepSlackInt == 1) {
                                        lineKeepSlack = true;
                                    } else {
                                        lineKeepSlack = false;
                                    }

                                    bool lineHeaveIn;
                                    if (lineHeaveInInt == 1) {
                                        lineHeaveIn = true;
                                    } else {
                                        lineHeaveIn = false;
                                    }

                                    // Check if lineStartType, lineStartID, lineEndType, lineEndID match. If not, clearLine, and re-create
                                    if ((model->getLines()->getLineStartType(i, true) != lineStartType) ||
                                        (model->getLines()->getLineEndType(i, true) != lineEndType) ||
                                        (model->getLines()->getLineStartID(i, true) != lineStartID) ||
                                        (model->getLines()->getLineEndID(i, true) != lineEndID))
                                    {
                                        model->getLines()->clearLine(i, true);

                                        if ((lineStartType > 0) && (lineEndType > 0)) {
                                            // Create Spheres for the start and end scene nodes, remember to match parent name for completeness
                                            // find start parent sceneNode
                                            // find end parent sceneNode

                                            irr::scene::ISceneNode* startParent = 0;
                                            irr::scene::ISceneNode* endParent = 0;
                                            if (lineStartType == 1) {
                                                // Own ship
                                                startParent = model->getOwnShipSceneNode();
                                            } else if (lineStartType == 2) {
                                                // Other ship
                                                startParent = model->getOtherShipSceneNode(lineStartID);
                                            } else if (lineStartType == 3) {
                                                // Buoy
                                                startParent = model->getBuoySceneNode(lineStartID);
                                            } else if (lineStartType == 4) {
                                                // Land object
                                                startParent = model->getLandObjectSceneNode(lineStartID);
                                            }

                                            if (lineEndType == 1) {
                                                // Own ship
                                                endParent = model->getOwnShipSceneNode();
                                            } else if (lineEndType == 2) {
                                                // Other ship
                                                endParent = model->getOtherShipSceneNode(lineEndID);
                                            } else if (lineEndType == 3) {
                                                // Buoy
                                                endParent = model->getBuoySceneNode(lineEndID);
                                            } else if (lineEndType == 4) {
                                                // Land object
                                                endParent = model->getLandObjectSceneNode(lineEndID);
                                            }

                                            // Make child sphere nodes based on these (in the right position), then pass in to create the lines
                                            irr::core::vector3df sphereScale = irr::core::vector3df(1.0, 1.0, 1.0);
                                            if (startParent && startParent->getScale().X > 0) {
                                                sphereScale = irr::core::vector3df(1.0f/startParent->getScale().X,
                                                                                1.0f/startParent->getScale().X,
                                                                                1.0f/startParent->getScale().X);
                                            }
                                            irr::scene::ISceneNode* startNode = device->getSceneManager()->addSphereSceneNode(0.25f,16,startParent,-1,
                                                                                        irr::core::vector3df(lineStartX, lineStartY, lineStartZ),
                                                                                        irr::core::vector3df(0, 0, 0),
                                                                                        sphereScale);
                                            sphereScale = irr::core::vector3df(1.0, 1.0, 1.0);
                                            if (endParent && endParent->getScale().X > 0) {
                                                sphereScale = irr::core::vector3df(1.0f/endParent->getScale().X,
                                                                                1.0f/endParent->getScale().X,
                                                                                1.0f/endParent->getScale().X);
                                            }
                                            irr::scene::ISceneNode* endNode = device->getSceneManager()->addSphereSceneNode(0.25f,16,endParent,-1,
                                                                                        irr::core::vector3df(lineEndX, lineEndY, lineEndZ),
                                                                                        irr::core::vector3df(0, 0, 0),
                                                                                        sphereScale);

                                            // Set name to match parent for convenience
                                            if (startParent && startNode) {
                                                startNode->setName(startParent->getName());
                                            }
                                            if (endParent && endNode) {
                                                endNode->setName(endParent->getName());
                                            }

                                            // Create the lines
                                            model->getLines()->setLineStart(startNode, lineStartType, lineStartID, true, i);
                                            model->getLines()->setLineEnd(endNode, lineNominalShipMass, lineEndType, lineEndID, true, i);
                                        }

                                    }

                                    // Check and update other parameters
                                    model->getLines()->setKeepSlack(i, lineKeepSlack, true);
                                    model->getLines()->setHeaveIn(i, lineHeaveIn, true);
                                    model->getLines()->setLineNominalLength(i, lineNominalLength, true);
                                    model->getLines()->setLineBreakingTension(i, lineBreakingTension, true);
                                    model->getLines()->setLineBreakingStrain(i, lineBreakingStrain, true);
                                    model->getLines()->setLineNominalShipMass(i, lineNominalShipMass, true);

                                    // TODO: Think about case where start or end positions have changed, but object/ID havent?

                                }
                            }
                        }

                    } else {
                        // No lines, so remove all from network list
                        while (model->getLines()->getNumberOfLines(true) > 0) {
                            model->getLines()->removeLine(0, true);
                        }
                    }


                } //Check if 4 number elements for Other ships, buoys, MOBs and lines

                //Get weather info from record 7
                //0 is weather, 1 is visibility, 3 is rain
                std::vector<std::string> weatherData = Utilities::split(receivedData.at(7),',');
                if (weatherData.size() == 5) {
                    model->setWeather(Utilities::lexical_cast<irr::f32>(weatherData.at(0)));
                    model->setVisibility(Utilities::lexical_cast<irr::f32>(weatherData.at(1)));
                    model->setRain(Utilities::lexical_cast<irr::f32>(weatherData.at(3)));
                }

                //Get view information from record 9
                std::vector<std::string> viewData = Utilities::split(receivedData.at(9),',');
                if (viewData.size() == 1) {
                    if (model->getMoveViewWithPrimary()) {
                        model->setView(Utilities::lexical_cast<irr::f32>(viewData.at(0)));
                    }
                }

                // Get engine control information from record 12, only used for display 
                std::vector<std::string> controlsData = Utilities::split(receivedData.at(12),',');
                if (controlsData.size() == 10) {
                    // Only set display values if we are not already controlling it
                    if (!model->getIsSecondaryControlWheel()) {
                        model->setWheel(Utilities::lexical_cast<irr::f32>(controlsData.at(0)));
                    }
                    // Rudder can always be set, as rudder motion is always set by primary
                    model->setRudder(Utilities::lexical_cast<irr::f32>(controlsData.at(1)));
                    if (!model->getIsSecondaryControlPortEngine()) {
                        model->setPortEngine(Utilities::lexical_cast<irr::f32>(controlsData.at(2)));
                    }
                    if (!model->getIsSecondaryControlStbdEngine()) {
                        model->setStbdEngine(Utilities::lexical_cast<irr::f32>(controlsData.at(3)));
                    }
                    if (!model->getIsSecondaryControlPortSchottel()) {
                        model->setPortSchottel(Utilities::lexical_cast<irr::f32>(controlsData.at(4)));
                    }
                    if (!model->getIsSecondaryControlStbdSchottel()) {
                        model->setStbdSchottel(Utilities::lexical_cast<irr::f32>(controlsData.at(5)));
                    }
                    if (!model->getIsSecondaryControlPortThrustLever()) {
                        model->setPortAzimuthThrustLever(Utilities::lexical_cast<irr::f32>(controlsData.at(6)));
                    }
                    if (!model->getIsSecondaryControlStbdThrustLever()) {
                        model->setStbdAzimuthThrustLever(Utilities::lexical_cast<irr::f32>(controlsData.at(7)));
                    }
                    if (!model->getIsSecondaryControlBowThruster()) {
                        model->setBowThruster(Utilities::lexical_cast<irr::f32>(controlsData.at(8)));
                    }
                    if (!model->getIsSecondaryControlSternThruster()) {
                        model->setSternThruster(Utilities::lexical_cast<irr::f32>(controlsData.at(9)));
                    }
                }

                //Todo: Think about how to get best synchronisation (and movement between updates, speed etc)

                //If in multiplayer mode, send back a message with our position and heading
                if (mode==OperatingMode::Multiplayer) {

                    std::string multiplayerFeedback = "MPF";
                    multiplayerFeedback.append(Utilities::lexical_cast<std::string>(model->getPosX()));
                    multiplayerFeedback.append("#");
                    multiplayerFeedback.append(Utilities::lexical_cast<std::string>(model->getPosZ()));
                    multiplayerFeedback.append("#");
                    multiplayerFeedback.append(Utilities::lexical_cast<std::string>(model->getHeading()));
                    multiplayerFeedback.append("#");
                    multiplayerFeedback.append(Utilities::lexical_cast<std::string>(model->getRateOfTurn()*irr::core::RADTODEG));
                    multiplayerFeedback.append("#");
                    multiplayerFeedback.append(Utilities::lexical_cast<std::string>(model->getSpeed()));
                    multiplayerFeedback.append("#");
                    multiplayerFeedback.append(Utilities::lexical_cast<std::string>(model->getTimeDelta()));
                    multiplayerFeedback.append("#");

                    // Mooring/towing lines
                    multiplayerFeedback.append(makeNetworkLinesString(model));

                    //Send back to event.peer
                    ENetPacket* packet = enet_packet_create (multiplayerFeedback.c_str(), strlen (multiplayerFeedback.c_str()) + 1,0/*reliable flag*/);
                    if (packet!=0) {
                        enet_peer_send (event.peer, 0, packet);
                        enet_host_flush (server);
                    }
                }

                // If overriding control input to the primary, send back a message with the thrust lever and wheel controls etc
                std::string controlOverride = "";
                if (model->getIsSecondaryControlWheel()) {
                    controlOverride.append("MCCO,0,");
                    controlOverride.append(Utilities::lexical_cast<std::string>(model->getWheel()));
                    controlOverride.append("|");
                }
                if (model->getIsSecondaryControlPortEngine()) {
                    controlOverride.append("MCCO,1,");
                    controlOverride.append(Utilities::lexical_cast<std::string>(model->getPortEngine()));
                    controlOverride.append("|");
                }
                if (model->getIsSecondaryControlStbdEngine()) {
                    controlOverride.append("MCCO,2,");
                    controlOverride.append(Utilities::lexical_cast<std::string>(model->getStbdEngine()));
                    controlOverride.append("|");
                }
                if (model->getIsSecondaryControlPortSchottel()) {
                    controlOverride.append("MCCO,3,");
                    controlOverride.append(Utilities::lexical_cast<std::string>(model->getPortSchottel()));
                    controlOverride.append("|");
                }
                if (model->getIsSecondaryControlStbdSchottel()) {
                    controlOverride.append("MCCO,4,");
                    controlOverride.append(Utilities::lexical_cast<std::string>(model->getStbdSchottel()));
                    controlOverride.append("|");
                }
                if (model->getIsSecondaryControlPortThrustLever()) {
                    controlOverride.append("MCCO,5,");
                    controlOverride.append(Utilities::lexical_cast<std::string>(model->getPortAzimuthThrustLever()));
                    controlOverride.append("|");
                }
                if (model->getIsSecondaryControlStbdThrustLever()) {
                    controlOverride.append("MCCO,6,");
                    controlOverride.append(Utilities::lexical_cast<std::string>(model->getStbdAzimuthThrustLever()));
                    controlOverride.append("|");
                }
                if (model->getIsSecondaryControlBowThruster()) {
                    controlOverride.append("MCCO,7,");
                    controlOverride.append(Utilities::lexical_cast<std::string>(model->getBowThruster()));
                    controlOverride.append("|");
                }
                if (model->getIsSecondaryControlSternThruster()) {
                    controlOverride.append("MCCO,8,");
                    controlOverride.append(Utilities::lexical_cast<std::string>(model->getSternThruster()));
                    controlOverride.append("|");
                }
                if (controlOverride != "") {
                    //Send back to event.peer
                    ENetPacket* packet = enet_packet_create (controlOverride.c_str(), strlen (controlOverride.c_str()) + 1,0/*reliable flag*/);
                    if (packet!=0) {
                        enet_peer_send (event.peer, 0, packet);
                        enet_host_flush (server);
                    }
                }
                

            } //Check for right number of elements in received data
        } //Check received message starts with BC
        else if (receivedString.substr(0,2).compare("OS") == 0 ) { //Check if it starts with OS (Update about ownship only)
            //Strip 'OS'
            receivedString = receivedString.substr(2,receivedString.length()-2);
            //Get own ship position info from record 1, if in secondary mode (not used in multiplayer)
            if (mode==OperatingMode::Secondary) {
                std::vector<std::string> positionData = Utilities::split(receivedString,',');
                if (positionData.size() == 5) { //5 elements in position data sent
                    //std::cout << "positionData.size() == 5" << std::endl;
                    model->setPos(Utilities::lexical_cast<irr::f32>(positionData.at(0)),
                                Utilities::lexical_cast<irr::f32>(positionData.at(1)));
                    model->setHeading(Utilities::lexical_cast<irr::f32>(positionData.at(2)));
                    model->setRateOfTurn(Utilities::lexical_cast<irr::f32>(positionData.at(3)));
                    model->setSpeed(Utilities::lexical_cast<irr::f32>(positionData.at(4))/MPS_TO_KTS);
                }
            }

        }
    }

}
