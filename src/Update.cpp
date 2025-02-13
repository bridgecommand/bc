#include "Update.hpp"

Update::Update()
{
}

Update::~Update()
{    
}

void Update::UpdateNetwork(SimulationModel* aModel, Network* aNet)
{
  eCmdMsg msgType;
  void* dataCmd = NULL;
  Message inMsg(aModel), outMsg(aModel);
  std::string msgKeepAliveScn = aModel->getSerialisedScenario();
  std::string msgKeepAlive = outMsg.KeepAlive();
  std::string msgKeepAliveShort = outMsg.KeepAliveShort();
  
  aNet->WaitMessage(inMsg, msgType, dataCmd);

  aModel->updateFromNetwork(msgType, dataCmd);	  

  if(aModel->getLoopNumber() % 100 == 0)
    aNet->SendMessage(msgKeepAliveScn, true);
  else if(aModel->getLoopNumber() % 10 == 0)
    aNet->SendMessage(msgKeepAlive);
  else 
    aNet->SendMessage(msgKeepAliveShort);
}	  
