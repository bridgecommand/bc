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

#include "SimulationModel.hpp"
#include "Utilities.hpp"
#include "Constants.hpp"
#include <iostream>
#include <cstdio>

Network::Network(SimulationModel* model) //Constructor
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
        //fprintf (stderr, "An error occurred while trying to create an ENet client host.\n");
        exit (EXIT_FAILURE);
    }

    std::cout << "Started enet\n";

    //TODO: Think if this is the best way to handle failure
    #endif // _WIN32
}

Network::~Network() //Destructor
{
    #ifdef _WIN32
    //shut down networking
    enet_host_destroy(client);
    enet_deinitialize();

    std::cout << "Shut down enet\n";
    #endif // _WIN32
}

void Network::connectToServer()
{
    #ifdef _WIN32
    /* Connect to some.server.net:1234. */
    enet_address_set_host (& address, "localhost");
    address.port = 1234;
    /* Initiate the connection, allocating the two channels 0 and 1. */
    peer = enet_host_connect (client, & address, 2, 0);
    if (peer == NULL)
    {
        fprintf (stderr,
        "No available peers for initiating an ENet connection.\n");
        exit (EXIT_FAILURE);
    }
    /* Wait up to 1 second for the connection attempt to succeed. */
    if (enet_host_service (client, & event, 1000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
        puts ("Connection to localhost:1234 succeeded.");
    }
    else
    {
        /* Either the 1 second is up or a disconnect event was */
        /* received. Reset the peer in the event the 1 second */
        /* had run out without any significant event. */
        enet_peer_reset (peer);
        puts ("Connection to localhost:1234 failed.");
    }
    #endif // _WIN32
}

void Network::update()
{
    #ifdef _WIN32
    receiveNetwork();
    sendNetwork();
    #endif // _WIN32
}

void Network::receiveNetwork()
{
    #ifdef _WIN32
    ENetEvent event;
    if (enet_host_service (client, & event, 10) > 0) {
        if (event.type==ENET_EVENT_TYPE_RECEIVE) {

            //Currently converting data to and from string. Should directly send data of the type required.
            std::string readSpeedString ((char*)event.packet -> data);

            /*Set this speed in the model */
            float readSpeed = Utilities::lexical_cast<float>(readSpeedString);
            model->setSpeed(readSpeed);

            enet_packet_destroy (event.packet);
        }
    }
    #endif // _WIN32
}

void Network::sendNetwork()
{
    #ifdef _WIN32
    /* Get data from model */
    //std::string stringToSend = Utilities::lexical_cast<std::string>(model->getHeading());
    std::string stringToSend = "BC";
    //Time:
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getTimestamp()));
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getTimeOffset()));
    stringToSend.append(",");
    stringToSend.append(Utilities::lexical_cast<std::string>(model->getTimeDelta()));
    stringToSend.append("#");

    //Position, speed etc
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

    //Numbers


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
