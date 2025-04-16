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

#ifndef __NETWORK_HPP_INCLUDED__
#define __NETWORK_HPP_INCLUDED__

#include <string>
#include <vector>
#include <enet/enet.h>
#include "irrlicht/irrlicht.h"
#include "OperatingModeEnum.hpp"
#include "Message.hpp"

class Message;

#define DEFAULT_PORT (18304)
#define MAX_PEERS (2) 

class Network
{
public:
  Network();
  ~Network();
  int Connect(std::string aAddr = "localhost", unsigned int aPort = DEFAULT_PORT, OperatingMode::Mode aMode = OperatingMode::Normal);
  void WaitMessage(Message& aInMessage, eCmdMsg& aMsgType, void** aCmdData, unsigned int aTimeout, bool aParse=true);
  int SendMessage(std::string& aMsg, bool aIsReliable=false);
  std::string GetIPServer(void);
  
private:
  ENetAddress mServAddr;
  ENetHost* mClient;
  ENetPeer* mPeer;

};

#endif
