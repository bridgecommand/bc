#ifndef MESSAGE_MISC_HPP
#define MESSAGE_MISC_HPP

#define MAX_HEADER_MSG (4)
#define MAX_RECORD_BC_MSG (13)

/*****************Enum cmds*****************************/
typedef enum{
  E_CMD_MESSAGE_UPDATE_LEG=0x10,
  E_CMD_MESSAGE_DELETE_LEG,
  E_CMD_MESSAGE_REPOSITION_SHIP,
  E_CMD_MESSAGE_RESET_LEGS,
  E_CMD_MESSAGE_SET_WEATHER,
  E_CMD_MESSAGE_MAN_OVERBOARD,
  E_CMD_MESSAGE_SET_MMSI,
  E_CMD_MESSAGE_RUDDER_WORKING,
  E_CMD_MESSAGE_RUDDER_FOLLOW_UP,
  E_CMD_MESSAGE_CONTROLS_OVERRIDE,
  E_CMD_MESSAGE_BRIDGE_COMMAND,
  E_CMD_MESSAGE_OWN_SHIP,
  E_CMD_MESSAGE_SCENARIO,
  E_CMD_MESSAGE_UNKNOWN=0x99
}eCmdMsg;

/*****************Struct usefull for process BC cmds*****************************/
typedef struct{
  float timeD;
  float accel;
}sTimeInf;

typedef struct{
  float posX;
  float posZ;
  float hdg;
  float rot;
  float speed;
}sShipInf;

typedef struct{
  unsigned int nbrShips;
  sShipInf* ships;
}sOthShipInf;

typedef struct{
  bool isMob;
  float posX;
  float posZ;
}sMobInf;

typedef struct{
  unsigned int lineNbr;
  float lineStartX;
  float lineStartY;
  float lineStartZ;
  float lineEndX;
  float lineEndY;
  float lineEndZ;
  int lineStartType;
  int lineEndType;
  int lineStartID;
  int lineEndID;
  float lineNominalLength;
  float lineBreakingTension;
  float lineBreakingStrain;
  float lineNominalShipMass;
  int lineKeepSlackInt;
  int lineHeaveInInt;
}sLinesInf;

typedef struct{
  float weather;
  float visibility;
  float rain;
}sWeatherInf;

typedef struct{
  float view;
}sViewInf;

typedef struct{
  float wheel;
  float rudder;
  float portEng;
  float stbdEng;
  float portSch;
  float stbdSch;
  float portThrust;
  float stbdThrust;
  float bowThrust;
  float sternThrust;
}sCtrlsInf;

typedef struct{
  sTimeInf time;
  sShipInf ownShip;
  sOthShipInf otherShips;
  sMobInf mob;
  sLinesInf lines;
  sWeatherInf weather;
  sViewInf view;
  sCtrlsInf controls; 
}sMasterCmdsInf;

/*****************Struct usefull for process MC cmds*****************************/
typedef struct{
  int shipNo;
  int legNo;
  float bearing;
  float speed;
  float dist;
}sUpLeg;

typedef struct{
  int shipNo;
  int legNo;
}sDelLeg;

typedef struct{
  int shipNo;
  int posX;
  int posZ;
}sRepoShip;

typedef struct{
  int shipNo;
  int posX;
  int posZ;
  int cog;
  int sog;
}sResetLegs;

typedef struct{
  int weather;
  int rain;
  int visibility;
}sWeather;

typedef struct{
  int mobMode;
}sMob;

typedef struct{
  int shipNo;
  unsigned int mmsi;
}sMmsi;

typedef struct{
  int whichPump;
  int rudderFunction;
}sRuddWork;

typedef struct{
  int rudderFunction;
}sRuddFol;

typedef struct{
  unsigned int overrideMode;
  float overrideData;
}sCtrlOv;

#endif
