#include "fsm.h"

Fsm::Fsm(Com& aCom)
{
  mCom = aCom;
}

Fsm::~Fsm()
{    
}


void Fsm::Run(void)
{
  int retEvent = -1;
  bool isRunning = false;
  unsigned short timeout = 0, watchdog = 0;
  
  if(0 == mCom.InitCom())
    {
      isRunning = true;
      mCom.SetState(E_SERVER_WAITING_CONNEXION);
      timeout = 1000;
    }
  else
    {
      std::cout << "Server is not running, Initialisation failed !"  << std::endl;
      return;
    }
  
  while(isRunning)
    {
      retEvent = mCom.WaitEvent(timeout);
      
      switch(mCom.GetState())
	{
	case E_SERVER_WAITING_CONNEXION:
	  {
	    timeout = 1000;
	    if(retEvent > 0)
	      std::cout << "EnetServer :: Waiting connection..." << std::endl;
	    else if(retEvent < 0)
	      mCom.SetState(E_SERVER_DISCONNECTED);
	    else
	      mCom.SetState(E_SERVER_ONLINE); 
	    
	    break;
	  }
	case E_SERVER_ONLINE:
	  {
	    timeout = 100;
	    if(retEvent > 0)
	      watchdog++;
	    else if(retEvent < 0)
	      mCom.SetState(E_SERVER_DISCONNECTED);
	    else
	      watchdog=0;
	    
	    if(watchdog > WATCHDOG_COUNTER)
	      {
		mCom.SetState(E_SERVER_DISCONNECTED);
		std::cout << "Watchog!" << std::endl;
	      }
	    break;
	  }

	case E_SERVER_DISCONNECTED:
	default:
	  {
	    std::cout << "Server disconnected!" << std::endl;
	    isRunning=false;
	  }
	}
    }
}
