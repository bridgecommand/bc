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

#ifndef __NETWORKPRIMARY_HPP_INCLUDED__
#define __NETWORKPRIMARY_HPP_INCLUDED__

#include "Network.hpp"

#include <string>

#ifdef _WIN32
#include <enet/enet.h>
#endif // _WIN32

//Forward declarations
class SimulationModel;

class NetworkPrimary : public Network
{
public:
    NetworkPrimary(SimulationModel* model);
    ~NetworkPrimary();

    void connectToServer(std::string hostnames);
    void update();

private:
    #ifdef _WIN32
    SimulationModel* model;

    ENetHost* client; //One client
    ENetEvent event;
    #endif // _WIN32

    std::string generateSendString(); //Prepare then normal data message to send
    std::string generateSendStringSC(); //Prepare the 'SC' message, with scenario information
    void sendNetwork();
    void receiveNetwork();

};

#endif
