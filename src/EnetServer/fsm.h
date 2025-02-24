#ifndef FSM_H
#define FSM_H

#include "com.h"

#define WATCHDOG_COUNTER (30000)

class Fsm
{
 public:
  
  Fsm(Com& aCom);
  ~Fsm();

  void Run(void);
    
 private:

  Com mCom;
  
  
};

#endif
