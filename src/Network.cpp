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
#include "Leg.hpp"
#include <iostream>
#include <cstdio>
#include <string>

Network::Network()
{  
  if(0 != enet_initialize ())
    {
      std::cerr << "An error occurred while initialising ENet" << std::endl;
    }
  else
    {
      mClient = enet_host_create(NULL, MAX_PEERS, 2, 0, 0);

      if (NULL == mClient)
	{
	  std::cerr << "Error occurred while trying to create an ENet client host." << std::endl;
	  enet_deinitialize();
	}
    }
}


Network::~Network()
{
  enet_host_destroy(mClient);
  enet_deinitialize();
}

int Network::Connect(std::string aAddr, unsigned int aPort, OperatingMode::Mode aMode)
{
  int ret = -1;
  ENetEvent event;
  
  enet_address_set_host (&mServAddr, aAddr.c_str());
  mServAddr.port = aPort;
  
  mPeer = enet_host_connect(mClient, &mServAddr, ENET_PROTOCOL_MAXIMUM_CHANNEL_COUNT, aMode);

  if(NULL == mPeer)
    {
      std::cerr << "No available peers for initiating an ENet connection." << std::endl;
      enet_deinitialize();
      return ret;
    }
    
  if(enet_host_service(mClient, &event, 1000) > 0 &&
     event.type == ENET_EVENT_TYPE_CONNECT)
    {
      std::cout << "Connect to server : " << aAddr << ":" << aPort << std::endl;
      enet_host_flush(mClient);
      ret=0;
    }
  else
      enet_peer_reset(mPeer);

  return ret;
}

void Network::WaitMessage(Message& aInMessage, eCmdMsg& aMsgType, void** aCmdData, unsigned int aTimeout)
{
  ENetEvent event;    

  if(enet_host_service(mClient, &event, aTimeout) > 0)
    {
      if(ENET_EVENT_TYPE_RECEIVE == event.type)
	{
	  aMsgType = aInMessage.Parse((char*)event.packet->data, event.packet->dataLength, aCmdData);
	  enet_packet_destroy(event.packet);
        }
    }
}

int Network::SendMessage(std::string& aMsg, bool aIsReliable)
{
  int ret = -1;

  if(aMsg.length() > 0)
    {
      enet_uint32 packetFlag = 0;
      if (aIsReliable) 
	packetFlag = ENET_PACKET_FLAG_RELIABLE;

      ENetPacket* packet = enet_packet_create(aMsg.c_str(), aMsg.length(), packetFlag);
      enet_peer_send(mPeer, 0, packet);
      enet_host_flush(mClient);
      ret = 0;
    }
  return ret;
}

std::string Network::GetIPServer(void)
{
  char ipAddr[16] ={0};
  std::string retStr;
  
  enet_address_get_host_ip(&mServAddr, ipAddr, 16);
  retStr = ipAddr;
  retStr.append(":");
  retStr.append(Utilities::lexical_cast<std::string>(mServAddr.port));

  return retStr;
}
