#include "comstatus.h"

ComStatus::ComStatus()
{
  mComState = E_SERVER_DISCONNECTED;
  mConnexionCount = 0;  
}

ComStatus::~ComStatus()
{    
}


eServState ComStatus::getComState(void){return mComState;}

void ComStatus::setComState(eServState aComState){mComState = aComState;}
