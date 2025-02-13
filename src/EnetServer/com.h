#ifndef COM_H
#define COM_H

#include <iostream>
#include <vector>
#include <enet/enet.h>
#include "miscstatus.h"
#include "comstatus.h"

#define MAX_RETRY_COUNTER (10)
#define MAX_CLIENT_CONNEXION (4)
#define MAX_RECEIVE_BUFFER (8192)

class Com
{
 public:

  Com();
  Com(std::string aAddr, unsigned short aPort);
  ~Com();

  int InitCom(void);
  int WaitEvent(unsigned short aTimeout);
  
  eServState GetState(void);
  void SetState(eServState aState);
  
 private:

  int ClientConnect(ENetPeer** aPeer);
  int ClientDisconnect(ENetPeer** aPeer);
  int ClientMsg(const unsigned char *aData);  
  void RouteMsg(void);
  void SendMsg(void);
  
  /*Server*/
  ENetAddress mAddrServ;
  ENetHost* mServer;
  ENetEvent mEvent;
  ComStatus mStatus;

  /*Client*/
  ENetPacket* mPacket;
  ENetPeer* mPeerClient[MAX_CLIENT_CONNEXION];
  unsigned char mClientCounter;

  /*Flag Broadcast*/
  bool mMsgToBd;
  
};

#endif
