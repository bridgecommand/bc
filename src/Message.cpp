#include <iostream>
#include <vector>
#include "Message.hpp"
#include "Utilities.hpp"

Message::Message(SimulationModel* aModel)
{
  mModel = aModel;
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
			      
eCmdMsg Message::Parse(const char *aData, size_t aDataSize, void** aCmdData)
{
  std::string inRawData(aData, aDataSize);
  std::vector<std::string> inData = Utilities::split(inRawData,'|');

  for(int idMessage=0; idMessage < inData.size(); idMessage++)
    {
      std::string message = inData.at(idMessage);

      if(message.length() > 2)
	{ 
	  if(message.substr(0,2).compare("MC") == 0) /*From Map Controller*/
	    { 
	      message = message.substr(2,message.length()-2);
	      std::vector<std::string> cmds = Utilities::split(message,'#');
		
	      if(cmds.size() > 0)
		{
		  for(std::vector<std::string>::iterator it=cmds.begin(); it!=cmds.end(); ++it)
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
	    }
	}
    }
  return E_CMD_MESSAGE_UNKNOWN; 
}

std::string Message::MakeLines(void)
{
  std::string msg;
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


std::string Message::KeepAlive(void)
{
  std::string msg;

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
      msg.append(Utilities::lexical_cast<std::string>(it->startTime));
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



std::string Message::KeepAliveShort(void)
{
  std::string msg;

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


std::string Message::ShutDown(void)
{
  std::string msg;
  msg.clear();
  msg = "SD"; 
  return msg;
}
