#include <thread>
#include "Update.hpp"

Update::Update()
{
}

Update::~Update()
{    
}

void Update::UpdateNetwork(SimulationModel* aModel, Network* aNet, OperatingMode::Mode aMode)
{
	eCmdMsg msgType = E_CMD_MESSAGE_UNKNOWN;
	void* dataCmd = NULL;
	Message inMsg(aModel), outMsg(aModel);
	unsigned int timeout = 1;

	while (true)
	{
		msgType = E_CMD_MESSAGE_UNKNOWN;

		aNet->WaitMessage(inMsg, msgType, &dataCmd, timeout);
		aModel->updateFromNetwork(msgType, dataCmd);

		if (OperatingMode::Secondary == aMode)
		{
			std::string msgCtrlOv = outMsg.ControlOverride();
			aNet->SendMessage(msgCtrlOv);
		}
		else
		{

			if (aModel->getLoopNumber() % 100 == 0)
			{
				std::string msgKeepAliveScn = aModel->getSerialisedScenario();
				aNet->SendMessage(msgKeepAliveScn, true);
			}
			else if (aModel->getLoopNumber() % 6 == 0)
			{
				std::string msgKeepAlive = outMsg.KeepAlive();
				aNet->SendMessage(msgKeepAlive);

				if (OperatingMode::Multiplayer == aMode)
				{
					std::string msgMpFeedBack = outMsg.MpFeedBack();
					aNet->SendMessage(msgMpFeedBack);

				}
			}
			/*else if (aModel->getLoopNumber() % 10 == 0)
			  {
			  std::string msgKeepAliveShort = outMsg.KeepAliveShort();
			  aNet->SendMessage(msgKeepAliveShort);
			  }*/
		}
	}
}	  
