#include <iostream>
#include <vector>
#include "Message.hpp"
#include "Utilities.hpp"

sParseHeader tParseHeader[MAX_HEADER_MSG] = {{"MC", &Message::ParseMapController},
					     {"BC", &Message::ParseMasterCommand},
					     {"OS", &Message::ParseOwnShip},
					     {"SC", &Message::ParseScenario},
					     {"SD", &Message::ParseShutDown},
                         {"MH", &Message::ParseMultiPlayer}
					    };

Message::Message(SimulationModel* aModel)
{
  mModel = aModel;
}

Message::Message()
{
 
}

Message::~Message()
{    
}

sUpLeg* Message::UpdateLeg(std::string aCmd)
{
  static sUpLeg dataUpdateLeg = {0};
  std::vector<std::string> parts = Utilities::split(aCmd,','); 
  if(parts.size() == 6)
    {
      dataUpdateLeg.shipNo = Utilities::lexical_cast<int>(parts.at(1)) - 1;
      dataUpdateLeg.legNo = Utilities::lexical_cast<int>(parts.at(2)) - 1;
      dataUpdateLeg.bearing = Utilities::lexical_cast<float>(parts.at(3));
      dataUpdateLeg.speed = Utilities::lexical_cast<float>(parts.at(4));
      dataUpdateLeg.dist = Utilities::lexical_cast<float>(parts.at(5));
    }
  return &dataUpdateLeg;
}

sDelLeg* Message::DeleteLeg(std::string aCmd)
{
  static sDelLeg dataDeleteLeg = {0};
  std::vector<std::string> parts = Utilities::split(aCmd,',');
  if(parts.size() == 3)
    {
      dataDeleteLeg.shipNo = Utilities::lexical_cast<int>(parts.at(1)) - 1;
      dataDeleteLeg.legNo = Utilities::lexical_cast<int>(parts.at(2)) - 1;
    }
  return &dataDeleteLeg;
}

sRepoShip* Message::RepositionShip(std::string aCmd)
{
  static sRepoShip dataRepositionShip = {0};
  std::vector<std::string> parts = Utilities::split(aCmd,','); 
  if(parts.size() == 4)
    {
      dataRepositionShip.shipNo = Utilities::lexical_cast<int>(parts.at(1)) - 1;
      dataRepositionShip.posX = Utilities::lexical_cast<float>(parts.at(2));
      dataRepositionShip.posZ = Utilities::lexical_cast<float>(parts.at(3));
    }
  return &dataRepositionShip;
}

sResetLegs* Message::ResetLegs(std::string aCmd)
{
  static sResetLegs dataResetLegs = {0};
  std::vector<std::string> parts = Utilities::split(aCmd,',');
  if (parts.size() == 6)
    {
      dataResetLegs.shipNo = Utilities::lexical_cast<int>(parts.at(1)) - 1;
      dataResetLegs.posX = Utilities::lexical_cast<float>(parts.at(2));
      dataResetLegs.posZ = Utilities::lexical_cast<float>(parts.at(3));
      dataResetLegs.cog = Utilities::lexical_cast<float>(parts.at(4));
      dataResetLegs.sog = Utilities::lexical_cast<float>(parts.at(5));
    }
  
  return &dataResetLegs;
}

sWeather* Message::SetWeather(std::string aCmd)
{
  static sWeather dataWeather = {0};
  std::vector<std::string> parts = Utilities::split(aCmd,',');
  if (parts.size() == 4)
    { 
      dataWeather.weather = Utilities::lexical_cast<float>(parts.at(1));
      dataWeather.rain = Utilities::lexical_cast<float>(parts.at(2));
      dataWeather.visibility = Utilities::lexical_cast<float>(parts.at(3));
    }
  return &dataWeather;
}

sMob* Message::ManOverboard(std::string aCmd)
{
  static sMob dataMob = {0};
  std::vector<std::string> parts = Utilities::split(aCmd,',');
  if(parts.size()==2)
    {
      dataMob.mobMode = Utilities::lexical_cast<int>(parts.at(1));
    }
  return &dataMob;
}

sMmsi* Message::SetMMSI(std::string aCmd)
{
  static sMmsi dataMmsi = {0};
  std::vector<std::string> parts = Utilities::split(aCmd,',');
  if(parts.size()==3)
    {
      dataMmsi.shipNo = Utilities::lexical_cast<int>(parts.at(1)) - 1;
      dataMmsi.mmsi = Utilities::lexical_cast<unsigned int>(parts.at(2));
    }
  return &dataMmsi;
}

sRuddWork* Message::RudderWorking(std::string aCmd)
{
  static sRuddWork dataRudderWork = {0};
  std::vector<std::string> parts = Utilities::split(aCmd,',');
  if(parts.size()==3)
    {
      dataRudderWork.whichPump = Utilities::lexical_cast<int>(parts.at(1));
      dataRudderWork.rudderFunction = Utilities::lexical_cast<int>(parts.at(2));
    }
  return &dataRudderWork;
}

sRuddFol* Message::RudderFollowUp(std::string aCmd)
{
  static sRuddFol dataRudderFunction = {0};
  std::vector<std::string> parts = Utilities::split(aCmd,','); 
  if(parts.size()==2)
    {
      dataRudderFunction.rudderFunction = Utilities::lexical_cast<int>(parts.at(1));
    }
  return &dataRudderFunction;
}

sCtrlOv* Message::CtrlOverride(std::string aCmd)
{
  static sCtrlOv dataCtrlOverride = {0};      
  std::vector<std::string> parts = Utilities::split(aCmd,',');
  if (parts.size()==3)
    {
      dataCtrlOverride.overrideMode = Utilities::lexical_cast<unsigned int>(parts.at(1));
      dataCtrlOverride.overrideData = Utilities::lexical_cast<float>(parts.at(2));
    }
  return &dataCtrlOverride;
}

sTimeInf Message::GetTimeInfos(std::vector<std::string>& aTimeData)
{
  float timeError = 0;
  float accelAdjustment = 0;
  static float previousTimeError = 0;
  sTimeInf timeInfos = {0};

  if(aTimeData.size() > 2)
    {
      timeError = Utilities::lexical_cast<float>(aTimeData.at(2)) - mModel->getTimeDelta(); //How far we are behind the master
      float baseAccelerator = Utilities::lexical_cast<float>(aTimeData.at(3)); //The master accelerator setting

      if(fabs(timeError) > 1)
	{
      timeInfos.setTimeD = true;
	  timeInfos.timeD = Utilities::lexical_cast<float>(aTimeData.at(2));
	  accelAdjustment = 0;	  
	}
      else
	{ 
      timeInfos.setTimeD = false;
      //Adjust accelerator to maintain time alignment
	  accelAdjustment += timeError*0.01; //Integral only at the moment
	  //Check for zero crossing, and reset
	  if(previousTimeError * timeError < 0)
	    {
	      accelAdjustment = 0;
	    }
	  if(baseAccelerator + accelAdjustment < 0)
	    {
	      accelAdjustment= -1*baseAccelerator;
	    }//Saturate at zero
	}
      timeInfos.accel = baseAccelerator + accelAdjustment;
      previousTimeError = timeError; //Store for next time
    }
  return timeInfos;
}

sShipInf Message::GetInfosOwnShip(std::vector<std::string>& aOwnShipData)
{
  sShipInf shipInfos = {0};
  if(aOwnShipData.size() == 9 || aOwnShipData.size() == 5)
    {
      shipInfos.posX = Utilities::lexical_cast<float>(aOwnShipData.at(0));
      shipInfos.posZ = Utilities::lexical_cast<float>(aOwnShipData.at(1));
      shipInfos.hdg = Utilities::lexical_cast<float>(aOwnShipData.at(2));
      shipInfos.rot = Utilities::lexical_cast<float>(aOwnShipData.at(3));
      if(aOwnShipData.size() == 9)
	shipInfos.speed = Utilities::lexical_cast<float>(aOwnShipData.at(6))/MPS_TO_KTS;
      else
	shipInfos.speed = Utilities::lexical_cast<float>(aOwnShipData.at(4))/MPS_TO_KTS;
    }
  return shipInfos;
}

void Message::GetInfosOtherShips(std::vector<std::string>& aOtherShipsData, unsigned int aNumberOthers, sOthShipInf& aOthersShipsInfos)
{
  if(aNumberOthers == aOtherShipsData.size())
    {
      aOthersShipsInfos.nbrShips = aNumberOthers;
      for(unsigned short i=0; i<aNumberOthers; i++)
	{
	  std::vector<std::string> currentShip = Utilities::split(aOtherShipsData.at(i),',');
	  if(currentShip.size() == 9)
	    {
	      aOthersShipsInfos.ships[i].posX = Utilities::lexical_cast<float>(currentShip.at(0));
	      aOthersShipsInfos.ships[i].posZ = Utilities::lexical_cast<float>(currentShip.at(1));
	      aOthersShipsInfos.ships[i].hdg = Utilities::lexical_cast<float>(currentShip.at(2));
	      aOthersShipsInfos.ships[i].speed = Utilities::lexical_cast<float>(currentShip.at(3));
	      aOthersShipsInfos.ships[i].rot = Utilities::lexical_cast<float>(currentShip.at(4));
	    }
	}
    }
}

sMobInf Message::GetInfosMob(std::vector<std::string>& aMobData, unsigned int aNbrMob)
{
  sMobInf mobInfos = {0};
  if(aMobData.size()==2 && aNbrMob > 0)
    {
      mobInfos.isMob = true;
      mobInfos.posX = Utilities::lexical_cast<float>(aMobData.at(0));
      mobInfos.posZ = Utilities::lexical_cast<float>(aMobData.at(1));
    }
  return mobInfos;
}

sLinesInf Message::GetInfosLines(std::vector<std::string>& aLinesData, unsigned int aNumberLines)
{
  sLinesInf linesInfos = {0};
  if(aNumberLines > 0)
    {
      linesInfos.lineNbr = aNumberLines;     
      if(aNumberLines == aLinesData.size())
	{
	  for(unsigned short i=0; i<aNumberLines; i++)
	    {
	      std::vector<std::string> currentLineData = Utilities::split(aLinesData.at(i),',');
	      if(currentLineData.size() == 16)
		{ 	    
		  linesInfos.lineStartX = Utilities::lexical_cast<float>(currentLineData.at(0));
		  linesInfos.lineStartY = Utilities::lexical_cast<float>(currentLineData.at(1));
		  linesInfos.lineStartZ = Utilities::lexical_cast<float>(currentLineData.at(2));
		  linesInfos.lineEndX = Utilities::lexical_cast<float>(currentLineData.at(3));
		  linesInfos.lineEndY = Utilities::lexical_cast<float>(currentLineData.at(4));
		  linesInfos.lineEndZ = Utilities::lexical_cast<float>(currentLineData.at(5));
		  linesInfos.lineStartType = Utilities::lexical_cast<int>(currentLineData.at(6));
		  linesInfos.lineEndType = Utilities::lexical_cast<int>(currentLineData.at(7));
		  linesInfos.lineStartID = Utilities::lexical_cast<int>(currentLineData.at(8));
		  linesInfos.lineEndID = Utilities::lexical_cast<int>(currentLineData.at(9));
		  linesInfos.lineNominalLength = Utilities::lexical_cast<float>(currentLineData.at(10));
		  linesInfos.lineBreakingTension = Utilities::lexical_cast<float>(currentLineData.at(11));
		  linesInfos.lineBreakingStrain = Utilities::lexical_cast<float>(currentLineData.at(12));
		  linesInfos.lineNominalShipMass = Utilities::lexical_cast<float>(currentLineData.at(13));
		  linesInfos.lineKeepSlackInt = Utilities::lexical_cast<int>(currentLineData.at(14));
		  linesInfos.lineHeaveInInt = Utilities::lexical_cast<int>(currentLineData.at(15));
		}
	    }
	}
    }      
  return linesInfos;
}

sWeatherInf Message::GetInfosWeather(std::vector<std::string>& aWeatherData)
{
  sWeatherInf weatherInfos = {0};
  if(aWeatherData.size() == 5)
    {
      weatherInfos.weather = Utilities::lexical_cast<float>(aWeatherData.at(0));
      weatherInfos.visibility = Utilities::lexical_cast<float>(aWeatherData.at(1));
      weatherInfos.rain = Utilities::lexical_cast<float>(aWeatherData.at(3));
    }
  return weatherInfos;
}

sViewInf Message::GetInfosView(std::vector<std::string>& aViewData)
{
  sViewInf viewInfos = {0};
  if(aViewData.size() == 1)
    {
       if(mModel->getMoveViewWithPrimary())
       {
           viewInfos.view = Utilities::lexical_cast<float>(aViewData.at(0));
       }
    }
  return viewInfos;
}

sCtrlsInf Message::GetInfosControls(std::vector<std::string>& aCtrlsData)
{
  sCtrlsInf controlsInfos = {0}; 
  if(aCtrlsData.size() == 10)
    {
      if(!mModel->getIsSecondaryControlWheel())
	controlsInfos.wheel = Utilities::lexical_cast<float>(aCtrlsData.at(0));
    
      controlsInfos.rudder = Utilities::lexical_cast<float>(aCtrlsData.at(1));
      
      if(!mModel->getIsSecondaryControlPortEngine())
	controlsInfos.portEng = Utilities::lexical_cast<float>(aCtrlsData.at(2));

      if(!mModel->getIsSecondaryControlStbdEngine())
	controlsInfos.stbdEng = Utilities::lexical_cast<float>(aCtrlsData.at(3));
  
      if(!mModel->getIsSecondaryControlPortSchottel())
	controlsInfos.portSch = Utilities::lexical_cast<float>(aCtrlsData.at(4));
    
      if(!mModel->getIsSecondaryControlStbdSchottel())
	controlsInfos.stbdSch = Utilities::lexical_cast<float>(aCtrlsData.at(5));
    
      if(!mModel->getIsSecondaryControlPortThrustLever())
	controlsInfos.portThrust = Utilities::lexical_cast<float>(aCtrlsData.at(6));
    
      if(!mModel->getIsSecondaryControlStbdThrustLever())
	controlsInfos.stbdThrust = Utilities::lexical_cast<float>(aCtrlsData.at(7));
    
      if(!mModel->getIsSecondaryControlBowThruster())
      	controlsInfos.bowThrust = Utilities::lexical_cast<float>(aCtrlsData.at(8));
   
      if(!mModel->getIsSecondaryControlSternThruster())
	controlsInfos.sternThrust = Utilities::lexical_cast<float>(aCtrlsData.at(9));
    }
  return controlsInfos;
}

std::string& Message::ControlOverride(void)
{
  static std::string controlOverride;
  controlOverride.clear();

  if(mModel->getIsSecondaryControlWheel())
    {
      controlOverride.append("MCCO,0,");
      controlOverride.append(Utilities::lexical_cast<std::string>(mModel->getWheel()));
      controlOverride.append("|");
    }
  if(mModel->getIsSecondaryControlPortEngine())
    {
      controlOverride.append("MCCO,1,");
      controlOverride.append(Utilities::lexical_cast<std::string>(mModel->getPortEngine()));
      controlOverride.append("|");
    }
  if(mModel->getIsSecondaryControlStbdEngine())
    {
      controlOverride.append("MCCO,2,");
      controlOverride.append(Utilities::lexical_cast<std::string>(mModel->getStbdEngine()));
      controlOverride.append("|");
    }
  if(mModel->getIsSecondaryControlPortSchottel())
    {
      controlOverride.append("MCCO,3,");
      controlOverride.append(Utilities::lexical_cast<std::string>(mModel->getPortSchottel()));
      controlOverride.append("|");
    }
  if(mModel->getIsSecondaryControlStbdSchottel())
    {
      controlOverride.append("MCCO,4,");
      controlOverride.append(Utilities::lexical_cast<std::string>(mModel->getStbdSchottel()));
      controlOverride.append("|");
    }
  if(mModel->getIsSecondaryControlPortThrustLever())
    {
      controlOverride.append("MCCO,5,");
      controlOverride.append(Utilities::lexical_cast<std::string>(mModel->getPortAzimuthThrustLever()));
      controlOverride.append("|");
    }
  if(mModel->getIsSecondaryControlStbdThrustLever())
    {
      controlOverride.append("MCCO,6,");
      controlOverride.append(Utilities::lexical_cast<std::string>(mModel->getStbdAzimuthThrustLever()));
      controlOverride.append("|");
    }
  if(mModel->getIsSecondaryControlBowThruster())
    {
      controlOverride.append("MCCO,7,");
      controlOverride.append(Utilities::lexical_cast<std::string>(mModel->getBowThruster()));
      controlOverride.append("|");
    }
  if(mModel->getIsSecondaryControlSternThruster())
    {
      controlOverride.append("MCCO,8,");
      controlOverride.append(Utilities::lexical_cast<std::string>(mModel->getSternThruster()));
      controlOverride.append("|");
    }
  return controlOverride;
}

eCmdMsg Message::ParseOwnShip(std::string& aMsg, void** aCmdData)
{
  std::vector<std::string> osRec = Utilities::split(aMsg,',');
  static sShipInf ownShipInfos = {0};

  if(osRec.size() > 0)
    { 
      ownShipInfos = GetInfosOwnShip(osRec);
      *aCmdData = (void*)&ownShipInfos;
      return E_CMD_MESSAGE_OWN_SHIP;
    }
  return E_CMD_MESSAGE_UNKNOWN;
}

eCmdMsg Message::ParseScenario(std::string& aMsg, void** aCmdData)
{
  static std::string rawScenario;
  rawScenario.clear();
  rawScenario = "SC" + aMsg;
  
  if(rawScenario.size() > 4)
    {      
      *aCmdData = (void*)rawScenario.c_str();
      return E_CMD_MESSAGE_SCENARIO;
    }
  return E_CMD_MESSAGE_UNKNOWN;
}

eCmdMsg Message::ParseShutDown(std::string& aMsg, void** aCmdData)
{
  return E_CMD_MESSAGE_SHUTDOWN;
}


eCmdMsg Message::ParseMapController(std::string& aMsg, void** aCmdData)
{
  std::vector<std::string> mcRec = Utilities::split(aMsg,'#');
  
  if(mcRec.size() > 0)
    {
      for(std::vector<std::string>::iterator it=mcRec.begin(); it!=mcRec.end(); ++it)
	{
	  std::string itCmd = *it;

	  if(itCmd.length() > 2)
	    {
	      if(itCmd.substr(0,2).compare("CL") == 0 || /*Change Leg*/
		 itCmd.substr(0,2).compare("AL") == 0)   /*Add Leg*/
		{
		  *aCmdData = (void*)UpdateLeg(itCmd);
		  return E_CMD_MESSAGE_UPDATE_LEG;
		}
	      else if(itCmd.substr(0,2).compare("DL") == 0) /*Delete Leg*/
		{
		  *aCmdData = (void*)DeleteLeg(itCmd); 		      
		  return E_CMD_MESSAGE_DELETE_LEG;
		}
	      else if(itCmd.substr(0,2).compare("RS") == 0) /*Reposition Ship*/
		{			
		  *aCmdData = (void*)RepositionShip(itCmd);
		  return E_CMD_MESSAGE_REPOSITION_SHIP;
		}
	      else if(itCmd.substr(0,2).compare("RL") == 0) /*Reset Legs*/
		{
		  *aCmdData = (void*)ResetLegs(itCmd);
		  return E_CMD_MESSAGE_RESET_LEGS;
		}
	      else if(itCmd.substr(0,2).compare("SW") == 0) /*Set Weather*/
		{
		  *aCmdData = (void*)SetWeather(itCmd);
		  return E_CMD_MESSAGE_SET_WEATHER;
		}
	      else if(itCmd.substr(0,2).compare("MO") == 0) /*Man Overboard*/
		{		      
		  *aCmdData = (void*)ManOverboard(itCmd);
		  return E_CMD_MESSAGE_MAN_OVERBOARD;
		}
	      else if(itCmd.substr(0,2).compare("MM") == 0) /*Set MMSI*/
		{			
		  *aCmdData = (void*)SetMMSI(itCmd);
		  return E_CMD_MESSAGE_SET_MMSI;
		}
	      else if(itCmd.substr(0,2).compare("RW") == 0) /*Rudder Working*/
		{
		  *aCmdData = (void*)RudderWorking(itCmd);
		  return E_CMD_MESSAGE_RUDDER_WORKING;
		}
	      else if(itCmd.substr(0,2).compare("RF") == 0) /*Rudder Follow up*/
		{
		  *aCmdData = (void*)RudderFollowUp(itCmd);
		  return E_CMD_MESSAGE_RUDDER_FOLLOW_UP;
		}
	      else if(itCmd.substr(0,2).compare("CO") == 0) /*Controls Override*/
		{
		  *aCmdData = (void*)CtrlOverride(itCmd);
		  return E_CMD_MESSAGE_CONTROLS_OVERRIDE;
		}
	    } 
	}
    }
  return E_CMD_MESSAGE_UNKNOWN;
}

eCmdMsg Message::ParseMultiPlayer(std::string& aMsg, void** aCmdData)
{
    if (aMsg.substr(0, 3).compare("SCN") == 0)
        return ParseScenario(aMsg.substr(2), aCmdData);
    else
        return ParseMasterCommand(aMsg, aCmdData);
}

eCmdMsg Message::ParseMasterCommand(std::string& aMsg, void** aCmdData)
{
  static sMasterCmdsInf masterCmdsData;
  std::vector<std::string> bcRec = Utilities::split(aMsg,'#');
  
  if(MAX_RECORD_BC_MSG == bcRec.size())
    {
      /*Time Infos*/
      std::vector<std::string> timeData = Utilities::split(bcRec.at(0),',');
      masterCmdsData.time = GetTimeInfos(timeData);

      /*Own Ship Infos*/
      std::vector<std::string> positionData = Utilities::split(bcRec.at(1),',');
      masterCmdsData.ownShip = GetInfosOwnShip(positionData);
      
      std::vector<std::string> numberData = Utilities::split(bcRec.at(2),',');
      if(numberData.size() == 4)
	{
	  /*Other Ships Infos*/
	  unsigned int numberOthers = Utilities::lexical_cast<unsigned int>(numberData.at(0));
      
	  if(numberOthers > 0)
	    {
	      std::vector<std::string> otherShipsData = Utilities::split(bcRec.at(3),'|');
	      masterCmdsData.otherShips.ships = new sShipInf[numberOthers];
	      GetInfosOtherShips(otherShipsData, numberOthers, masterCmdsData.otherShips);
	    }
	  
	  /*Buoys*/
	  //Not recovered

	  /*MOB*/
	  unsigned int numberMOB = Utilities::lexical_cast<unsigned int>(numberData.at(2));
	  if(numberMOB)
	    {
	      std::vector<std::string> mobData = Utilities::split(numberData.at(5),',');
	      masterCmdsData.mob = GetInfosMob(mobData, numberMOB);
	    }	  
	  
	  /*Lines*/
	  unsigned int numberLines = Utilities::lexical_cast<unsigned int>(numberData.at(3));
	  std::vector<std::string> linesData = Utilities::split(bcRec.at(11),'|');
	  masterCmdsData.lines = GetInfosLines(linesData, numberLines);
	}

      /*Weather*/
      std::vector<std::string> weatherData = Utilities::split(bcRec.at(7),',');
      masterCmdsData.weather = GetInfosWeather(weatherData);

      /*Views*/
      std::vector<std::string> viewData = Utilities::split(bcRec.at(9),',');
      masterCmdsData.view = GetInfosView(viewData);

      /*Controls*/
      std::vector<std::string> controlsData = Utilities::split(bcRec.at(12),',');
      masterCmdsData.controls = GetInfosControls(controlsData);

      *aCmdData = (void*)&masterCmdsData;
      
      return E_CMD_MESSAGE_BRIDGE_COMMAND;
    }
  return E_CMD_MESSAGE_UNKNOWN;
}

eCmdMsg Message::Parse(const char *aData, size_t aDataSize, void** aCmdData)
{
  std::string inRawData(aData, aDataSize);
  unsigned int nbrMsg = 1;
  std::string message = inRawData;
  unsigned int idMessage = 0;

  /*Map Controller message*/
  if(inRawData.substr(0,2).compare(tParseHeader[0].header)==0)
    {
      std::vector<std::string> inData = Utilities::split(inRawData,'|');
      nbrMsg = inData.size();
      message = inData.at(idMessage);
    }
 
  for(idMessage=0; idMessage < nbrMsg; idMessage++)
    {
      if(message.length() >= 2)
	{
	  for(unsigned char i = 0;i<MAX_HEADER_MSG;i++)
	    {
	      if(message.substr(0,2).compare(tParseHeader[i].header)==0) 
		{ 
		  message = message.substr(2,message.length()-2);
		  return (this->*tParseHeader[i].pFuncParse)(message, aCmdData);
		}
	    }
	}
    }
  return E_CMD_MESSAGE_UNKNOWN; 
}

std::string& Message::MpFeedBack(void)
{
  static std::string mpFeedBack = "MPF";
  mpFeedBack.clear();
  mpFeedBack.append(Utilities::lexical_cast<std::string>(mModel->getPosX()));
  mpFeedBack.append("#");
  mpFeedBack.append(Utilities::lexical_cast<std::string>(mModel->getPosZ()));
  mpFeedBack.append("#");
  mpFeedBack.append(Utilities::lexical_cast<std::string>(mModel->getHeading()));
  mpFeedBack.append("#");
  mpFeedBack.append(Utilities::lexical_cast<std::string>(mModel->getRateOfTurn()*irr::core::RADTODEG));
  mpFeedBack.append("#");
  mpFeedBack.append(Utilities::lexical_cast<std::string>(mModel->getSpeed()));
  mpFeedBack.append("#");
  mpFeedBack.append(Utilities::lexical_cast<std::string>(mModel->getTimeDelta()));
  mpFeedBack.append("#");

  mpFeedBack.append(MakeLines());

  return mpFeedBack;
}

std::string& Message::MakeLines(void)
{
  static std::string msg;
  msg.clear();
    
  for(int number = 0; number < (int)(mModel->getLines()->getNumberOfLines()); number++ )
    {
      msg.append(Utilities::lexical_cast<std::string>(mModel->getLines()->getLineStartX(number)));
      msg.append(",");
      msg.append(Utilities::lexical_cast<std::string>(mModel->getLines()->getLineStartY(number)));
      msg.append(",");
      msg.append(Utilities::lexical_cast<std::string>(mModel->getLines()->getLineStartZ(number)));
      msg.append(",");
      msg.append(Utilities::lexical_cast<std::string>(mModel->getLines()->getLineEndX(number)));
      msg.append(",");
      msg.append(Utilities::lexical_cast<std::string>(mModel->getLines()->getLineEndY(number)));
      msg.append(",");
      msg.append(Utilities::lexical_cast<std::string>(mModel->getLines()->getLineEndZ(number)));
      msg.append(",");
      msg.append(Utilities::lexical_cast<std::string>(mModel->getLines()->getLineStartType(number)));
      msg.append(",");
      msg.append(Utilities::lexical_cast<std::string>(mModel->getLines()->getLineEndType(number)));
      msg.append(",");
      msg.append(Utilities::lexical_cast<std::string>(mModel->getLines()->getLineStartID(number)));
      msg.append(",");
      msg.append(Utilities::lexical_cast<std::string>(mModel->getLines()->getLineEndID(number)));
      msg.append(",");
      msg.append(Utilities::lexical_cast<std::string>(mModel->getLines()->getLineNominalLength(number)));
      msg.append(",");
      msg.append(Utilities::lexical_cast<std::string>(mModel->getLines()->getLineBreakingTension(number)));
      msg.append(",");
      msg.append(Utilities::lexical_cast<std::string>(mModel->getLines()->getLineBreakingStrain(number)));
      msg.append(",");
      msg.append(Utilities::lexical_cast<std::string>(mModel->getLines()->getLineNominalShipMass(number)));
      msg.append(",");
      if (mModel->getLines()->getKeepSlack(number)) {
	msg.append("1");
      } else {
	msg.append("0");
      }
      msg.append(",");
      if (mModel->getLines()->getHeaveIn(number)) {
	msg.append("1");
      } else {
	msg.append("0");
      }
        
      if (number < (int)mModel->getLines()->getNumberOfLines()-1) {msg.append("|");}
    }
  return msg;
}


std::string& Message::KeepAlive(void)
{
  static std::string msg;

  msg.clear();
  msg = "BC";
  //0 Time:
  msg.append(Utilities::lexical_cast<std::string>(mModel->getTimestamp())); //Current timestamp
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getTimeOffset())); //Timestamp of start of first day of scenario
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getTimeDelta())); //Time from start day of scenario
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getAccelerator())); //Current accelerator
  msg.append("#");

  //1 Position, speed etc
  msg.append(Utilities::lexical_cast<std::string>(mModel->getPosX()));
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getPosZ()));
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getHeading()));
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getRateOfTurn()));
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(0)); //Fixme: Pitch
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(0)); //Fixme: Roll
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getSOG()*MPS_TO_KTS));
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getCOG()));
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getRudder()));
  msg.append(":");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getWheel()));
  msg.append(":");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getPortEngineRPM()));
  msg.append(":");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getStbdEngineRPM()));
  msg.append("#");

  //2 Numbers: Number Other, Number buoys, Number MOB #
  msg.append(Utilities::lexical_cast<std::string>(mModel->getNumberOfOtherShips()));
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getNumberOfBuoys()));
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getManOverboardVisible()? 1 : 0));
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getLines()->getNumberOfLines()));
  msg.append("#");

  //3 Each 'Other' (Pos X (abs), Pos Z, angle, rate of turn, SART, MMSI |) #
  for(int number = 0; number < (int)mModel->getNumberOfOtherShips(); number++ ) {
    msg.append(Utilities::lexical_cast<std::string>(mModel->getOtherShipPosX(number)));
    msg.append(",");
    msg.append(Utilities::lexical_cast<std::string>(mModel->getOtherShipPosZ(number)));
    msg.append(",");
    msg.append(Utilities::lexical_cast<std::string>(mModel->getOtherShipHeading(number)));
    msg.append(",");
    msg.append(Utilities::lexical_cast<std::string>(mModel->getOtherShipSpeed(number)*MPS_TO_KTS));
    msg.append(",");
    msg.append("0"); // Rate of turn: This is not currently used in normal mode
    msg.append(",");
    msg.append("0"); //Fixme: Sart enabled
    msg.append(",");
    msg.append(Utilities::lexical_cast<std::string>(mModel->getOtherShipMMSI(number)));
    msg.append(",");

    //std::cout << "MMSI for other ship " << number << ":" << mModel->getOtherShipMMSI(number) << std::endl;

    //Send leg information
    std::vector<Leg> legs = mModel->getOtherShipLegs(number);
    msg.append(Utilities::lexical_cast<std::string>(legs.size())); //Number of legs
    msg.append(",");
    //Build leg information, each leg separated by a '/', each value by ':'
    for(std::vector<Leg>::iterator it = legs.begin(); it != legs.end(); ++it) {
      msg.append(Utilities::lexical_cast<std::string>(it->bearing));
      msg.append(":");
      msg.append(Utilities::lexical_cast<std::string>(it->speed));
      msg.append(":");
      msg.append(Utilities::lexical_cast<std::string>(it->distance));
      if (it!= (legs.end()-1)) {msg.append("/");}
    }

    if (number < (int)mModel->getNumberOfOtherShips()-1) {msg.append("|");}
  }
  msg.append("#");

  //4 Each Buoy
  for(int number = 0; number < (int)mModel->getNumberOfBuoys(); number++ ) {
    msg.append(Utilities::lexical_cast<std::string>(mModel->getBuoyPosX(number)));
    msg.append(",");
    msg.append(Utilities::lexical_cast<std::string>(mModel->getBuoyPosZ(number)));
    if (number < (int)mModel->getNumberOfBuoys()-1) {msg.append("|");}
  }
  msg.append("#");

  //5 MOB
  msg.append(Utilities::lexical_cast<std::string>(mModel->getManOverboardPosX()));
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getManOverboardPosZ()));
  msg.append("#");

  //6 Loop
  msg.append(Utilities::lexical_cast<std::string>(mModel->getLoopNumber()));
  msg.append("#");

  //7 Weather: Weather, Fog range, wind dirn, rain, light level #
  msg.append(Utilities::lexical_cast<std::string>(mModel->getWeather()));
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getVisibility()));
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(0)); //Fixme: Wind dirn
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getRain()));
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(0)); //Fixme: Light level
  msg.append("#");

  //8 EBL Brg, height, show (or 0,0,0) #
  msg.append("0,0,0#"); //Fixme: Mob details

  //9 View number
  msg.append(Utilities::lexical_cast<std::string>(mModel->getCameraView()));
  msg.append("#");

  //10 Multiplayer request here (Not used)
  msg.append("0");
  msg.append("#");

  //11 Lines (mooring/towing)
  msg.append(MakeLines());
  msg.append("#");
    
  //12 Controls state (wheel, rudder, port/stbd engine, port/stbd schottel, port/stbd thrust lever, bow/stern thruster)
  msg.append(Utilities::lexical_cast<std::string>(mModel->getWheel()));
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getRudder()));
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getPortEngine()));
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getStbdEngine()));
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getPortSchottel()));
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getStbdSchottel()));
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getPortAzimuthThrustLever()));
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getStbdAzimuthThrustLever()));
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getBowThruster()));
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getSternThruster()));

  return msg;
}



std::string& Message::KeepAliveShort(void)
{
  static std::string msg;

  msg.clear();
  msg = "OS"; //Own ship only
  //1 Position, speed etc
  msg.append(Utilities::lexical_cast<std::string>(mModel->getPosX()));
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getPosZ()));
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getHeading()));
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getRateOfTurn()));
  msg.append(",");
  msg.append(Utilities::lexical_cast<std::string>(mModel->getSOG()*MPS_TO_KTS));

  return msg;
}


std::string& Message::ShutDown(void)
{
  static std::string msg;
  msg.clear();
  msg = "SD"; 
  return msg;
}
