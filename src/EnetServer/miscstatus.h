#ifndef MISCSTATUS_H
#define MISCSTATUS_H

typedef enum{

  E_SERVER_DISCONNECTED=0x44,
  E_SERVER_WAITING_CONNEXION,
  E_SERVER_ONLINE
  
}eServState;

#define E_MSG_TO_MC			 (0x01)
#define E_MSG_TO_MASTER		 (0x02)
#define E_MSG_TO_SLAVE		 (0x04)
#define E_MSG_TO_MH			 (0x08)
#define E_MSG_TO_MASTER_MP	 (0x10)
#define E_MSG_TO_UNKNOW_HOST (0x20)



#endif
