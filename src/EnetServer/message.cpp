#include <iostream>
#include "message.h"

Message::Message()
{
}

Message::~Message()
{    
}


eMsgDest Message::Process(std::string& aMsg)
{
    
  if(aMsg.substr(0, 3) == "SCN"
     || aMsg.substr(0,2) == "BC"
     || aMsg.substr(0,2) == "OS"
     || aMsg.substr(0,2) == "SD") 
    {
      return E_MSG_TO_SLAVE;
    }

  if (aMsg.substr(0, 3) == "SCN")
  {
      return E_MSG_TO_SLAVE;
  }

    if (aMsg.substr(0, 3) == "MPH")
        {
            return E_MSG_TO_MH;
    }
  
  if(aMsg.substr(0,2) == "MC"
      || aMsg.substr(0, 2) == "MH")
    {
      return E_MSG_TO_MASTER;
    }

  return E_MSG_TO_UNKNOW_HOST;
}
