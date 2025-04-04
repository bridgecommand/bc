#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include "miscstatus.h"

class Message
{
 public:
  
  Message();
  ~Message();

  static int Process(std::string& aMsg);
    
 private:

  
};

#endif
