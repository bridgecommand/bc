/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2014 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY Or FITNESS For A PARTICULAR PURPOSE.  See the
     GNU General Public License For more details.

     You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#include <cmath>
#include <fstream>

#include "SimulationModel.hpp"
#include "ModelParams.hpp"
#include "ScenarioDataStructure.hpp"
#include "GUIMain.hpp"
#include "Terrain.hpp"
#include "Sky.hpp"
#include "Buoys.hpp"
#include "Sound.hpp"
#include "Collision.hpp"
#include "IniFile.hpp"
#include "Constants.hpp"
#include "Utilities.hpp"
#include "MessageMisc.hpp"


SimulationModel::SimulationModel()
{

}

SimulationModel::SimulationModel(irr::IrrlichtDevice* aDev, GUIMain* aGui, Sound* aSound, ScenarioData aScenarioData, ModelParameters aModelParameters)
{
  mTerrain = new Terrain();
  mWater = new Water();
  mTide = new Tide();
  mBuoys = new Buoys();
  mOtherShips = new OtherShips(); 
  mOwnShip = new OwnShip();
  mCamera = new Camera();
  mRadarCalculation = new RadarCalculation();
  mRadarScreen = new RadarScreen();
  mRadarCamera = new Camera();
  mLines = new Lines();
  mRain = new Rain();
  mManOverboard = new ManOverboard();
  mGuiData = new GUIData();
  mLandObjects = new LandObjects();
  mLandLights = new LandLights();
  mLight = new Light();
  mCollision = new Collision();
  mSolver = new Solver();
  mWind = new Wind();
  
  mDevice = aDev;
  mSmgr = mDevice->getSceneManager();;
  mDriver = mDevice->getVideoDriver();
  mGuiMain = aGui;
  mSound = aSound;
  mMoveViewWithPrimary = true;

  //Store a serialised form of the scenario loaded, as we may want to send this over the network
  mSerialisedScenarioData = aScenarioData.serialise(false);
  mScenarioName = aScenarioData.scenarioName;

  // Store model parameters
  mModelParameters = aModelParameters;

  //Set loop number to zero
  mTime.loopNumber = 0;

  mWorldName = aScenarioData.worldName;
  float startTime = aScenarioData.startTime;
  irr::u32 startDay = aScenarioData.startDay;
  irr::u32 startMonth = aScenarioData.startMonth;
  irr::u32 startYear = aScenarioData.startYear;

  //load the sun times
  float sunRise = aScenarioData.sunRise;
  float sunSet  = aScenarioData.sunSet;
  if(sunRise==0) sunRise=6;
  if(sunSet==0) sunSet=18;

  //load the weather:
  //Fixme: add in wind direction etc
  mWeather = aScenarioData.weather;
  mRainIntensity = aScenarioData.rainIntensity;
  mVisibilityRange = aScenarioData.visibilityRange;
  if(mVisibilityRange <= 0) mVisibilityRange = 5*M_IN_NM; //TODO: Check units

  mWind->setTrueDirection(aScenarioData.windDirection);
  mWind->setTrueSpeed(aScenarioData.windSpeed);

  //std::cout << "Wind direction: " << windDirection << " Wind speed: " << windSpeed << std::endl;

  //Fixme: Think about time zone handling
  //Fixme: Note that if the time_t isn't long enough, 2038 problem exists
  mTime.scenarioOffsetTime = Utilities::dmyToTimestamp(startDay,startMonth,startYear);//Time in seconds to start of scenario day (unix timestamp for 0000h on day scenario starts)

  //set internal scenario time to start
  mTime.scenarioTime = startTime * SECONDS_IN_HOUR;

  //Set initial tide height to zero
  mTideHeight = 0;

  if (mWorldName == "") {
    //Could not load world name from scenario, so end here
    std::cerr << "World model name not defined" << std::endl;
    exit(EXIT_FAILURE);
  }

  //construct path to world model
  std::string worldPath = "world/";
  worldPath.append(mWorldName);

  //Check if this world model exists in the user dir.
  std::string userFolder = Utilities::getUserDir();
  if (Utilities::pathExists(userFolder + worldPath)) {
    worldPath = userFolder + worldPath;
  }

  // Store world model readme.txt file contents here if available
  std::string worldReadmePath = worldPath + "/readme.txt";
  mWorldModelReadmeText = "";
  if (Utilities::pathExists(worldReadmePath)) {
    std::ifstream file(worldReadmePath.c_str());
    if (file.is_open()) {
      std::string line;
      while (std::getline(file, line)) {
	mWorldModelReadmeText.append(line);
	mWorldModelReadmeText.append("\n");
      }
    }    
  }
        
  //Add terrain: Needs to happen first, so the terrain parameters are available
  mTerrain->load(worldPath, mSmgr, mDevice, mModelParameters.limitTerrainResolution);

  //sky box/dome
  Sky sky(mSmgr);

  //Load own ship model.
  // TODO: It would be better to pass in modelParameters directly
  mOwnShip->Load(aScenarioData.ownShipData, mWater, mTide, mTerrain, mDevice);

  if(mModelParameters.mode == OperatingMode::Secondary) {
    mOwnShip->setSpeed(0); //Don't start moving if in secondary mode
  }

  mSolver->Init(mOwnShip);
  
  //Load rain
  mRain->load(mSmgr, mCamera->getSceneNode(), mDevice, mOwnShip->getPosition().X, mOwnShip->getPosition().Y, mOwnShip->getPosition().Z, mOwnShip->getLength(), mOwnShip->getBreadth());

  //add water
  bool waterReflection = true;
  if (mModelParameters.vrMode) waterReflection = false;
  
  mWater->load(mSmgr,mOwnShip->getSceneNode(),mWeather,mModelParameters.disableShaders,waterReflection,mModelParameters.waterSegments);

  if (1 == mOwnShip->getNumberProp()) aGui->setSingleEngine();

  //Load the radar with config parameters
  mRadarCalculation->load(mOwnShip->getRadarConfigFile(),mDevice);

  //set camera zoom to 1
  mCurrentZoom = 1.0;
  mZoomLevel = 7.0; //Default zoom of 7x

  //make a camera, setting parent and offset
  std::vector<irr::core::vector3df> views = mOwnShip->getCameraViews(); //Get the initial camera offset from the own ship model
  std::vector<bool> isHighView = mOwnShip->getCameraIsHighView(); //Are these special 'looking down' views
  float angleCorrection = mOwnShip->getAngleCorrection();
  mCamera->load(mSmgr,mDevice->getLogger(),mOwnShip->getSceneNode(),views, isHighView,irr::core::degToRad(mModelParameters.viewAngle),mModelParameters.lookAngle,angleCorrection);
  mCamera->setNearValue(mModelParameters.cameraMinDistance);
  mCamera->setFarValue(mModelParameters.cameraMaxDistance);

  //make ambient light
  mLight->load(mSmgr,sunRise,sunSet, mCamera->getSceneNode());

  //Load other ships
  mOtherShips->load(aScenarioData.otherShipsData,mTime.scenarioTime,mTerrain,mWater,mModelParameters.mode,mDevice);

  //Load buoys
  mBuoys->load(worldPath, mTerrain, mWater, mDevice);

  //Load land objects
  mLandObjects->load(worldPath, mTerrain, mDevice);

  //Load land lights
  mLandLights->load(worldPath, mTerrain, mDevice);

  //Load tidal information
  mTide->load(worldPath, aScenarioData);

  //Wind
  mWind->load(mOwnShip);

  //MoB
  mManOverboard->load(irr::core::vector3df(0,0,0),mTerrain, mWater, mWind, mTide, mDevice);
  
  //Load Collsion detection
  mCollision->load(mOwnShip->getSceneNode(), mTerrain, mOtherShips, mOwnShip, mBuoys, mModelParameters, mDevice);
  
  //make a radar screen, setting parent and offset from own ship
  mRadarScreen->load(mSmgr,mOwnShip->getSceneNode(), mOwnShip->getRadarPosition(), mOwnShip->getRadarSize(), mOwnShip->getRadarTilt());

  //make radar camera
  std::vector<irr::core::vector3df> radarViews; //Get the initial camera offset from the radar screen
  std::vector<bool> radarViewsLookDown; //Not needed for the radar camera, but needed for compatability
  float radarTilt = mOwnShip->getRadarTilt();
  
  radarViews.push_back(mOwnShip->getRadarPosition() + irr::core::vector3df(0,0.5*sin(irr::core::DEGTORAD*radarTilt)*mOwnShip->getRadarSize(),-0.5*cos(irr::core::DEGTORAD*radarTilt)*mOwnShip->getRadarSize()));
  radarViewsLookDown.push_back(false);

  mRadarCamera->load(mSmgr, mDevice->getLogger(),mOwnShip->getSceneNode(),radarViews,radarViewsLookDown,irr::core::PI/2.0,0,0);
  mRadarCamera->setLookUp(-1.0 * radarTilt); //FIXME: Why doesn't simply -1.0*screenTilt work?
  mRadarCamera->updateViewport(1.0);
  mRadarCamera->setNearValue(0.8*0.5*mOwnShip->getRadarSize());
  mRadarCamera->setFarValue(1.2*0.5*mOwnShip->getRadarSize());

  
  //Hide the man overboard model
  mManOverboard->setVisible(false);

  //store time
  mTime.previousTime = mDevice->getTimer()->getTime();

  // Initialise as paused to start with
  mGuiData->paused = true;

} //end of SimulationModel constructor

SimulationModel::~SimulationModel()
{
  delete mGuiData;
  delete mTerrain;
  delete mWater;
  delete mTide;
  delete mBuoys;
  delete mOtherShips; 
  delete mOwnShip;
  delete mCamera;
  delete mRadarCalculation;
  delete mRadarScreen;
  delete mRadarCamera;
  delete mLines;
  delete mRain;
  delete mManOverboard;
  delete mLandObjects;
  delete mLandLights;
  delete mLight;
  delete mCollision;
  delete mWind;
}

OwnShip* SimulationModel::getOwnShip(void)
{
  if(NULL != mOwnShip) return mOwnShip;
  else return NULL;
}

Terrain* SimulationModel::getTerrain(void)
{
  if(NULL != mTerrain) return mTerrain;
  else return NULL;
}

Water* SimulationModel::getWater(void)
{
  if(NULL != mWater) return mWater;
  else return NULL;
}

Tide* SimulationModel::getTide(void)
{
  if(NULL != mTide) return mTide;
  else return NULL;
}

Buoys* SimulationModel::getBuoys(void)
{
  if(NULL != mBuoys) return mBuoys;
  else return NULL;
}

OtherShips* SimulationModel::getOtherShips(void)
{
  if(NULL != mOtherShips) return mOtherShips;
  else return NULL;
}

Camera* SimulationModel::getCamera(void)
{
  if(NULL != mCamera) return mCamera;
  else return NULL;
}

RadarCalculation* SimulationModel::getRadarCalculation(void)
{
  if(NULL != mRadarCalculation) return mRadarCalculation;
  else return NULL;
}

RadarScreen* SimulationModel::getRadarScreen(void)
{
  if(NULL != mRadarScreen) return mRadarScreen;
  else return NULL;
}

Camera* SimulationModel::getRadarCamera(void)
{
  if(NULL != mRadarCamera) return mRadarCamera;
  else return NULL;
}

Lines* SimulationModel::getLines(void)
{
  if(NULL != mLines) return mLines;
  else return NULL;
}

Rain* SimulationModel::getRain(void)
{
  if(NULL != mRain) return mRain;
  else return NULL;
}

Sound* SimulationModel::getSound(void)
{
  if(NULL != mSound) return mSound;
  else return NULL;
}

ManOverboard* SimulationModel::getMoB(void)
{
  if(NULL != mManOverboard) return mManOverboard;
  else return NULL;
}

Collision* SimulationModel::getCollision(void)
{
  if(NULL != mCollision) return mCollision;
  else return NULL;
}

Wind* SimulationModel::getWind(void)
{
  if(NULL != mWind) return mWind;
  else return NULL;
}

/*Weather, Wind, Rain, Visibility*/
void SimulationModel::setWeather(float aWeather){mWeather = aWeather;}
float SimulationModel::getWeather() const {return mWeather;}
void SimulationModel::setRain(float aRainIntensity){mRainIntensity = aRainIntensity;}
float SimulationModel::getRain() const {return mRainIntensity;}
void SimulationModel::setVisibility(float aVisibilityNm){mVisibilityRange = aVisibilityNm;}
float SimulationModel::getVisibility() const{return mVisibilityRange;}

/*Time*/
sTime& SimulationModel::getTime() {return mTime;}
unsigned int SimulationModel::getCurrentTime() const {return mTime.currentTime;}
unsigned long long SimulationModel::getTimestamp() const {return mTime.absoluteTime;}
unsigned long long SimulationModel::getTimeOffset() const {return mTime.scenarioOffsetTime;} //The timestamp at the start of the first day of the scenario
void SimulationModel::setTimeDelta(float aScenarioTime){mTime.scenarioTime = aScenarioTime;}
float SimulationModel::getTimeDelta() const {return mTime.scenarioTime;} //The change in time (s) since the start of the start day of the scenario
void SimulationModel::setAccelerator(float aAccelerator){mDevice->getTimer()->setSpeed(aAccelerator);}
float SimulationModel::getAccelerator() const {return mDevice->getTimer()->getSpeed();}
irr::u32 SimulationModel::getLoopNumber() const {return mTime.loopNumber;}

/*Models Params*/
ModelParameters& SimulationModel::getModelParameters(void){return mModelParameters;}

/*Scenario*/
std::string SimulationModel::getSerialisedScenario() const {return mSerialisedScenarioData;}
std::string SimulationModel::getScenarioName() const {return mScenarioName;}
std::string SimulationModel::getWorldName() const {return mWorldName;}
std::string SimulationModel::getWorldReadme() const{return mWorldModelReadmeText;}

/*Views*/
bool SimulationModel::getMoveViewWithPrimary() const {return mMoveViewWithPrimary;}
void SimulationModel::setMoveViewWithPrimary(bool aMoveView) {mMoveViewWithPrimary = aMoveView;}

void SimulationModel::setZoom(bool aZoomOn)
{
  if(aZoomOn) mCurrentZoom = mZoomLevel;
  else mCurrentZoom = 1;
  
  mCamera->setHFOV(irr::core::degToRad(mModelParameters.viewAngle) / mCurrentZoom);
}
    
void SimulationModel::setZoom(bool aZoomOn, float aZoomLevel)
{
  mZoomLevel = aZoomLevel;
  setZoom(aZoomOn);
}

void SimulationModel::setViewAngle(float aViewAngle)
{
  mModelParameters.viewAngle = aViewAngle;
  mCamera->setHFOV(irr::core::degToRad(mModelParameters.viewAngle) / mCurrentZoom);
}

void SimulationModel::updateCameraVRPos(irr::core::quaternion quat, irr::core::vector3df pos, irr::core::vector2df lensShift)
{
  sTime tFake = {0};
  mCamera->update(tFake, quat, pos, lensShift, true);
}

void SimulationModel::update()
{
  bool paused;
  bool collided;

  //Time management
  mTime.currentTime = mDevice->getTimer()->getTime();
  mTime.deltaTime = (mTime.currentTime - mTime.previousTime)/1000.f;
  mTime.previousTime = mTime.currentTime;
  mTime.scenarioTime += mTime.deltaTime;
  mTime.absoluteTime = Utilities::round(mTime.scenarioTime) + mTime.scenarioOffsetTime;
  mTime.loopNumber++;

  //Ensure we have the right radar screen resolution
  mRadarCalculation->setRadarDisplayRadius(mGuiMain->getRadarPixelRadius());
  mRadarScreen->setRadarDisplayRadius(mGuiMain->getRadarPixelRadius());

  //Update tide height and tidal stream here.
  mTide->update(mTime);
  mTideHeight = mTide->getTideHeight();

  //update ambient lighting
  mLight->update(mTime);
  
  //Note that linear fog is hardcoded into the water shader, so should be changed there if we use other fog types
  mDriver->setFog(mLight->getLightSColor(), irr::video::EFT_FOG_LINEAR , 0.01*mVisibilityRange*M_IN_NM, mVisibilityRange*M_IN_NM, 0.00003f /*exp fog parameter*/, true, true);

  //update rain
  //rain.setIntensity(rainIntensity);
  mRain->update(mOwnShip->getPosition().X, mOwnShip->getPosition().Y, mOwnShip->getPosition().Z, mRainIntensity);

  //update other ship positions etc
  mOtherShips->update(mTime, mTideHeight,mLight->getLightLevel(),mOwnShip->getPosition(),mOwnShip->getLength()); //Update other ship motion (based on leg information), and light visibility.

  //Wind update
  mWind->update();
  
  //update buoys (for lights, floating, and if collision detection is turned on)
  mBuoys->update(mTime, mTideHeight,mLight->getLightLevel(),mOwnShip->getPosition(),mOwnShip->getLength());

  //Update land lights
  mLandLights->update(mTime, mLight->getLightLevel());

  //update all lines, ready to be used for own ship force
  mLines->update(mTime);

  //Solver Man 3Ddl
  mSolver->SolveRk4(mTime, mOwnShip->getEta(), mOwnShip->getMu());

  //update own ship
  mOwnShip->Update(mTime, mTideHeight, mWeather, mWind, mSolver);

  if (mOwnShip->getNumberProp() > 1)
    mSound->setVolumeEngine(fabs(mOwnShip->getPortEngine())*0.5);
  else 
    mSound->setVolumeEngine((fabs(mOwnShip->getPortEngine()) + fabs(mOwnShip->getStbdEngine()))*0.5);

  //update man overboard
  mManOverboard->update(mTime, mTideHeight);

  //Check for collisions
  collided = mCollision->getBuoyCollision() || mCollision->getOtherShipCollision();

  //update water position
  mWater->update(mTideHeight,mCamera->getPosition(),mLight->getLightLevel(), mWeather);

  //update the camera position
  mCamera->update(mTime);


  // Check depth and update collision response forces and torque
  irr::f32 groundingAxialDrag = 0;
  irr::f32 groundingLateralDrag = 0;
  irr::f32 groundingTurnDrag = 0;
  mCollision->DetectAndRespond(groundingAxialDrag, groundingLateralDrag, groundingTurnDrag); // The drag values will get modified by this call

  // Add in response from mooring lines here
  //groundingAxialDrag -= linesForce.Z;
  //groundingLateralDrag -= linesForce.X;
  //groundingTurnDrag -= linesTorque.Y;

  
  //update radar
  if(mRadarCalculation->isRadarOn())
    {
      irr::video::IImage *imChosen = mRadarCalculation->update(mRadarScreen, mTerrain, mOwnShip, mBuoys, mOtherShips, mWeather, mRainIntensity, mTideHeight, mTime.deltaTime, mTime.absoluteTime, mGuiMain);
      mRadarScreen->update(imChosen);
      sTime tFake = {0};
      mRadarCamera->update(tFake);
    }
  else 
    mRadarScreen->getSceneNode()->setVisible(false);
  
  //check if paused
  paused = mDevice->getTimer()->getSpeed()==0.0;

 
  //get radar ARPA data to show
  mGuiData->arpaContactStates.clear();
  for(unsigned int i = 0; i<mRadarCalculation->getARPATracksSize(); i++)
    {
      mGuiData->arpaContactStates.push_back(mRadarCalculation->getARPAContactFromTrackIndex(i).estimate);
    }
  mGuiData->arpaListSelection = mRadarCalculation->getArpaListSelection();
 
  //Collate data to show in gui
  mGuiData->lat = mTerrain->zToLat(mOwnShip->getPosition().Z);
  mGuiData->longitude = mTerrain->xToLong(mOwnShip->getPosition().X);
  mGuiData->hdg = mOwnShip->getHeading();
  mGuiData->viewAngle = atan2(mCamera->getForwardVector().X, mCamera->getForwardVector().Z) * irr::core::RADTODEG;
  mGuiData->viewElevationAngle = asin(mCamera->getForwardVector().Y) * irr::core::RADTODEG;
  mGuiData->spd = mOwnShip->getSpeedThroughWater();
  mGuiData->portEng = mOwnShip->getPortEngine();
  mGuiData->stbdEng = mOwnShip->getStbdEngine();
  mGuiData->rudder = mOwnShip->getRudder().getDelta()*180/PI;  // inner workings of this will be modified in model DEE
  mGuiData->wheel = mOwnShip->getWheel();    // inner workings of this will be modified in model DEE
  mGuiData->depth = mOwnShip->getDepth(getTerrain());
  mGuiData->weather = mWeather;
  mGuiData->rain = mRainIntensity;
  mGuiData->visibility = mVisibilityRange;
  mGuiData->windDirection = mWind->getTrueDirection();
  mGuiData->windSpeed = mWind->getTrueSpeed();
  mGuiData->streamDirection = mTide->getStreamOverrideDirection();
  mGuiData->streamSpeed = mTide->getStreamOverrideSpeed();
  mGuiData->streamOverride = mTide->getStreamOverride();
  mGuiData->radarRangeNm = mRadarCalculation->getRangeNm();
  mGuiData->radarGain = mRadarCalculation->getGain();
  mGuiData->radarClutter = mRadarCalculation->getClutter();
  mGuiData->radarRain = mRadarCalculation->getRainClutter();
  mGuiData->guiRadarEBLBrg = mRadarCalculation->getEBLBrg();
  mGuiData->guiRadarEBLRangeNm = mRadarCalculation->getEBLRangeNm();
  mGuiData->guiRadarCursorBrg = mRadarCalculation->getCursorBrg();
  mGuiData->guiRadarCursorRangeNm = mRadarCalculation->getCursorRangeNm();
  mGuiData->currentTime = Utilities::timestampToString(mTime.absoluteTime);
  mGuiData->paused = paused;
  mGuiData->collided = collided;
  mGuiData->headUp = mRadarCalculation->getHeadUp();
  mGuiData->radarOn = mRadarCalculation->isRadarOn();
  //mGuiData->pump1On = mOwnShip->getRudderPumpState(1);
  //mGuiData->pump2On = mOwnShip->getRudderPumpState(2);
  mGuiData->tideHeight = mTideHeight;
  mGuiData->RateOfTurn = mOwnShip->getRateOfTurn();

  mGuiMain->updateGuiData(mGuiData); //Set GUI heading in degrees and speed (in m/s)
  
}

void SimulationModel::updateFromNetwork(eCmdMsg aMsgType, void* aDataCmd)
{
  switch(aMsgType)
    {
    case E_CMD_MESSAGE_UPDATE_LEG:
      {
	sUpLeg *dataUpdateLeg = (sUpLeg*)aDataCmd;
	mOtherShips->changeLeg(dataUpdateLeg->shipNo, dataUpdateLeg->legNo, dataUpdateLeg->bearing, dataUpdateLeg->speed, dataUpdateLeg->dist, mTime.scenarioTime);
	break;
      }
    case E_CMD_MESSAGE_DELETE_LEG:
      {
	sDelLeg *dataDeleteLeg = (sDelLeg*)aDataCmd;
	mOtherShips->deleteLeg(dataDeleteLeg->shipNo, dataDeleteLeg->legNo, mTime.scenarioTime);
	break;
      }
    case E_CMD_MESSAGE_REPOSITION_SHIP:
      {
	sRepoShip *dataRepoShip = (sRepoShip*)aDataCmd;	
	if(dataRepoShip->shipNo < 0)
	  mOwnShip->setPosition(dataRepoShip->posX, dataRepoShip->posZ);
	else
	  mOtherShips->setPos(dataRepoShip->shipNo, dataRepoShip->posX, dataRepoShip->posZ);

	break;
      }
    case E_CMD_MESSAGE_RESET_LEGS:
      {
        sResetLegs *dataResetLegs = (sResetLegs*)aDataCmd;	

	if(dataResetLegs->shipNo >= 0)
	  {
	    mOtherShips->setPos(dataResetLegs->shipNo, dataResetLegs->posX, dataResetLegs->posZ);
	    mOtherShips->resetLegs(dataResetLegs->shipNo, dataResetLegs->cog, dataResetLegs->sog, 1, mTime.scenarioTime);
	    //1Nm Hard-coded
	  }
	
	break;
      }
    case E_CMD_MESSAGE_SET_WEATHER:
      {
	sWeather *dataWeather = (sWeather*)aDataCmd;

        if(dataWeather->weather >= 0) {setWeather(dataWeather->weather);}
	if(dataWeather->rain >= 0) {setRain(dataWeather->rain);}
	if(dataWeather->visibility > 0) {setVisibility(dataWeather->visibility);}
	if(dataWeather->windSpeed > 0) {mWind->setTrueSpeed(dataWeather->windSpeed);}
	if(dataWeather->windDirection > 0) {mWind->setTrueDirection(dataWeather->windDirection);}
	if(dataWeather->streamDirection > 0) {mTide->setStreamOverrideDirection(dataWeather->streamDirection);}
	if(dataWeather->streamSpeed > 0) {mTide->setStreamOverrideSpeed(dataWeather->streamSpeed);}
	if(dataWeather->streamOverrideInt > 0) {mTide->setStreamOverride(dataWeather->streamOverrideInt);}

	break;
      }
    case E_CMD_MESSAGE_MAN_OVERBOARD:
      {
	sMob *dataMob = (sMob*)aDataCmd;

        if(dataMob->mobMode==1) mManOverboard->releaseManOverboard(mOwnShip->getPosition(), mOwnShip->getBreadth(), mOwnShip->getHeading());
	else if(dataMob->mobMode==-1) mManOverboard->retrieveManOverboard();

	break;
      }
    case E_CMD_MESSAGE_SET_MMSI:
      {
	sMmsi *dataMmmsi = (sMmsi*)aDataCmd;
        mOtherShips->setMMSI(dataMmmsi->shipNo, dataMmmsi->mmsi);
	
	break;
      }
    case E_CMD_MESSAGE_RUDDER_WORKING:
      {
	sRuddWork *dataRudderWorking = (sRuddWork*)aDataCmd;

	if(dataRudderWorking->whichPump==1)
	  {
	    if(dataRudderWorking->rudderFunction==0)
	      {
		//setRudderPumpState(1,false);
		//setAlarm(true);
	      }
	    else
	      {
		//setRudderPumpState(1,true);
		//if(getRudderPumpState(2))
		//setAlarm(false);
	      }
	  }
	else if(dataRudderWorking->whichPump==2)
	  {
	    if(dataRudderWorking->rudderFunction==0)
	      {
	        //setRudderPumpState(2,false);
		//setAlarm(true);
	      }
	    else
	      {
		//setRudderPumpState(2,true);
		//if(getRudderPumpState(1))
		//setAlarm(false);
	      }
	  }
	break;
      }
    case E_CMD_MESSAGE_RUDDER_FOLLOW_UP:
      {
	sRuddFol *dataRudderFollowUp = (sRuddFol*)aDataCmd;

	//if (dataRudderFollowUp->rudderFunction==1) setFollowUpRudderWorking(true);
	//else if(dataRudderFollowUp->rudderFunction==0) setFollowUpRudderWorking(false);

	break;
      }
    case E_CMD_MESSAGE_CONTROLS_OVERRIDE:
      {
	sCtrlOv	*dataCtrlOverride = (sCtrlOv*)aDataCmd;
	
	if(dataCtrlOverride->overrideMode == 0) mOwnShip->setWheel(dataCtrlOverride->overrideData); 
	else if(dataCtrlOverride->overrideMode == 1) mOwnShip->setPortEngine(dataCtrlOverride->overrideData);
	else if(dataCtrlOverride->overrideMode == 2) mOwnShip->setStbdEngine(dataCtrlOverride->overrideData);
	//else if(dataCtrlOverride->overrideMode == 7) setBowThruster(dataCtrlOverride->overrideData);
	//else if(dataCtrlOverride->overrideMode == 8) setSternThruster(dataCtrlOverride->overrideData);
	break;
      }
    case E_CMD_MESSAGE_BRIDGE_COMMAND:
      {
	sMasterCmdsInf *dataMasterCmds = (sMasterCmdsInf*)aDataCmd;

    
	/************************************************************************/
	if(dataMasterCmds->time.setTimeD)
	  setTimeDelta(dataMasterCmds->time.timeD);
	
	setAccelerator(dataMasterCmds->time.accel);

	/************************************************************************/
	mOwnShip->setPosition(dataMasterCmds->ownShip.posX, dataMasterCmds->ownShip.posZ);
        mOwnShip->setHeading(dataMasterCmds->ownShip.hdg);
	mOwnShip->setRateOfTurn(dataMasterCmds->ownShip.rot);
	mOwnShip->setSpeed(dataMasterCmds->ownShip.speed);
	
	/************************************************************************/
	if(dataMasterCmds->otherShips.nbrShips > 0)
	  {
	    for(unsigned short i=0; i<dataMasterCmds->otherShips.nbrShips; i++)
	      {
		mOtherShips->setHeading(i, dataMasterCmds->otherShips.ships[i].hdg);
		mOtherShips->setSpeed(i, (dataMasterCmds->otherShips.ships[i].speed)/MPS_TO_KTS);
		mOtherShips->setRateOfTurn(i, dataMasterCmds->otherShips.ships[i].rot);
		mOtherShips->setPos(i, dataMasterCmds->otherShips.ships[i].posX, dataMasterCmds->otherShips.ships[i].posZ);
	      }
	    delete[] dataMasterCmds->otherShips.ships;
	  }
	
	/************************************************************************/	
	if(dataMasterCmds->mob.isMob)
	  {
	    mManOverboard->setVisible(true);
	    mManOverboard->setPos(dataMasterCmds->mob.posX, dataMasterCmds->mob.posZ);
	  }
	else
	  mManOverboard->setVisible(false);

	/************************************************************************/
	if (dataMasterCmds->lines.lineNbr > 0)
	  {
	    if(dataMasterCmds->lines.lineNbr > (unsigned int)getLines()->getNumberOfLines(true))
	      {
		while(dataMasterCmds->lines.lineNbr > (unsigned int)getLines()->getNumberOfLines(true))
		  {
		    getLines()->addLine(this, true);
		  }
	      }
	    else if(dataMasterCmds->lines.lineNbr < (unsigned int)getLines()->getNumberOfLines(true))
	      {
		while (dataMasterCmds->lines.lineNbr < (unsigned int)getLines()->getNumberOfLines(true))
		  {
		    getLines()->removeLine(0, true);
		  }
	      }

	    bool lineKeepSlack = false;
	    if(dataMasterCmds->lines.lineKeepSlackInt == 1)
	      lineKeepSlack = true;
	   
	    bool lineHeaveIn = false;
	    if (dataMasterCmds->lines.lineHeaveInInt == 1)
	      lineHeaveIn = true;

	    for(unsigned short i=0; i<dataMasterCmds->lines.lineNbr; i++)
	      {
		if((getLines()->getLineStartType(i, true) != dataMasterCmds->lines.lineStartType) ||
		   (getLines()->getLineEndType(i, true) != dataMasterCmds->lines.lineEndType) ||
		   (getLines()->getLineStartID(i, true) != dataMasterCmds->lines.lineStartID) ||
		   (getLines()->getLineEndID(i, true) != dataMasterCmds->lines.lineEndID))
		  {
		    getLines()->clearLine(i, true);

		    if((dataMasterCmds->lines.lineStartType > 0) &&
		       (dataMasterCmds->lines.lineEndType > 0))
		      {
			irr::scene::ISceneNode* startParent = 0;
			irr::scene::ISceneNode* endParent = 0;
			if(dataMasterCmds->lines.lineStartType == 1)
			  {
			    // Own ship
			    startParent = mOwnShip->getSceneNode();
			  }
			else if(dataMasterCmds->lines.lineStartType == 2)
			  {
			    // Other ship
			    startParent = mOtherShips->getSceneNode(dataMasterCmds->lines.lineStartID);
			  }
			else if(dataMasterCmds->lines.lineStartType == 3)
			  {
			    // Buoy
			    startParent = mBuoys->getSceneNode(dataMasterCmds->lines.lineStartID);
			  }
			else if(dataMasterCmds->lines.lineStartType == 4)
			  {
			    // Land object
			    startParent = mLandObjects->getSceneNode(dataMasterCmds->lines.lineStartID);
			  }
			else if (dataMasterCmds->lines.lineStartType == 5)
			  {
			    // Terrain			   
			    startParent = mTerrain->getSceneNode(dataMasterCmds->lines.lineStartID);
			  }
			
			if(dataMasterCmds->lines.lineEndType == 1)
			  {
			    // Own ship
			    endParent = mOwnShip->getSceneNode();
			  }
			else if(dataMasterCmds->lines.lineEndType == 2)
			  {
			    // Other ship
			    endParent = mOtherShips->getSceneNode(dataMasterCmds->lines.lineEndID);
			  }
			else if(dataMasterCmds->lines.lineEndType == 3)
			  {
			    // Buoy
			    endParent = mBuoys->getSceneNode(dataMasterCmds->lines.lineEndID);
			  }
			else if(dataMasterCmds->lines.lineEndType == 4)
			  {
			    // Land object
			    endParent = mLandObjects->getSceneNode(dataMasterCmds->lines.lineEndID);
			  }
			else if (dataMasterCmds->lines.lineEndType == 5)
			  {
			    // Terrain
			    endParent = mTerrain->getSceneNode(dataMasterCmds->lines.lineEndID);			    
			  }

			// Make child sphere nodes based on these (in the right position), then pass in to create the lines
			irr::core::vector3df sphereScale = irr::core::vector3df(1.0, 1.0, 1.0);
			if(startParent && startParent->getScale().X > 0)
			  {
			    sphereScale = irr::core::vector3df(1.0f/startParent->getScale().X,
							       1.0f/startParent->getScale().X,
							       1.0f/startParent->getScale().X);
			  }
			irr::scene::ISceneNode* startNode = mDevice->getSceneManager()->addSphereSceneNode(0.25f,16,startParent,-1,irr::core::vector3df(dataMasterCmds->lines.lineStartX, dataMasterCmds->lines.lineStartY, dataMasterCmds->lines.lineStartZ),irr::core::vector3df(0, 0, 0),sphereScale);
			sphereScale = irr::core::vector3df(1.0, 1.0, 1.0);
			if(endParent && endParent->getScale().X > 0)
			  {
			    sphereScale = irr::core::vector3df(1.0f/endParent->getScale().X,1.0f/endParent->getScale().X,1.0f/endParent->getScale().X);
			  }
			irr::scene::ISceneNode* endNode = mDevice->getSceneManager()->addSphereSceneNode(0.25f,16,endParent,-1,irr::core::vector3df(dataMasterCmds->lines.lineEndX, dataMasterCmds->lines.lineEndY, dataMasterCmds->lines.lineEndZ),irr::core::vector3df(0, 0, 0),sphereScale);

			// Set name to match parent for convenience
			if (startParent && startNode) {
			  startNode->setName(startParent->getName());
			}
			if (endParent && endNode) {
			  endNode->setName(endParent->getName());
			}

			// Length factor can be hard coded as 1.0, as line nominal length will be updated later
			float lengthFactor = 1.0;

			// Create the lines
			getLines()->setLineStart(startNode, dataMasterCmds->lines.lineStartType, dataMasterCmds->lines.lineStartID, true, i);
			getLines()->setLineEnd(endNode, dataMasterCmds->lines.lineNominalShipMass, dataMasterCmds->lines.lineEndType, dataMasterCmds->lines.lineEndID, lengthFactor, true, i);
		      }
		  }

		// Check and update other parameters
		getLines()->setKeepSlack(i, lineKeepSlack, true);
		getLines()->setHeaveIn(i, lineHeaveIn, true);
		getLines()->setLineNominalLength(i, dataMasterCmds->lines.lineNominalLength, true);
		getLines()->setLineBreakingTension(i, dataMasterCmds->lines.lineBreakingTension, true);
		getLines()->setLineBreakingStrain(i, dataMasterCmds->lines.lineBreakingStrain, true);
		getLines()->setLineNominalShipMass(i, dataMasterCmds->lines.lineNominalShipMass, true);
	      }
	  }
	else
	  {	    
	    while(getLines()->getNumberOfLines(true) > 0)
	      {
		getLines()->removeLine(0, true);	    
	      }
	  }

	/************************************************************************/
        setWeather(dataMasterCmds->weather.weather);
	setVisibility(dataMasterCmds->weather.visibility);
	setRain(dataMasterCmds->weather.rain);
	mWind->setTrueSpeed(dataMasterCmds->weather.windSpeed);
	mWind->setTrueDirection(dataMasterCmds->weather.windDirection);
	mTide->setStreamOverrideDirection(dataMasterCmds->weather.streamDirection);
	mTide->setStreamOverrideSpeed(dataMasterCmds->weather.streamSpeed);
	mTide->setStreamOverride(dataMasterCmds->weather.streamOverrideInt);
	
	/************************************************************************/
	mCamera->setView(dataMasterCmds->view.view);

	/************************************************************************/
        mOwnShip->setWheel(dataMasterCmds->controls.wheel);
        //mOwnShip->setRudder(dataMasterCmds->controls.rudder);
        mOwnShip->setPortEngine(dataMasterCmds->controls.portEng);
	mOwnShip->setStbdEngine(dataMasterCmds->controls.stbdEng);
	//setBowThruster(dataMasterCmds->controls.bowThrust);
	//setSternThruster(dataMasterCmds->controls.sternThrust);
	
	break;
      }
    case E_CMD_MESSAGE_OWN_SHIP:
      {
	sShipInf *dataOwnShip = (sShipInf*)aDataCmd;
	
	mOwnShip->setPosition(dataOwnShip->posX, dataOwnShip->posZ);
	mOwnShip->setHeading(dataOwnShip->hdg);
	mOwnShip->setRateOfTurn(dataOwnShip->rot);
	mOwnShip->setSpeed(dataOwnShip->speed);
	break;    
      }
    case E_CMD_MESSAGE_SCENARIO:
      {


	break;
      }
    case E_CMD_MESSAGE_SHUTDOWN:
      {
	mDevice->closeDevice();

	break;
      }
    case E_CMD_MESSAGE_MULTIPLAYER_COMMAND:
      {
        sMasterCmdsInf* dataMasterCmds = (sMasterCmdsInf*)aDataCmd;


        /************************************************************************/
        if (dataMasterCmds->time.setTimeD)
	  setTimeDelta(dataMasterCmds->time.timeD);

        setAccelerator(dataMasterCmds->time.accel);
        /************************************************************************/
        if (dataMasterCmds->otherShips.nbrShips > 0)
	  {
            for (unsigned short i = 0; i < dataMasterCmds->otherShips.nbrShips; i++)
	      {
                mOtherShips->setHeading(i, dataMasterCmds->otherShips.ships[i].hdg);
                mOtherShips->setSpeed(i, (dataMasterCmds->otherShips.ships[i].speed) / MPS_TO_KTS);
                mOtherShips->setRateOfTurn(i, dataMasterCmds->otherShips.ships[i].rot);
                mOtherShips->setPos(i, dataMasterCmds->otherShips.ships[i].posX, dataMasterCmds->otherShips.ships[i].posZ);
	      }
            delete[] dataMasterCmds->otherShips.ships;
	  }

        break;
      }
    case E_CMD_MESSAGE_WIND_INJECTION:
      {
	sWeather *dataWeather = (sWeather*)aDataCmd;

	if(dataWeather->windSpeed > 0) {mWind->setTrueSpeed(dataWeather->windSpeed);}
	if(dataWeather->windDirection > 0) {mWind->setTrueDirection(dataWeather->windDirection);}

	break;
      }
    case E_CMD_MESSAGE_UNKNOWN:
    default:
      {	
	break;
      }
    }
}
