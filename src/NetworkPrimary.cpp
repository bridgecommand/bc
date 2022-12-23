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

#include "NetworkPrimary.hpp"

#include "SimulationModel.hpp"
#include "Utilities.hpp"
#include "Constants.hpp"
#include "Leg.hpp"
#include <iostream>
#include <cstdio>
#include <vector>

NetworkPrimary::NetworkPrimary(int port, irr::IrrlichtDevice* dev) //Constructor
{

    model=0; //Not linked at the moment
    this->port = port;
    device = dev;

    //start networking
    if (enet_initialize () != 0) {
        //fprintf(stderr, "An error occurred while initializing ENet.\n");
		std::cerr << "An error occurred while initialising ENet" << std::endl;
		exit(EXIT_FAILURE);
    }

    client = enet_host_create (NULL /* create a client host */,
    10 /* Allow up to 10 outgoing connections */, //Todo: Should this be configurable?
    2 /* allow up 2 channels to be used, 0 and 1 */,
    57600 / 8 /* 56K modem with 56 Kbps downstream bandwidth */, //Todo: Think about bandwidth limits
    14400 / 8 /* 56K modem with 14 Kbps upstream bandwidth */);
    if (client == NULL) {
        std::cerr << "An error occurred while trying to create an ENet client host." << std::endl;
		enet_deinitialize();
		exit(EXIT_FAILURE); //TODO: Think if this is the best way to handle failure
    }

    device->getLogger()->log("Started enet.");

}

NetworkPrimary::~NetworkPrimary() //Destructor
{
    //shut down networking
    enet_host_destroy(client);
    enet_deinitialize();
}

void NetworkPrimary::connectToServer(std::string hostnames)
{
    //hostname may be multiple comma separated names
    hostnames = Utilities::trim(hostnames);
    std::vector<std::string> multipleHostnames = Utilities::split(hostnames,',');

    //Ensure there's at least one entry, or don't use networking
    if (multipleHostnames.size() < 1 ) {
        networkRequested = false;
    } else {
        networkRequested = true;
    }

    if (networkRequested) { //Only bother with networking if the user actually wants to!
        //Trim each hostname (for spaces etc), and make lower case
        for (unsigned int i = 0; i<multipleHostnames.size(); i++) {
            multipleHostnames.at(i) = Utilities::trim(multipleHostnames.at(i));
            Utilities::to_lower(multipleHostnames.at(i));
        }

        //Set up a peer for each hostname
        for (unsigned int i = 0; i<multipleHostnames.size(); i++) {
            ENetAddress address;
            ENetPeer* peer;

            std::string thisHostname = multipleHostnames.at(i);
            //Todo: validate this?

            //Check if the string contains a ':', and if so, split into hostname and port part
            if (thisHostname.find(':') != std::string::npos) {
                std::vector<std::string> splitHostname = Utilities::split(thisHostname,':');
                if (splitHostname.size()==2) {
                    thisHostname = splitHostname.at(0);
                    address.port = Utilities::lexical_cast<enet_uint16>(splitHostname.at(1));
                } else {
                    address.port = port; //Fall back to default
                }
            } else {
                address.port = port;

                //Count number of instances of this earlier in list, and if so, increment port, so
                //localhost,localhost,localhost would become like localhost:port,localhost:port+1,localhost:port+2
                for (unsigned int j=0; j<i; j++) {
                    if (thisHostname.compare(multipleHostnames.at(j))==0) {
                        address.port++;
                    }
                }
            }

            /* Connect to some.server.net:18304. */
            enet_address_set_host (& address, thisHostname.c_str());

            /* Initiate the connection, allocating the two channels 0 and 1. */
            peer = enet_host_connect (client, & address, 2, 0);
            //Note we don't store peer pointer, as we broadcast to all connected peers.
            if (peer == NULL)
            {
                std::cerr << "No available peers for initiating an ENet connection." << std::endl;
                enet_deinitialize();
                exit(EXIT_FAILURE);
            }
            /* Wait up to 1 second for the connection attempt to succeed. */
            if (enet_host_service (client, & event, 1000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
                //std::string logMessage = "ENet connection succeeded to: ";
                //logMessage.append(thisHostname);
                device->getLogger()->log("ENet connection succeeded to:");
                device->getLogger()->log(thisHostname.c_str());
            } else {
                /* Either the 1 second is up or a disconnect event was */
                /* received. Reset the peer in the event the 1 second */
                /* had run out without any significant event. */
                enet_peer_reset (peer);
                device->getLogger()->log("ENet connection failed to:");
                device->getLogger()->log(thisHostname.c_str());
            }
        }
    }
}

void NetworkPrimary::getScenarioFromNetwork(std::string& dataString)
{
    //Not used by primary
}

void NetworkPrimary::setModel(SimulationModel* model) //This MUST be called before update()
{
    this->model = model;
}

void NetworkPrimary::update()
{
    if (!networkRequested) {
        return;
    }
    receiveNetwork();
    sendNetwork();
}

int NetworkPrimary::getPort()
{
    return 0;
}

void NetworkPrimary::receiveNetwork()
{
    if (!networkRequested) {
        return;
    }

    if (model==0) {
        std::cerr << "Network not linked to model" << std::endl;
        return;
    }
    if (enet_host_service (client, & event, 10) > 0) {
        if (event.type==ENET_EVENT_TYPE_RECEIVE) {

            //Convert into a string, max length 8192
            char tempString[8192]; //Fixme: Think if this is long enough
            snprintf(tempString,8192,"%s",event.packet -> data);
            std::string receivedStrings(tempString);

            std::vector<std::string> receivedData  = Utilities::split(receivedStrings,'|');

            for(int subMessage = 0; subMessage < receivedData.size(); subMessage++) {
                std::string receivedString = receivedData.at(subMessage);

                //Basic checks
                if (receivedString.length() > 2) { //Check if more than 2 chars long, ie we have at least some data
                    if (receivedString.substr(0,2).compare("MC") == 0 ) { //Check if it starts with MC
                        //Strip 'MC'
                        receivedString = receivedString.substr(2,receivedString.length()-2);

                        //Populate the data structures from the stripped string
                        //findDataFromString(receivedString, time, ownShipData, otherShipsData, buoysData);
                        std::vector<std::string> commands = Utilities::split(receivedString,'#'); //Split into basic commands
                        if (commands.size() > 0) {

                            //Iterate through commands
                            for(std::vector<std::string>::iterator it = commands.begin(); it != commands.end(); ++it) {

                                std::string thisCommand = *it;

                                //Check what sort of command
                                if (thisCommand.length() > 2) {
                                    if (thisCommand.substr(0,2).compare("CL") == 0) {
                                        //'CL', change leg
                                        std::vector<std::string> parts = Utilities::split(thisCommand,','); //Split into parts, 1st is command itself, 2nd and greater is the data
                                        if (parts.size() == 6) {
                                            //6 elements in 'Change leg' command: CL,shipNo,legNo,bearing,speed,distance
                                            int shipNo =        Utilities::lexical_cast<int>(parts.at(1)) - 1; //Numbering on network starts at 1, internal numbering at 0
                                            int legNo =         Utilities::lexical_cast<int>(parts.at(2)) - 1; //Numbering on network starts at 1, internal numbering at 0
                                            irr::f32 bearing =  Utilities::lexical_cast<irr::f32>(parts.at(3));
                                            irr::f32 speed =    Utilities::lexical_cast<irr::f32>(parts.at(4));
                                            irr::f32 distance = Utilities::lexical_cast<irr::f32>(parts.at(5));
                                            model->changeOtherShipLeg(shipNo,legNo,bearing,speed,distance);
                                        } //If six data parts received
                                    } else if (thisCommand.substr(0,2).compare("AL") == 0) {
                                        //'AL' add leg
                                        std::vector<std::string> parts = Utilities::split(thisCommand,','); //Split into parts, 1st is command itself, 2nd and greater is the data
                                        if (parts.size() == 6) {
                                            //6 elements in 'Add leg' command: CL,shipNo,afterLegNo,bearing,speed,distance
                                            int shipNo =        Utilities::lexical_cast<int>(parts.at(1)) - 1; //Numbering on network starts at 1, internal numbering at 0
                                            int legNo =         Utilities::lexical_cast<int>(parts.at(2)) - 1; //Numbering on network starts at 1, internal numbering at 0
                                            irr::f32 bearing =  Utilities::lexical_cast<irr::f32>(parts.at(3));
                                            irr::f32 speed =    Utilities::lexical_cast<irr::f32>(parts.at(4));
                                            irr::f32 distance = Utilities::lexical_cast<irr::f32>(parts.at(5));
                                            model->addOtherShipLeg(shipNo,legNo,bearing,speed,distance);
                                        } //If six data parts received
                                    } else if (thisCommand.substr(0,2).compare("DL") == 0) {
                                        //'DL' delete leg
                                        std::vector<std::string> parts = Utilities::split(thisCommand,','); //Split into parts, 1st is command itself, 2nd and greater is the data
                                        if (parts.size() == 3) {
                                            //3 elements in 'Delete Leg' command: DL,shipNo,legNo
                                            int shipNo =        Utilities::lexical_cast<int>(parts.at(1)) - 1; //Numbering on network starts at 1, internal numbering at 0
                                            int legNo =         Utilities::lexical_cast<int>(parts.at(2)) - 1; //Numbering on network starts at 1, internal numbering at 0
                                            model->deleteOtherShipLeg(shipNo,legNo);
                                        }
                                    } else if (thisCommand.substr(0,2).compare("RS") == 0) {
                                        //'RS' reposition ship
                                        std::vector<std::string> parts = Utilities::split(thisCommand,','); //Split into parts, 1st is command itself, 2nd and greater is the data
                                        if (parts.size() == 4) {
                                            //4 elements in 'Reposition ship' command: RS,shipNo,posX,posZ
                                            int shipNo =        Utilities::lexical_cast<int>(parts.at(1)) - 1; //Numbering on network starts at 1, internal numbering at 0
                                            irr::f32 positionX = Utilities::lexical_cast<irr::f32>(parts.at(2));
                                            irr::f32 positionZ = Utilities::lexical_cast<irr::f32>(parts.at(3));
                                            if (shipNo<0){
                                                model->setPos(positionX,positionZ);
                                            } else {
                                                model->setOtherShipPos(shipNo,positionX,positionZ);
                                            }
                                        }
                                    } else if (thisCommand.substr(0,2).compare("RL") == 0) {
                                        //'RL' reset legs and position, used for AIS contacts
                                        std::vector<std::string> parts = Utilities::split(thisCommand,','); //Split into parts, 1st is command itself, 2nd and greater is the data
                                        if (parts.size() == 6) {
                                            //6 elements in 'Reset legs' command: RS,shipNo,posX,posZ
                                            int shipNo =         Utilities::lexical_cast<int>(parts.at(1)) - 1; //Numbering on network starts at 1, internal numbering at 0
                                            irr::f32 positionX = Utilities::lexical_cast<irr::f32>(parts.at(2));
                                            irr::f32 positionZ = Utilities::lexical_cast<irr::f32>(parts.at(3));
                                            irr::f32 cog =       Utilities::lexical_cast<irr::f32>(parts.at(4));
                                            irr::f32 sog =       Utilities::lexical_cast<irr::f32>(parts.at(5));
                                            if (shipNo>=0){
                                                model->setOtherShipPos(shipNo,positionX,positionZ);
                                                model->resetOtherShipLegs(shipNo,cog,sog,1); //Hard coded 1Nm distance
                                            }
                                        }

                                    } else if (thisCommand.substr(0,2).compare("SW") == 0) {
                                        //'SW' Set weather
                                        std::vector<std::string> parts = Utilities::split(thisCommand,','); //Split into parts, 1st is command itself, 2nd and greater is the data
                                        if (parts.size() == 4) {
                                            //4 elements in 'Set weather' command: SW,weather,rain,vis
                                            irr::f32 weather    = Utilities::lexical_cast<irr::f32>(parts.at(1));
                                            irr::f32 rain       = Utilities::lexical_cast<irr::f32>(parts.at(2));
                                            irr::f32 visibility = Utilities::lexical_cast<irr::f32>(parts.at(3));
                                            if (weather >= 0) {model->setWeather(weather);}
                                            if (rain >=0) {model->setRain(rain);}
                                            if (visibility>0) {model->setVisibility(visibility);}
                                        }


                                    } else if (thisCommand.substr(0,2).compare("MO") == 0) {
                                        //'MO', Man overboard
                                        std::vector<std::string> parts = Utilities::split(thisCommand,','); //Split into parts, 1st is command itself, 2nd and greater is the data
                                        if (parts.size()==2) {
                                            irr::s32 mobMode = Utilities::lexical_cast<irr::s32>(parts.at(1));
                                            if (mobMode==1) {
                                                model->releaseManOverboard();
                                            } else if (mobMode==-1) {
                                                model->retrieveManOverboard();
                                            }
                                        }
                                    } else if (thisCommand.substr(0,2).compare("MM") == 0) {
                                        //'MM', Set MMSI
                                        std::vector<std::string> parts = Utilities::split(thisCommand,','); //Split into parts, 1st is command itself, 2nd and greater is the data
                                        if (parts.size()==3) {
                                            int shipNo =        Utilities::lexical_cast<int>(parts.at(1)) - 1; //Numbering on network starts at 1, internal numbering at 0
                                            irr::u32 mmsi = Utilities::lexical_cast<irr::u32>(parts.at(2));
                                            model->setOtherShipMMSI(shipNo,mmsi);
                                        }
                                    } else if (thisCommand.substr(0,2).compare("RW") == 0) {
                                        //'RW', How rudder actuation is working (which pump, is it working?)
                                        std::vector<std::string> parts = Utilities::split(thisCommand,','); //Split into parts, 1st is command itself, 2nd and greater is the data
                                        if (parts.size()==3) {
                                            irr::s32 whichPump = Utilities::lexical_cast<irr::s32>(parts.at(1));
                                            irr::s32 rudderFunction = Utilities::lexical_cast<irr::s32>(parts.at(2));
                                            if (whichPump==1) {
                                                if (rudderFunction==0) {
                                                    model->setRudderPumpState(1,false);
                                                    model->setAlarm(true);
                                                } else {
                                                    model->setRudderPumpState(1,true);
                                                    if (model->getRudderPumpState(2)) {
                                                        model->setAlarm(false); //Only turn off alarm if other pump is working
                                                    }
                                                }
                                            } else if (whichPump==2) {
                                                if (rudderFunction==0) {
                                                    model->setRudderPumpState(2,false);
                                                    model->setAlarm(true);
                                                } else {
                                                    model->setRudderPumpState(2,true);
                                                    if (model->getRudderPumpState(1)) {
                                                        model->setAlarm(false); //Only turn off alarm if other pump is working
                                                    }
                                                }
                                            } 
                                        }
                                    } else if (thisCommand.substr(0,2).compare("RF") == 0) {
                                        //'RF', How rudder follow up is working (0, or 1)
                                        std::vector<std::string> parts = Utilities::split(thisCommand,','); //Split into parts, 1st is command itself, 2nd and greater is the data
                                        if (parts.size()==2) {
                                            irr::s32 rudderFunction = Utilities::lexical_cast<irr::s32>(parts.at(1));
                                            if (rudderFunction==1) {
                                                //Follow up rudder working
                                                model->setFollowUpRudderWorking(true);
                                            } else if (rudderFunction==0) {
                                                //Follow up rudder not working
                                                model->setFollowUpRudderWorking(false);
                                            }
                                        }
                                    }



                                } //This command has at least three characters

                            }

                            //model->setSpeed(speed);
                            //model->setHeading(angle);
                        } //At least one command
                    } //Check received message starts with MC

                } //Check message at least 3 characters
            }

            enet_packet_destroy (event.packet);
        }
    }
}

void NetworkPrimary::sendNetwork()
{
    if (!networkRequested) {
        return;
    }

    std::string stringToSend;
    if ( model->getLoopNumber() % 100 == 0 ) { //every 100th loop, send the 'SCN' message with all scenario details
        stringToSend = generateSendStringScn();
    } else if ( model->getLoopNumber() % 10 == 0 ){ //every 10th loop, send the main BC message
        stringToSend = generateSendString();
    } else {
        stringToSend = generateSendStringShort();
    }

    if (stringToSend.length() > 0) {
        /* Create a packet */
        ENetPacket * packet = enet_packet_create (stringToSend.c_str(),
        strlen (stringToSend.c_str()) + 1,
        /*ENET_PACKET_FLAG_RELIABLE*/0);

        /* Send the packet to all connected peers over channel id 0. */
        enet_host_broadcast(client, 0, packet);

        /* One could just use enet_host_service() instead. */
        enet_host_flush (client);
    }
}

std::string NetworkPrimary::generateSendStringShort()
{
    // Get data from model

    std::string stringToSend = "OS"; //Own ship only

    //1 Position, speed etc
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getPosX()));
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getPosZ()));
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getHeading()));
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getRateOfTurn()));
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getSOG()*MPS_TO_KTS));

    return stringToSend;
}

std::string NetworkPrimary::generateSendString()
{
    // Get data from model
    //Note that in each 'for' loop, we only add the terminator if it isn't the last in the list

    std::string stringToSend = "BC";
    //0 Time:
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getTimestamp())); //Current timestamp
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getTimeOffset())); //Timestamp of start of first day of scenario
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getTimeDelta())); //Time from start day of scenario
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getAccelerator())); //Current accelerator
    stringToSend.append("#");

    //1 Position, speed etc
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getPosX()));
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getPosZ()));
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getHeading()));
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getRateOfTurn()));
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(0)); //Fixme: Pitch
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(0)); //Fixme: Roll
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getSOG()*MPS_TO_KTS));
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getCOG()));
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getRudder()));
    stringToSend.append(":");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getWheel()));
    stringToSend.append(":");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getPortEngineRPM()));
    stringToSend.append(":");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getStbdEngineRPM()));
    stringToSend.append("#");

    //2 Numbers: Number Other, Number buoys, Number MOB #
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getNumberOfOtherShips()));
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getNumberOfBuoys()));
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getManOverboardVisible()? 1 : 0));
    stringToSend.append("#");

    //3 Each 'Other' (Pos X (abs), Pos Z, angle, rate of turn, SART, MMSI |) #
    for(int number = 0; number < (int)model->getNumberOfOtherShips(); number++ ) {
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getOtherShipPosX(number)));
        stringToSend.append(",");
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getOtherShipPosZ(number)));
        stringToSend.append(",");
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getOtherShipHeading(number)));
        stringToSend.append(",");
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getOtherShipSpeed(number)*MPS_TO_KTS));
        stringToSend.append(",");
        stringToSend.append("0"); // Rate of turn: This is not currently used in normal mode
        stringToSend.append(",");
        stringToSend.append("0"); //Fixme: Sart enabled
        stringToSend.append(",");
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getOtherShipMMSI(number)));
        stringToSend.append(",");

        //std::cout << "MMSI for other ship " << number << ":" << model->getOtherShipMMSI(number) << std::endl;

        //Send leg information
        std::vector<Leg> legs = model->getOtherShipLegs(number);
        stringToSend.append(Utilities::lexical_cast<std::string>(legs.size())); //Number of legs
        stringToSend.append(",");
        //Build leg information, each leg separated by a '/', each value by ':'
        for(std::vector<Leg>::iterator it = legs.begin(); it != legs.end(); ++it) {
            stringToSend.append(Utilities::lexical_cast<std::string>(it->bearing));
            stringToSend.append(":");
            stringToSend.append(Utilities::lexical_cast<std::string>(it->speed));
            stringToSend.append(":");
            stringToSend.append(Utilities::lexical_cast<std::string>(it->startTime));
            if (it!= (legs.end()-1)) {stringToSend.append("/");}
        }

        if (number < (int)model->getNumberOfOtherShips()-1) {stringToSend.append("|");}
    }
    stringToSend.append("#");

    //4 Each Buoy
    for(int number = 0; number < (int)model->getNumberOfBuoys(); number++ ) {
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getBuoyPosX(number)));
        stringToSend.append(",");
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getBuoyPosZ(number)));
        if (number < (int)model->getNumberOfBuoys()-1) {stringToSend.append("|");}
    }
    stringToSend.append("#");

    //5 MOB
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getManOverboardPosX()));
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getManOverboardPosZ()));
    stringToSend.append("#");

    //6 Loop
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getLoopNumber()));
    stringToSend.append("#");

    //7 Weather: Weather, Fog range, wind dirn, rain, light level #
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getWeather()));
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getVisibility()));
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(0)); //Fixme: Wind dirn
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getRain()));
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(0)); //Fixme: Light level
    stringToSend.append("#");

    //8 EBL Brg, height, show (or 0,0,0) #
    stringToSend.append("0,0,0#"); //Fixme: Mob details

    //9 View number
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getCameraView()));
    stringToSend.append("#");

    //10 Multiplayer request here (Not used)
    stringToSend.append("0");

    return stringToSend;
}

std::string NetworkPrimary::generateSendStringScn()
{
    std::string stringToSend = model->getSerialisedScenario();
    return stringToSend;
}
