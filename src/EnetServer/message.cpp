#include <iostream>
#include "message.h"

Message::Message()
{
}

Message::~Message()
{    
}


eMsgSrc Message::Process(std::string& aMsg)
{
    
  if(aMsg.substr(0,3) == "SCN"
     || aMsg.substr(0,2) == "BC"
     || aMsg.substr(0,2) == "OS"
     || aMsg.substr(0,2) == "SD")
    {
      //std::cout << "------> " << aMsg << std::endl;
      return E_MSG_FROM_MASTER;    
    }
  
  if(aMsg.substr(0,2) == "MC")
    {
      //std::cout << "------> " << aMsg << std::endl;
      return E_MSG_FROM_MC;    
    }
  return E_MSG_FROM_UNKNOW_HOST;
}
