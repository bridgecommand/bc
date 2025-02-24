#include <vector>
#include "com.h"
#include "message.h"

#ifdef _WIN32
	#include <windows.h>
	#define SLEEP Sleep(1000)
#else
	#define SLEEP sleep(1)	
#endif

Com::Com(std::string aAddr, unsigned short aPort)
{    
  mServer = NULL;
  mAddrServ.port = aPort;
  enet_address_set_host (&mAddrServ, aAddr.c_str());
  mClientCounter = 0;
  mMsgToMaster = 0;
  mMsgToSlave = 0;
  for(unsigned char i=0; i<=MAX_CLIENT_CONNEXION; i++)
    {
      mPeerClient[i] = NULL;
    }
}

Com::Com()
{    
  mServer = NULL;
  mAddrServ.host = ENET_HOST_ANY;
  mAddrServ.port = 0;
  mClientCounter = 0;
  mMsgToMaster = 0;
  mMsgToSlave = 0;
  for(unsigned char i=0; i<=MAX_CLIENT_CONNEXION; i++)
    {
      mPeerClient[i] = NULL;
    }  
}


Com::~Com()
{
  enet_host_destroy(mServer);
  enet_deinitialize();
}

int Com::InitCom(void)
{
  unsigned char retryCount = 0;
  int ret = -1;
  char ipAddr[16] ={0};
 
  
  if(0 != enet_initialize())
    {
      std::cout << "An error occurred while initializing Enet.\n";
      return ret;
    }
  
  while(NULL == mServer && MAX_RETRY_COUNTER > retryCount)
    {
      mServer = enet_host_create(&mAddrServ, MAX_CLIENT_CONNEXION, 2, 0, 0);			       
      if (NULL == mServer)
	{
	  retryCount++;
	  SLEEP;
	}
    }

  if (NULL == mServer)
    {
      std::cout << "An error occurred while trying to create an ENet server host." << std::endl;
      return ret;
    }
  else
    {
      enet_address_get_host_ip(&mServer->address, ipAddr, 16);
      std::cout << "Listening UDP on "  << ipAddr << ":" << mServer->address.port << std::endl;
      ret = 0;
    }

  return ret;
}

int Com::ClientConnect(ENetPeer** aPeer, unsigned char aData)
{
  bool isConnected = false;
  char ipAddr[16] = {0};
  int ret = -1;
  
  std::cout << "-- Connect Event received --"  << std::endl;

  if(mClientCounter <= MAX_CLIENT_CONNEXION)
    {
      for(unsigned char i=0; i<=mClientCounter; i++)
	{
	  if(NULL != mPeerClient[i])
	    {
	      if((*aPeer)->address.host == mPeerClient[i]->address.host &&
		 (*aPeer)->address.port == mPeerClient[i]->address.port)
		{
		  std::cout << "Client already connected!"  << std::endl;
		  isConnected = true;
		  ret = 1;
		}
	    }
	}

      if(!isConnected)
	{
	  mTypeClient[mClientCounter] = aData;
     	  mPeerClient[mClientCounter] = *aPeer;
	  enet_address_get_host_ip(&mPeerClient[mClientCounter]->address, ipAddr, 16);
	  std::cout << "Client :" << ipAddr << ":" << mPeerClient[mClientCounter]->address.port << " connected" << std::endl;
	  mClientCounter++;
	  ret = 0;
	}
    }
  else
    std::cout << "No more connexions available!"  << std::endl;
  
  return ret;
}


int Com::ClientDisconnect(ENetPeer** aPeer)
{
  char ipAddr[16] ={0};
  int ret = -1;
  
  std::cout << "-- Disconnect Event received --"  << std::endl;

  for(unsigned char i=0; i<=mClientCounter; i++)
    {
      if(NULL != mPeerClient[i])
	{	  
	  if((*aPeer)->address.host == mPeerClient[i]->address.host &&
	     (*aPeer)->address.port == mPeerClient[i]->address.port)
	    {
	      enet_address_get_host_ip(&mPeerClient[i]->address, ipAddr, 16);
	      std::cout << "Client :" << ipAddr << ":" << mPeerClient[i]->address.port << " disconnected" << std::endl;

	      mPeerClient[i] = {0};
	      mClientCounter--;
	      ret = 0;
	    }
	  else
	    {
	      enet_address_get_host_ip(&(*aPeer)->address, ipAddr, 16);
	      std::cout << "Client :" << ipAddr << "never been connected" << std::endl;
	      ret = 1;
	    }
	}
    }
  
  return ret;
}

int Com::ClientMsg(const char *aData, size_t aDataSize)
{
  std::string tmpBuffer(aData, aDataSize);
  
  //std::cout << "-- Message Event received --"  << std::endl;
  
  if(tmpBuffer.length() > 4)
    {
      if(E_MSG_FROM_MASTER == Message::Process(tmpBuffer))
	{
	  //std::cout << "Message from BC to MC"  << std::endl;
	  mMsgToSlave = true;
	}
      else if(E_MSG_FROM_MC == Message::Process(tmpBuffer))
	{
	  //std::cout << "Message from MC to BC"  << std::endl;
	  mMsgToMaster = true;
	}
    }

  return 0;
}

void Com::RouteMsg(void)
{
  eTarget target = UNKNOWN;
  if(true == mMsgToMaster)
    {
      target = MASTER;
      mMsgToMaster = false;
    }
  
  if(true == mMsgToSlave)
    {
      target = SLAVE;
      mMsgToSlave = false;
    }

   SendMsg(target);
}

void Com::SendMsg(eTarget aTarget)
{
  mPacket = enet_packet_create(mEvent.packet->data, mEvent.packet->dataLength + 1, 1);

  for(unsigned char i=0; i<mClientCounter; i++)
    {
      if(NULL != mPeerClient[i])
	{
	  if(mPeerClient[i]->address.host != 0 && mTypeClient[i] == aTarget)
	    {
	      enet_peer_send(mPeerClient[i], 0, mPacket);
	      //std::cout << "Send Message ! size : " << mEvent.packet->dataLength << std::endl;
	    }
	}
    }
  enet_host_flush (mServer);
}

int Com::WaitEvent(unsigned short aTimeout)
{
  int retEvent = -1, ret = 1;

  retEvent = enet_host_service(mServer, &mEvent, aTimeout);
  
  if(0 < retEvent)
    {
      //std::cout << "-- Event received : " << mEvent.type << " --"  << std::endl;
      
      switch(mEvent.type)
	{
	case ENET_EVENT_TYPE_RECEIVE:
	  {
	    ret = ClientMsg((char*)mEvent.packet->data, mEvent.packet->dataLength);
	    RouteMsg();
	    break;
	  }
	case ENET_EVENT_TYPE_CONNECT:
	  {
	    ret = ClientConnect(&mEvent.peer, (unsigned char)mEvent.data);
	    break;
	  }
	case ENET_EVENT_TYPE_DISCONNECT:
	  {
	    ret = ClientDisconnect(&mEvent.peer);
	    break;
	  }

	default:
	  break;
	}
 
      enet_packet_destroy (mEvent.packet);
    }
  else if(0 == retEvent)
    {
      //nothing to do, no event
      ret = 1;
    }
  else ret = -1;
  
  return ret;
}


eServState Com::GetState(void)
{
  return mStatus.getComState();
}

void Com::SetState(eServState aState)
{
  mStatus.setComState(aState);
}
