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
#include "ControllerModel.hpp"
#include "../Utilities.hpp"

#include <iostream>
#include <vector>

//Constructor
Network::Network(int port)
{

    server = 0;

    if (enet_initialize () != 0)
    {
        std::cout << "An error occurred while initializing ENet.\n";
        exit(EXIT_FAILURE);
    }

    /* Bind the server to the default localhost. */
    /* A specific host address can be specified by */
    /* enet_address_set_host (& address, "x.x.x.x"); */
    address.host = ENET_HOST_ANY;

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
        exit (EXIT_FAILURE);
    } else {
        std::cout << "Connected on UDP port " << server->address.port << std::endl;
    }

}

//Destructor
Network::~Network()
{
    enet_host_destroy(server);
    enet_deinitialize();
}

void Network::update(irr::f32& time, ShipData& ownShipData)
{
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

                //receive it
                receiveMessage(time,ownShipData);

                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy (event.packet);

                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                printf ("%s disconected.\n", (char*)(event.peer -> data));
                /* Reset the peer's client information. */
                event.peer -> data = NULL;
            default:
                break;

        }
    }

    //std::cout << "Heading: " << ownShipData.heading << std::endl;
}

int Network::getPort()
{
    if (server) {
        return server->address.port;
    }
    return 0;

}

void Network::receiveMessage(irr::f32& time, ShipData& ownShipData)
{
    //Assumes that event contains a received message
    /*printf ("A packet of length %u was received from %s on channel %u.\n",
                    event.packet -> dataLength,
                    event.peer -> data,
                    event.channelID);*/

    //Convert into a string, max length 2048
    char tempString[2048]; //Fixme: Think if this is long enough
    snprintf(tempString,2048,"%s",event.packet -> data);
    std::string receivedString(tempString);

    //Basic checks
    if (receivedString.length() > 2) { //Check if more than 2 chars long, ie we have at least some data
        if (receivedString.substr(0,2).compare("BC") == 0 ) { //Check if it starts with BC
            //Strip 'BC'
            receivedString = receivedString.substr(2,receivedString.length()-2);

            //Populate the data structures from the stripped string
            findDataFromString(receivedString, time, ownShipData);

        } //Check received message starts with BC
    } //Check message at least 3 characters

}

void Network::findDataFromString(const std::string& receivedString, irr::f32& time, ShipData& ownShipData) {
//Split into main parts
    std::vector<std::string> receivedData = Utilities::split(receivedString,'#');

    //Check number of elements
    if (receivedData.size() == 11) { //11 basic records in data sent

        //Time info is record 0
        std::vector<std::string> timeData = Utilities::split(receivedData.at(0),',');
        //Time since start of scenario day 1 is record 2
        if (timeData.size() > 0) {
            time = Utilities::lexical_cast<irr::f32>(timeData.at(2)); //
        }

        //Position info is record 1
        std::vector<std::string> positionData = Utilities::split(receivedData.at(1),',');
        findOwnShipPositionData(positionData, ownShipData); //Populate ownShipData from the positionData


    } //Check correct number of records received
}

void Network::findOwnShipPositionData(const std::vector<std::string>& positionData, ShipData& ownShipData)
{
    if (positionData.size() == 9) { //8 elements in position data sent
        ownShipData.X = Utilities::lexical_cast<irr::f32>(positionData.at(0));
        ownShipData.Z = Utilities::lexical_cast<irr::f32>(positionData.at(1));
        ownShipData.heading = Utilities::lexical_cast<irr::f32>(positionData.at(2));

        //In format rudder:wheel angle, so split to get wheel component, and just use cast to get rudder, discarding wheel part
        ownShipData.rudder = Utilities::lexical_cast<irr::f32>(positionData.at(8));
        std::vector<std::string> rudderWheelData = Utilities::split(positionData.at(8),':');
        if (rudderWheelData.size() == 2) {
            ownShipData.wheel =  Utilities::lexical_cast<irr::f32>(rudderWheelData.at(1));
        }

    }
}
