#include "Update.hpp"

Update::Update()
{
}

Update::~Update()
{    
}

void Update::UpdateNetwork(SimulationModel* aModel, Network* aNet, OperatingMode::Mode aMode)
{
  eCmdMsg msgType;
  void* dataCmd = NULL;
  Message inMsg(aModel), outMsg(aModel);

  aNet->WaitMessage(inMsg, msgType, &dataCmd);
  std::cout << "msg : " << msgType << std::endl;
  aModel->updateFromNetwork(msgType, dataCmd);	  

  if(OperatingMode::Secondary == aMode)
    {
      std::string msgCtrlOv = outMsg.ControlOverride();
      aNet->SendMessage(msgCtrlOv);
    }
  else if(OperatingMode::Multiplayer == aMode)
    {
      std::string msgMpFeedBack = outMsg.MpFeedBack();
      aNet->SendMessage(msgMpFeedBack);
    }
  else
    {
      if(aModel->getLoopNumber() % 100 == 0)
	{
	  std::string msgKeepAliveScn = aModel->getSerialisedScenario();
	  aNet->SendMessage(msgKeepAliveScn, true);
	}
      else if(aModel->getLoopNumber() % 10 == 0)
	{
	  std::string msgKeepAlive = outMsg.KeepAlive();
	  aNet->SendMessage(msgKeepAlive);
	}
      else
	{
	  std::string msgKeepAliveShort = outMsg.KeepAliveShort();      
	  aNet->SendMessage(msgKeepAliveShort);
	}    
    } 
}	  
