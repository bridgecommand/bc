/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2015 James Packer

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

#ifndef __NETWORK_HPP_INCLUDED__
#define __NETWORK_HPP_INCLUDED__

#include <string>
#include <vector>

#include <enet/enet.h> //Main networking

#include "PositionDataStruct.hpp"
#include "ShipDataStruct.hpp"
#include "OtherShipDataStruct.hpp"

//Forward declarations
class ControllerModel;

class Network
{
public:
    Network(int port);
    ~Network();

    std::string findWorldName();
    int getPort();
    void update(irr::f32& time, ShipData& ownShipData, std::vector<OtherShipDisplayData>& otherShipsData, std::vector<PositionData>& buoysData, irr::f32& weather, irr::f32& visibility, irr::f32& rain, bool& mobVisible, PositionData& mobData, irr::f32& windDirection, irr::f32& windSpeed, irr::f32& streamDirection, irr::f32& streamSpeed, bool& streamOverride);
    void setStringToSend(std::string stringToSend);

private:

    ControllerModel* model;
    ENetAddress address;
    ENetHost * server;

    ENetEvent event;
    std::string stringToSend;
    ENetPacket * packet;

    void receiveMessage(irr::f32& time, ShipData& ownShipData, std::vector<OtherShipDisplayData>& otherShipsData, std::vector<PositionData>& buoysData, irr::f32& weather, irr::f32& visibility, irr::f32& rain, bool& mobVisible, PositionData& mobData, irr::f32& windDirection, irr::f32& windSpeed, irr::f32& streamDirection, irr::f32& streamSpeed, bool& streamOverride); //Acts on 'event'
    //Subroutines to break down process of extracting data from the received string:
    void findDataFromString(const std::string& receivedString, irr::f32& time, ShipData& ownShipData, std::vector<OtherShipDisplayData>& otherShipsData, std::vector<PositionData>& buoysData, irr::f32& weather, irr::f32& visibility, irr::f32& rain, bool& mobVisible, PositionData& mobData, irr::f32& windDirection, irr::f32& windSpeed, irr::f32& streamDirection, irr::f32& streamSpeed, bool& streamOverride);
    void findOwnShipPositionData(const std::vector<std::string>& positionData, ShipData& ownShipData);
    void findOtherShipData(const std::vector<std::string>& otherShipsDataString, std::vector<OtherShipDisplayData>& otherShipsData);
    void findBuoyPositionData(const std::vector<std::string>& buoysDataString, std::vector<PositionData>& buoysData);

    void sendMessage(ENetPeer * peer);

};
#endif // __NETWORK_HPP_INCLUDED__
