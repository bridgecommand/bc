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

        enet_address_set_host (& address, thisHostname.c_str());

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
