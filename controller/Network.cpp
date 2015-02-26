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
#include <iostream>

//Constructor
Network::Network(ControllerModel* model)
{
    //link to model so network can interact with model
    this->model = model; //Link to the model


    if (enet_initialize () != 0)
    {
        std::cout << "An error occurred while initializing ENet.\n";
        exit(EXIT_FAILURE);
    }

    /* Bind the server to the default localhost. */
    /* A specific host address can be specified by */
    /* enet_address_set_host (& address, "x.x.x.x"); */
    address.host = ENET_HOST_ANY;
    /* Bind the server to port 1234. */
    address.port = 1234;
    server = enet_host_create (& address /* the address to bind the server host to */,
    32 /* allow up to 32 clients and/or outgoing connections */,
    2 /* allow up to 2 channels to be used, 0 and 1 */,
    0 /* assume any amount of incoming bandwidth */,
    0 /* assume any amount of outgoing bandwidth */);
    if (server == NULL)
    {
        std::cout << "An error occurred while trying to create an ENet server host.\n";
        exit (EXIT_FAILURE);
    }

}

//Destructor
Network::~Network()
{
    enet_host_destroy(server);
    enet_deinitialize();
}

void Network::update()
{

    while (enet_host_service (server, & event, 100) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                printf ("A new client connected from %x:%u.\n",
                    event.peer -> address.host,
                    event.peer -> address.port);
                /* Store any relevant client information here. */
                //event.peer -> data = "Client information";
                break;
            case ENET_EVENT_TYPE_RECEIVE:

                //receive it
                receiveMessage();

                //send something back
                sendMessage(event.peer);

                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy (event.packet);

                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                printf ("%s disconected.\n", event.peer -> data);
                /* Reset the peer's client information. */
                event.peer -> data = NULL;

        }
    }
}

void Network::sendMessage(ENetPeer* peer)
{
    //Assumes that event contains a received message
    stringToSend = "5#170"; //5m/s
                /* Create a packet */
                packet = enet_packet_create (stringToSend.c_str(),
                strlen (stringToSend.c_str()) + 1,
                /*ENET_PACKET_FLAG_RELIABLE*/0);

                /* Send the packet to the peer over channel id 0. */
                /* One could also broadcast the packet by */
                /* enet_host_broadcast (host, 0, packet); */
                enet_peer_send (peer, 0, packet);
                /* One could just use enet_host_service() instead. */
                enet_host_flush (server);
}

void Network::receiveMessage()
{
    //Assumes that event contains a received message
    printf ("A packet of length %u containing %s was received from %s on channel %u.\n",
                    event.packet -> dataLength,
                    event.packet -> data,
                    event.peer -> data,
                    event.channelID);
}
