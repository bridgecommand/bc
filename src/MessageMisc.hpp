#ifndef MESSAGE_MISC_HPP
#define MESSAGE_MISC_HPP

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
  E_CMD_MESSAGE_UNKNOWN=0x99
}eCmdMsg;

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
