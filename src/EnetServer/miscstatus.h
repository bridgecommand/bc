#ifndef MISCSTATUS_H
#define MISCSTATUS_H

typedef enum{

  E_SERVER_DISCONNECTED=0x44,
  E_SERVER_WAITING_CONNEXION,
  E_SERVER_ONLINE
  
}eServState;

typedef enum{

  E_MSG_TO_BC=0x55,
  E_MSG_TO_MC,
  E_MSG_TO_SOLVER
  
}eMsgDest;

typedef enum{

  E_MSG_FROM_BC=0x66,
  E_MSG_FROM_MC,
  E_MSG_FROM_SOLVER,
  E_MSG_FROM_UNKNOW_HOST=0x99
  
}eMsgSrc;


#endif
