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

#include "Network.hpp"

#include "../Utilities.hpp"
#include "../Constants.hpp"
#include <iostream>
#include <cstdio>
#include <vector>

Network::Network(int port) //Constructor
{

    this->port = port;

    //start networking
    if (enet_initialize () != 0) {
        //fprintf(stderr, "An error occurred while initializing ENet.\n");
        exit (EXIT_FAILURE);
    }

    client = enet_host_create (NULL /* create a client host */,
    10 /* Allow up to 10 outgoing connections */, //Todo: Should this be configurable?
    2 /* allow up 2 channels to be used, 0 and 1 */,
    57600 / 8 /* 56K modem with 56 Kbps downstream bandwidth */, //Todo: Think about bandwidth limits
    14400 / 8 /* 56K modem with 14 Kbps upstream bandwidth */);
    if (client == NULL) {
        std::cout << "An error occurred while trying to create an ENet client host." << std::endl;
        exit (EXIT_FAILURE);
    }

    std::cout << "Started enet\n";

    //TODO: Think if this is the best way to handle failure
}

Network::~Network() //Destructor
{
    //shut down networking
    enet_host_destroy(client);
    enet_deinitialize();

    std::cout << "Shut down enet\n";
}

void Network::connectToServer(std::string hostnames)
{

    //hostname may be multiple comma separated names
    std::vector<std::string> multipleHostnames = Utilities::split(hostnames,',');

    //Ensure there's at least one entry
    if (multipleHostnames.size() < 1 ) {
        multipleHostnames.push_back(""); //Add an empty record
    }

    //Set up a peer for each hostname
    for (int i = 0; i<multipleHostnames.size(); i++) {
        ENetAddress address;
        ENetPeer* peer;

        std::string thisHostname = Utilities::trim(multipleHostnames.at(i));
        //Todo: validate this?

        /* Connect to some.server.net:18304. */
        enet_address_set_host (& address, thisHostname.c_str());
        address.port = port;
        /* Initiate the connection, allocating the two channels 0 and 1. */
        peer = enet_host_connect (client, & address, 2, 0);

        if (peer == NULL)
        {
            std::cout << "No available peers for initiating an ENet connection." << std::endl;
            exit (EXIT_FAILURE);
        }
        /* Wait up to 1 second for the connection attempt to succeed. */
        if (enet_host_service (client, & event, 1000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
            std::cout << "ENet connection succeeded to: " << thisHostname << std::endl;
            //Store peer, and initialise the vector of latest strings received
            peers.push_back(peer);
            latestMessageFromPeer.push_back("");
        } else {
            /* Either the 1 second is up or a disconnect event was */
            /* received. Reset the peer in the event the 1 second */
            /* had run out without any significant event. */
            enet_peer_reset (peer);
            std::cout << "ENet connection failed to:" << thisHostname << std::endl;
        }
    }
}

unsigned int Network::getNumberOfPeers()
{
    return peers.size();
}

void Network::sendString(std::string stringToSend, bool reliable, unsigned int peerNumber)
{
    if (peerNumber < peers.size()) {

        int reliableFlag;
        if(reliable) {
            reliableFlag = ENET_PACKET_FLAG_RELIABLE;
        } else {
            reliableFlag = 0;
        }

        if (stringToSend.length() > 0) {
            ENetPacket * packet = enet_packet_create (stringToSend.c_str(),
            strlen (stringToSend.c_str()) + 1,
            reliableFlag); //Flag

            // Send the packet to peer over channel id 0.
            enet_peer_send(peers.at(peerNumber), 0, packet);

            // One could just use enet_host_service() instead.
            enet_host_flush (client);
        }
    }
}

void Network::listenForMessages()
{
    while (enet_host_service (client, & event, 10) > 0) {
        if (event.type==ENET_EVENT_TYPE_RECEIVE) {

            //Convert into a string, max length 8192
            char tempString[8192]; //Fixme: Think if this is long enough
            snprintf(tempString,8192,"%s",event.packet -> data);
            std::string receivedString(tempString);

            //check which peer, if any it came from
            for(unsigned int i=0; i<peers.size(); i++) {
                if (event.peer==peers.at(i)) {
                    if (i<latestMessageFromPeer.size()) {
                        latestMessageFromPeer.at(i) = receivedString;
                    }
                }
            }
            enet_packet_destroy (event.packet);
        }
    }
}

std::string Network::getLatestMessage(unsigned int peerNumber)
{
    if (peerNumber < latestMessageFromPeer.size()) {
        return latestMessageFromPeer.at(peerNumber);
    } else {
        return "";
    }
}

/*

void Network::update()
{
    receiveNetwork();
    sendNetwork();
}

void Network::receiveNetwork()
{

    if (enet_host_service (client, & event, 10) > 0) {
        if (event.type==ENET_EVENT_TYPE_RECEIVE) {

            //Convert into a string, max length 2048
            char tempString[2048]; //Fixme: Think if this is long enough
            snprintf(tempString,2048,"%s",event.packet -> data);
            std::string receivedString(tempString);

            //Basic checks
            if (receivedString.length() > 3) { //Check if more than 3 chars long, ie we have at least some data
                if (receivedString.substr(0,3).compare("MPF") == 0 ) { //Check if it starts with MPF

                    //Populate the data structures from the stripped string
                    //findDataFromString(receivedString, time, ownShipData, otherShipsData, buoysData);
                    std::vector<std::string> commands = Utilities::split(receivedString,'#'); //Split into basic commands
                    //TODO: Rename from commands to something more sensible, and use (finding which peer it came from)
                    if (commands.size() > 0) {} //At least one command
                } //Check received message starts with MC


            } //Check message at least 3 characters

            enet_packet_destroy (event.packet);
        }
    }
}

void Network::sendNetwork()
{
    //Will need to be updated to send a different message to each peer
    std::string stringToSend = generateSendString();

    if (stringToSend.length() > 0) {
        ENetPacket * packet = enet_packet_create (stringToSend.c_str(),
        strlen (stringToSend.c_str()) + 1,
        0); //Flag

        // Send the packet to all connected peers over channel id 0.
        enet_host_broadcast(client, 0, packet);

        // One could just use enet_host_service() instead.
        enet_host_flush (client);
    }
}

std::string Network::generateSendString()
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
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getOtherShipSpeed(number)*MPS_TO_KTS));
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
            if (it!= (legs.end()-1)) {stringToSend.append("/");}
        }

        if (number < model->getNumberOfOtherShips()-1) {stringToSend.append("|");}
    }
    stringToSend.append("#");

    //4 Each Buoy
    for(int number = 0; number < model->getNumberOfBuoys(); number++ ) {
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getBuoyPosX(number)));
        stringToSend.append(",");
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getBuoyPosZ(number)));
        if (number < model->getNumberOfBuoys()-1) {stringToSend.append("|");}
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

std::string Network::generateSendStringScn()
{
    //TODO: Implement
    std::string stringToSend = "";//model->getSerialisedScenario();
    return stringToSend;
}

*/
