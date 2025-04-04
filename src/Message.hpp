#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include "MessageMisc.hpp"
#include "SimulationModel.hpp"
#include "Constants.hpp"

class Message
{
 public:
  
  Message(SimulationModel* aModel);
  Message();
  ~Message();
  eCmdMsg Parse(const char *aData, size_t aDataSize, void** aCmdData);
  eCmdMsg ParseMapController(std::string& aMsg, void** aCmdData);
  eCmdMsg ParseMasterCommand(std::string& aMsg, void** aCmdData);
  eCmdMsg ParseMultiPlayer(std::string& aMsg, void** aCmdData);
  eCmdMsg ParseOwnShip(std::string& aMsg, void** aCmdData);
  eCmdMsg ParseScenario(std::string& aMsg, void** aCmdData);
  eCmdMsg ParseShutDown(std::string& aMsg, void** aCmdData);
  eCmdMsg ParseWindInjection(std::string& aMsg, void** aCmdData);
  std::string& KeepAliveShort(void);
  std::string& KeepAlive(void);
  std::string& MakeLines(void);
  static std::string& ShutDown(void);
  std::string& MpFeedBack(void);
  std::string& ControlOverride(void);
private:
  sUpLeg* UpdateLeg(std::string aCmd);
  sDelLeg* DeleteLeg(std::string aCmd);
  sRepoShip* RepositionShip(std::string aCmd);
  sResetLegs* ResetLegs(std::string aCmd);
  sWeather* SetWeather(std::string aCmd);
  sMob* ManOverboard(std::string aCmd);
  sMmsi* SetMMSI(std::string aCmd);
  sRuddWork* RudderWorking(std::string aCmd);
  sRuddFol* RudderFollowUp(std::string aCmd);
  sCtrlOv* CtrlOverride(std::string aCmd);
  sTimeInf GetTimeInfos(std::vector<std::string>& aTimeData);
  sShipInf GetInfosOwnShip(std::vector<std::string>& aOwnShipData);
  void GetInfosOtherShips(std::vector<std::string>& aOtherShipsData, unsigned int aNumberOthers, sOthShipInf& othersShipsInfos);
  sMobInf GetInfosMob(std::vector<std::string>& aMobData, unsigned int aNbrMob);
  sLinesInf GetInfosLines(std::vector<std::string>& aLinesData, unsigned int aNumberLines);
  sWeatherInf GetInfosWeather(std::vector<std::string>& aWeatherData);
  sViewInf GetInfosView(std::vector<std::string>& aViewData);
  sCtrlsInf GetInfosControls(std::vector<std::string>& aCtrlsData);
  SimulationModel* mModel; /*Only for getter*/
  
};

typedef struct{
  std::string header;
  eCmdMsg (Message::*pFuncParse)(std::string& aMsG, void** aCmdData);
}sParseHeader;


#endif
