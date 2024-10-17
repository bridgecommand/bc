#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include "miscstatus.h"

class Message
{
 public:
  
  Message();
  ~Message();

  static eMsgSrc Process(std::string& aMsg);
    
 private:

  
};

#endif
