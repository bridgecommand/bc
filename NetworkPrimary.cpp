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

NetworkPrimary::NetworkPrimary(SimulationModel* model) //Constructor
{

    #ifdef _WIN32
    //link to model so network can interact with model
    this->model = model; //Link to the model

    //start networking
    if (enet_initialize () != 0) {
        //fprintf(stderr, "An error occurred while initializing ENet.\n");
        exit (EXIT_FAILURE);
    }

    client = enet_host_create (NULL /* create a client host */,
    1 /* only allow 1 outgoing connection */,
    2 /* allow up 2 channels to be used, 0 and 1 */,
    57600 / 8 /* 56K modem with 56 Kbps downstream bandwidth */,
    14400 / 8 /* 56K modem with 14 Kbps upstream bandwidth */);
    if (client == NULL) {
        std::cout << "An error occurred while trying to create an ENet client host." << std::endl;
        exit (EXIT_FAILURE);
    }

    std::cout << "Started enet\n";

    //TODO: Think if this is the best way to handle failure
    #endif // _WIN32
}

NetworkPrimary::~NetworkPrimary() //Destructor
{
    #ifdef _WIN32
    //shut down networking
    enet_host_destroy(client);
    enet_deinitialize();

    std::cout << "Shut down enet\n";
    #endif // _WIN32
}

void NetworkPrimary::connectToServer(std::string hostname)
{
    #ifdef _WIN32
    /* Connect to some.server.net:18304. */
    enet_address_set_host (& address, hostname.c_str());
    address.port = 18304; //Todo: Make this configurable
    /* Initiate the connection, allocating the two channels 0 and 1. */
    peer = enet_host_connect (client, & address, 2, 0);
    if (peer == NULL)
    {
        std::cout << "No available peers for initiating an ENet connection." << std::endl;
        exit (EXIT_FAILURE);
    }
    /* Wait up to 1 second for the connection attempt to succeed. */
    if (enet_host_service (client, & event, 1000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
        std::cout << "ENet connection succeeded." << std::endl;
    }
    else
    {
        /* Either the 1 second is up or a disconnect event was */
        /* received. Reset the peer in the event the 1 second */
        /* had run out without any significant event. */
        enet_peer_reset (peer);
        std::cout << "ENet connection failed." << std::endl;
    }
    #endif // _WIN32
}

void NetworkPrimary::update()
{
    #ifdef _WIN32
    receiveNetwork();
    sendNetwork();
    #endif // _WIN32
}

void NetworkPrimary::receiveNetwork()
{
    #ifdef _WIN32

    ENetEvent event;
    if (enet_host_service (client, & event, 10) > 0) {
        if (event.type==ENET_EVENT_TYPE_RECEIVE) {

            //Convert into a string, max length 2048
            char tempString[2048]; //Fixme: Think if this is long enough
            snprintf(tempString,2048,"%s",event.packet -> data);
            std::string receivedString(tempString);

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

                            //std::cout << "Received: " << thisCommand << std::endl;

                            //Check what sort of command
                            if (thisCommand.length() > 2) {
                                if (thisCommand.substr(0,2).compare("CL") == 0) {
                                    //'CL', change leg
                                    std::vector<std::string> parts = Utilities::split(thisCommand,','); //Split into parts, 1st is command itself, 2nd and greater is the data
                                    if (parts.size() == 6) {
                                        //6 elements in 'Change leg' command: CL,shipNo,legNo,bearing,speed,distance
                                        //std::cout << "Change leg command received" << std::endl;
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
                                        //std::cout << "Add leg command received" << std::endl;
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

                                }
                            } //This command has at least three characters

                        }

                        //model->setSpeed(speed);
                        //model->setHeading(angle);
                    } //At least one command
                } //Check received message starts with MC
            } //Check message at least 3 characters

            enet_packet_destroy (event.packet);
        }
    }
    #endif // _WIN32
}

void NetworkPrimary::sendNetwork()
{
    #ifdef _WIN32
    std::string stringToSend;
    if ( model->getLoopNumber() % 100 == 0 ) { //every 100th loop, send the 'SC' message
        stringToSend = generateSendStringSC();
    } else {
        stringToSend = generateSendString();
    }

    /* Create a packet */
    ENetPacket * packet = enet_packet_create (stringToSend.c_str(),
    strlen (stringToSend.c_str()) + 1,
    /*ENET_PACKET_FLAG_RELIABLE*/0);

    /* Send the packet to the peer over channel id 0. */
    /* One could also broadcast the packet by */
    /* enet_host_broadcast (host, 0, packet); */
    enet_peer_send (peer, 0, packet);
    /* One could just use enet_host_service() instead. */
    enet_host_flush (client);
    #endif // _WIN32
}

std::string NetworkPrimary::generateSendString()
{
    /* Get data from model */
    //std::string stringToSend = Utilities::lexical_cast<std::string>(model->getHeading());
    std::string stringToSend = "BC";
    //0 Time:
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getTimestamp())); //Current timestamp
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getTimeOffset())); //Timestamp of start of first day of scenario
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getTimeDelta())); //Time from start day of scenario
    stringToSend.append("#");

    //1 Position, speed etc
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getPosX()));
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getPosZ()));
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getHeading()));
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(0)); //Fixme: Pitch
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(0)); //Fixme: Roll
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getSOG()*MPS_TO_KTS));
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getCOG()));
    stringToSend.append("#");

    //2 Numbers: Number Other, Number Controlled, Number buoys, Number MOB #
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getNumberOfOtherShips()));
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getNumberOfBuoys()));
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(0)); //Fixme: MOB
    stringToSend.append("#");

    //3 Each 'Other' (Pos X (abs), Pos Z, angle, SART |) #
    for(int number = 0; number < model->getNumberOfOtherShips(); number++ ) {
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getOtherShipPosX(number)));
        stringToSend.append(",");
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getOtherShipPosZ(number)));
        stringToSend.append(",");
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getOtherShipHeading(number)));
        stringToSend.append(",");
        stringToSend.append("0"); //Fixme: Sart enabled
        stringToSend.append(",");

        //TODO: Send leg information
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
            stringToSend.append("/");
        }

        stringToSend.append("|");
    }
    stringToSend.append("#");

    //4 Each Buoy
    for(int number = 0; number < model->getNumberOfBuoys(); number++ ) {
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getBuoyPosX(number)));
        stringToSend.append(",");
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getBuoyPosZ(number)));
        stringToSend.append("|");
    }
    stringToSend.append("#");

    //5 MOB
    stringToSend.append("0,0#"); //Fixme: Mob details

    //6 Loop
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getLoopNumber()));
    stringToSend.append("#");

    //7 Weather: Weather, Fog range, wind dirn, rain, light level #
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getWeather()));
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(0)); //Fixme: Fog range
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
    stringToSend.append("0#");

    return stringToSend;
}

std::string NetworkPrimary::generateSendStringSC()
{
    std::string stringToSend = "SC";
    stringToSend.append(model->getScenarioName());
    stringToSend.append("|");
    stringToSend.append(model->getWorldName());
    stringToSend.append("|");
    //todo: Add rest of records here
    return stringToSend;
}
