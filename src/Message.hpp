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
  ~Message();
  eCmdMsg Parse(const char *aData, size_t aDataSize, void *aCmdData);
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
  std::string KeepAliveShort(void);
  std::string KeepAlive(void);
  std::string MakeLines(void);
  static std::string ShutDown(void);
private:
  SimulationModel* mModel; /*Only for getter*/
  
};

#endif
