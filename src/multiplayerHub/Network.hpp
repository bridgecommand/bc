/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2016 James Packer

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

class Network
{
public:
    Network(int port);
    ~Network();
    void connectToServer(std::string hostnames);
    unsigned int getNumberOfPeers();

    void sendString(std::string stringToSend, bool reliable, unsigned int peerNumber);
    void listenForMessages();
    std::string getLatestMessage(unsigned int peerNumber);


private:
    int port;

    ENetHost* client; //One client
    ENetEvent event;
    std::vector<ENetPeer*> peers;
    std::vector<std::string> latestMessageFromPeer;

};

#endif
