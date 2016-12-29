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

    if (enet_initialize () != 0)
    {
        std::cout << "An error occurred while initializing ENet.\n";
        exit(EXIT_FAILURE);
    }

    /* Bind the server to the default localhost. */
    /* A specific host address can be specified by */
    /* enet_address_set_host (& address, "x.x.x.x"); */
    address.host = ENET_HOST_ANY;
    /* Bind the server to port XXXX. */
    address.port = port;
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

    stringToSend = "";

}

//Destructor
Network::~Network()
{
    enet_host_destroy(server);
    enet_deinitialize();
}

std::string Network::findWorldName()
{
    //Return world name if it's just been sent, or an empty string
    //This function should normally be called from a loop

    std::string worldName = "";


    if (enet_host_service (server, & event, 10) > 0) {
        if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            //receive it
            char tempString[2048]; //Fixme: Think if this is long enough
            snprintf(tempString,2048,"%s",event.packet -> data);
            std::string receivedString(tempString);

            //Basic checks
            if (receivedString.length() > 4) { //Check if more than 2 chars long, ie we have at least some data
                if (receivedString.substr(0,4).compare("SCN1") == 0 ) { //Check if it starts with SC

                    //Find world model from this
                    std::vector<std::string> receivedData = Utilities::split(receivedString,'#');
                    if (receivedData.size() > 2) {
                        worldName = receivedData.at(2);
                    }
                }
            }

            /* Clean up the packet now that we're done using it. */
            enet_packet_destroy (event.packet);
        } //received message
    } //enet event

    return worldName;
}

void Network::update(irr::f32& time, ShipData& ownShipData, std::vector<OtherShipDisplayData>& otherShipsData, std::vector<PositionData>& buoysData, irr::f32& weather, irr::f32& visibility, irr::f32& rain)
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
                receiveMessage(time,ownShipData,otherShipsData,buoysData,weather,visibility,rain);

                //send something back
                sendMessage(event.peer); //Todo: Think if we only want to send after receipt?

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

void Network::setStringToSend(std::string stringToSend)
{
    this->stringToSend = stringToSend;
}

void Network::sendMessage(ENetPeer* peer)
{
    //Assumes that event contains a received message

    if (stringToSend.length() > 0) {
        /* Create a packet */
        packet = enet_packet_create (stringToSend.c_str(),
        strlen (stringToSend.c_str()) + 1,
        /*ENET_PACKET_FLAG_RELIABLE*/1);

        /* Send the packet to the peer over channel id 0. */
        /* One could also broadcast the packet by */
        /* enet_host_broadcast (host, 0, packet); */
        enet_peer_send (peer, 0, packet);
        /* One could just use enet_host_service() instead. */
        enet_host_flush (server);

        stringToSend = ""; //Sent message, so clear it
    }
}

void Network::receiveMessage(irr::f32& time, ShipData& ownShipData, std::vector<OtherShipDisplayData>& otherShipsData, std::vector<PositionData>& buoysData, irr::f32& weather, irr::f32& visibility, irr::f32& rain)
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
            findDataFromString(receivedString, time, ownShipData, otherShipsData, buoysData, weather, visibility, rain);

        } //Check received message starts with BC
    } //Check message at least 3 characters

}

void Network::findDataFromString(const std::string& receivedString, irr::f32& time, ShipData& ownShipData, std::vector<OtherShipDisplayData>& otherShipsData, std::vector<PositionData>& buoysData, irr::f32& weather, irr::f32& visibility, irr::f32& rain) {
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

        //Numbers of objects in record 2 (Others, buoys, MOBs)
        std::vector<std::string> numberData = Utilities::split(receivedData.at(2),',');
        if (numberData.size() == 3) {
            irr::u32 numberOthers = Utilities::lexical_cast<irr::u32>(numberData.at(0));
            irr::u32 numberBuoys  = Utilities::lexical_cast<irr::u32>(numberData.at(1));

            //Update other ship data
            std::vector<std::string> otherShipsDataString = Utilities::split(receivedData.at(3),'|');
            if (numberOthers == otherShipsDataString.size()) {
                findOtherShipData(otherShipsDataString, otherShipsData); //Populate otherShipsData from otherShipsDataString vector
            }

            //Update buoy data
            std::vector<std::string> buoysDataString = Utilities::split(receivedData.at(4),'|');
            if (numberBuoys == buoysDataString.size()) {
                findBuoyPositionData(buoysDataString, buoysData); //Populate buoysData from the buoysDataString vector
            } //Check number of buoys matches the amount of data

        } //Check if 3 number elements for Other ships, buoys and MOBs

        //Weather data in record 7 (Weather, rain, vis etc)
        std::vector<std::string> weatherData = Utilities::split(receivedData.at(7),',');
        if (weatherData.size() == 5) {
            //Weather at 0, Vis at 1, rain at 3
            weather = Utilities::lexical_cast<irr::f32>(weatherData.at(0));
            visibility = Utilities::lexical_cast<irr::f32>(weatherData.at(1));
            rain = Utilities::lexical_cast<irr::f32>(weatherData.at(3));
        }

    } //Check correct number of records received
}

void Network::findOwnShipPositionData(const std::vector<std::string>& positionData, ShipData& ownShipData)
{
    if (positionData.size() == 8) { //8 elements in position data sent
        ownShipData.X = Utilities::lexical_cast<irr::f32>(positionData.at(0));
        ownShipData.Z = Utilities::lexical_cast<irr::f32>(positionData.at(1));
        ownShipData.heading = Utilities::lexical_cast<irr::f32>(positionData.at(2));
    }
}

void Network::findOtherShipData(const std::vector<std::string>& otherShipsDataString, std::vector<OtherShipDisplayData>& otherShipsData)
{
    //Ensure otherShipsData vector is the right size
    if (otherShipsData.size() != otherShipsDataString.size()) {
        otherShipsData.resize(otherShipsDataString.size());
    }

    //Check this has been successful
    if (otherShipsData.size() != otherShipsDataString.size()) {
        std::cout << "Could not resize otherShipsData" << std::endl;
        exit(EXIT_FAILURE);
    }

    for (irr::u32 i=0; i<otherShipsDataString.size(); i++) {
        std::vector<std::string> thisShipData = Utilities::split(otherShipsDataString.at(i),',');
        if (thisShipData.size() == 7) { //7 elements for each ship
            //Update data
            otherShipsData.at(i).X=Utilities::lexical_cast<irr::u32>(thisShipData.at(0));
            otherShipsData.at(i).Z=Utilities::lexical_cast<irr::u32>(thisShipData.at(1));

            //Todo: use SART etc
            irr::u32 numberOfLegs = Utilities::lexical_cast<irr::u32>(thisShipData.at(5));
            std::vector<std::string> legsDataString = Utilities::split(thisShipData.at(6),'/');
            if (numberOfLegs == legsDataString.size()) {
                //Ensure legs vector is the right size
                if (otherShipsData.at(i).legs.size() != legsDataString.size()) {
                    otherShipsData.at(i).legs.resize(legsDataString.size());
                }

                //Check this has been successful
                if (otherShipsData.at(i).legs.size() != legsDataString.size()) {
                    std::cout << "Could not resize otherShipsData.at(i).legs" << std::endl;
                    exit(EXIT_FAILURE);
                }


                //Populate the leg data
                for (irr::u32 j=0; j<legsDataString.size(); j++) {
                    std::vector<std::string> thisLegData = Utilities::split(legsDataString.at(j),':');
                    if (thisLegData.size() ==3) {
                        otherShipsData.at(i).legs.at(j).bearing = Utilities::lexical_cast<irr::f32>(thisLegData.at(0));
                        otherShipsData.at(i).legs.at(j).speed = Utilities::lexical_cast<irr::f32>(thisLegData.at(1));
                        otherShipsData.at(i).legs.at(j).startTime = Utilities::lexical_cast<irr::f32>(thisLegData.at(2));

                        //std::cout << "Ship " << i << " Leg " << j << " Bearing " << otherShipsData.at(i).legs.at(j).bearing << " Speed " << otherShipsData.at(i).legs.at(j).speed << " Start Time " << otherShipsData.at(i).legs.at(j).startTime << std::endl;

                    }
                }//Iterate through legs

            }//If number of legs matches data received

        }//Check number of basic data elements
    } //Iterate through ships

}

void Network::findBuoyPositionData(const std::vector<std::string>&buoysDataString, std::vector<PositionData>& buoysData)
{
    //Ensure buoysData vector is the right size
    if (buoysData.size() != buoysDataString.size()) {
        buoysData.resize(buoysDataString.size());
    }

    //Check this has been successful
    if (buoysData.size() != buoysDataString.size()) {
        std::cout << "Could not resize buoysData" << std::endl;
        exit(EXIT_FAILURE);
    }

    for (irr::u32 i=0; i<buoysDataString.size(); i++) {
        std::vector<std::string> thisBuoyData = Utilities::split(buoysDataString.at(i),',');
        if (thisBuoyData.size() == 2) {
            //Update data
            buoysData.at(i).X=Utilities::lexical_cast<irr::u32>(thisBuoyData.at(0));
            buoysData.at(i).Z=Utilities::lexical_cast<irr::u32>(thisBuoyData.at(1));
        } //Check if buoy data contains 2 elements for X,Z
    } //Iterate through buoys
}
