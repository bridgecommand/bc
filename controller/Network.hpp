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

#include <enet/enet.h>

#include "PositionDataStruct.hpp"
#include "ShipDataStruct.hpp"
#include "OtherShipDataStruct.hpp"

//Forward declarations
class ControllerModel;

class Network
{
public:
    Network();
    ~Network();

    void update(irr::f32& time, ShipData& ownShipData, std::vector<OtherShipData>& otherShipsData, std::vector<PositionData>& buoysData);

private:

    ControllerModel* model;
    ENetAddress address;
    ENetHost * server;

    ENetEvent event;
    std::string stringToSend;
    ENetPacket * packet;

    void receiveMessage(irr::f32& time, ShipData& ownShipData, std::vector<OtherShipData>& otherShipsData, std::vector<PositionData>& buoysData); //Acts on 'event'
    //Subroutines to break down process of extracting data from the received string:
    void findDataFromString(const std::string& receivedString, irr::f32& time, ShipData& ownShipData, std::vector<OtherShipData>& otherShipsData, std::vector<PositionData>& buoysData);
    void findOwnShipPositionData(const std::vector<std::string>& positionData, ShipData& ownShipData);
    void findOtherShipData(const std::vector<std::string>& otherShipsDataString, std::vector<OtherShipData>& otherShipsData);
    void findBuoyPositionData(const std::vector<std::string>& buoysDataString, std::vector<PositionData>& buoysData);

    void sendMessage(ENetPeer * peer);

};
#endif // __NETWORK_HPP_INCLUDED__
