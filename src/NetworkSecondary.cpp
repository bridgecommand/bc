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
        2 /* allow up to 2 channels to be used, 0 and 1 */,
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
     if (enet_host_service (server, & event, 1000) > 0) { //Wait 1s for event
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
            if (receivedData.size() == 11) { //11 basic records in data sent

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
                //Numbers of objects in record 2 (Others, buoys, MOBs)
                std::vector<std::string> numberData = Utilities::split(receivedData.at(2),',');
                if (numberData.size() == 3) {
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

                } //Check if 3 number elements for Other ships, buoys and MOBs

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

                    //Send back to event.peer
                    ENetPacket* packet = enet_packet_create (multiplayerFeedback.c_str(), strlen (multiplayerFeedback.c_str()) + 1,0/*reliable flag*/);
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
