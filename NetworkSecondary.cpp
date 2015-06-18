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


NetworkSecondary::NetworkSecondary(SimulationModel* model)
{
    #ifdef _WIN32
    this->model = model;

    if (enet_initialize () != 0)
    {
        std::cout << "An error occurred while initializing ENet.\n";
        exit(EXIT_FAILURE);
    }

    /* Bind the server to the default localhost. */
    /* A specific host address can be specified by */
    /* enet_address_set_host (& address, "x.x.x.x"); */
    address.host = ENET_HOST_ANY;
    /* Bind the server to port 18304. */
    address.port = 18304;
    server = enet_host_create (& address /* the address to bind the server host to */,
    32 /* allow up to 32 clients and/or outgoing connections */,
    2 /* allow up to 2 channels to be used, 0 and 1 */,
    0 /* assume any amount of incoming bandwidth */,
    0 /* assume any amount of outgoing bandwidth */);
    if (server == NULL)
    {
        std::cout << "An error occurred while trying to create an ENet server host." << std::endl;
        exit (EXIT_FAILURE);
    }
    #endif
}

NetworkSecondary::~NetworkSecondary()
{
    #ifdef _WIN32
    enet_host_destroy(server);
    enet_deinitialize();
    std::cout << "Shut down enet\n";
    #endif
}

void NetworkSecondary::connectToServer(std::string hostname)
{
    #ifdef _WIN32
    #endif // _WIN32

    //Don't need to do anything
}

void NetworkSecondary::update()
{
    #ifdef _WIN32

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
                printf ("%s disconected.\n", event.peer -> data);
                /* Reset the peer's client information. */
                event.peer -> data = NULL;
        }
    }

    #endif // _WIN32
}

void NetworkSecondary::receiveMessage()
{
    //receive it
    char tempString[2048]; //Fixme: Think if this is long enough
    snprintf(tempString,2048,"%s",event.packet -> data);
    std::string receivedString(tempString);

    //Basic checks
    if (receivedString.length() > 2) { //Check if more than 2 chars long, ie we have at least some data
        if (receivedString.substr(0,2).compare("BC") == 0 ) { //Check if it starts with BC
            //Strip 'BC'
            receivedString = receivedString.substr(2,receivedString.length()-2);

            //Split into main parts
            std::vector<std::string> receivedData = Utilities::split(receivedString,'#');

            //Check number of elements
            if (receivedData.size() == 11) { //11 basic records in data sent

                //Position info is record 1
                std::vector<std::string> positionData = Utilities::split(receivedData.at(1),',');

                if (positionData.size() == 7) { //7 elements in position data sent
                    //ownShipData.X = Utilities::lexical_cast<irr::f32>(positionData.at(0));
                    //ownShipData.Z = Utilities::lexical_cast<irr::f32>(positionData.at(1));
                    model->setHeading(Utilities::lexical_cast<float>(positionData.at(2)));
                }

            } //Check for right number of elements in received data
        } //Check received message starts with BC
    } //Check message at least 3 characters
}
