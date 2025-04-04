#ifndef STATUS_H
#define STATUS_H

#include "miscstatus.h"

class ComStatus
{
 public:
  
  ComStatus();
  ~ComStatus();

  eServState getComState(void);
  void setComState(eServState aComState);
  
 private:

  eServState mComState;
  unsigned char mConnexionCount;
};

#endif
