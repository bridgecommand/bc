#include "Network.hpp"

#include "SimulationModel.hpp"
#include <iostream>
#include <cstdio>
#include "lexical_cast.hpp"

Network::Network(SimulationModel* mdl) //Constructor
{
    //link to model so network can interact with model
    model = mdl; //Link to the model

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
}

Network::~Network() //Destructor
{
    //shut down networking
    enet_host_destroy(client);
    enet_deinitialize();

    std::cout << "Shut down enet\n";
}

void Network::connectToServer()
{
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
    /* Wait up to 5 seconds for the connection attempt to succeed. */
    if (enet_host_service (client, & event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
        puts ("Connection to localhost:1234 succeeded.");
    }
    else
    {
        /* Either the 5 seconds are up or a disconnect event was */
        /* received. Reset the peer in the event the 5 seconds */
        /* had run out without any significant event. */
        enet_peer_reset (peer);
        puts ("Connection to localhost:1234 failed.");
    }
}

void Network::update()
{
    receiveNetwork();
    sendNetwork();
}

void Network::receiveNetwork()
{
    ENetEvent event;
    if (enet_host_service (client, & event, 10) > 0) {
        if (event.type==ENET_EVENT_TYPE_RECEIVE) {

            //Currently converting data to and from string. Should directly send data of the type required.
            std::string readSpeedString ((char*)event.packet -> data);

            /*Set this speed in the model */
            float readSpeed = lexical_cast<float>(readSpeedString);
            model->setSpeed(readSpeed);

            enet_packet_destroy (event.packet);
        }
    }
}

void Network::sendNetwork()
{

    /* Get data from model */
    std::string stringToSend = lexical_cast<std::string>(model->getHeading());

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
}
