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

#include "SimulationModel.hpp"

#include "ScenarioDataStructure.hpp"
#include "GUIMain.hpp"
#include "Terrain.hpp"
#include "Sky.hpp"
#include "Buoys.hpp"
#include "Sound.hpp"

#include "IniFile.hpp"
#include "Constants.hpp"
#include "Utilities.hpp"
#include "MessageMisc.hpp"

#include <cmath>
#include <fstream>

#ifdef WITH_PROFILING
#include "iprof.hpp"
#else
#define IPROF(a) //intentionally empty placeholder
#endif

//#include <ctime>

//using namespace irr;

SimulationModel::SimulationModel(irr::IrrlichtDevice* dev,
                                 irr::scene::ISceneManager* scene,
                                 GUIMain* gui,
                                 Sound* aSound,
                                 ScenarioData scenarioData,
                                 ModelParameters aModelParameters)
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
  
  mManOverboard->load(irr::core::vector3df(0,0,0),scene,dev,this,mTerrain);
  
  device = dev;
  smgr = scene;
  driver = scene->getVideoDriver();
  guiMain = gui;
  mSound = aSound;
  isMouseDown = false;
  moveViewWithPrimary = true;

  //Store a serialised form of the scenario loaded, as we may want to send this over the network
  serialisedScenarioData = scenarioData.serialise(false);

  
  mScenarioName = scenarioData.scenarioName;

  // Store model parameters
  mModelParameters = aModelParameters;

  //Set loop number to zero
  loopNumber = 0;

  worldName = scenarioData.worldName;
  float startTime = scenarioData.startTime;
  irr::u32 startDay=scenarioData.startDay;
  irr::u32 startMonth=scenarioData.startMonth;
  irr::u32 startYear=scenarioData.startYear;

  //load the sun times
  float sunRise = scenarioData.sunRise;
  float sunSet  = scenarioData.sunSet;
  if(sunRise==0.0) {sunRise=6;}
  if(sunSet==0.0) {sunSet=18;}

  //load the weather:
  //Fixme: add in wind direction etc
  mWeather = scenarioData.weather;
  mRainIntensity = scenarioData.rainIntensity;
  mVisibilityRange = scenarioData.visibilityRange;
  if (mVisibilityRange <= 0) {mVisibilityRange = 5*M_IN_NM;} //TODO: Check units

  mWindDirection = scenarioData.windDirection;
  mWindSpeed = scenarioData.windSpeed;

  //std::cout << "Wind direction: " << windDirection << " Wind speed: " << windSpeed << std::endl;

  //Fixme: Think about time zone handling
  //Fixme: Note that if the time_t isn't long enough, 2038 problem exists
  scenarioOffsetTime = Utilities::dmyToTimestamp(startDay,startMonth,startYear);//Time in seconds to start of scenario day (unix timestamp for 0000h on day scenario starts)

  //set internal scenario time to start
  mScenarioTime = startTime * SECONDS_IN_HOUR;

  //Set initial tide height to zero
  mTideHeight = 0;

  if (worldName == "") {
    //Could not load world name from scenario, so end here
    std::cerr << "World model name not defined" << std::endl;
    exit(EXIT_FAILURE);
  }

  //construct path to world model
  std::string worldPath = "world/";
  worldPath.append(worldName);

  //Check if this world model exists in the user dir.
  std::string userFolder = Utilities::getUserDir();
  if (Utilities::pathExists(userFolder + worldPath)) {
    worldPath = userFolder + worldPath;
  }

  // Store world model readme.txt file contents here if available
  std::string worldReadmePath = worldPath + "/readme.txt";
  worldModelReadmeText = "";
  if (Utilities::pathExists(worldReadmePath)) {
    std::ifstream file(worldReadmePath.c_str());
    if (file.is_open()) {
      std::string line;
      while (std::getline(file, line)) {
	worldModelReadmeText.append(line);
	worldModelReadmeText.append("\n");
      }
    }    
  }
        
  //Add terrain: Needs to happen first, so the terrain parameters are available
  mTerrain->load(worldPath, smgr, device, mModelParameters.limitTerrainResolution);

  //sky box/dome
  Sky sky (smgr);

  //Load own ship model.
  // TODO: It would be better to pass in modelParameters directly
  mOwnShip->load(scenarioData.ownShipData, mModelParameters, smgr, this, mTerrain, device);


  if(mModelParameters.mode == OperatingMode::Secondary) {
    mOwnShip->setSpeed(0); //Don't start moving if in secondary mode
  }

  mSolver.Init(mOwnShip);
  
  //Load rain
  mRain->load(smgr, mCamera->getSceneNode(), device, mOwnShip->getPosition().X, mOwnShip->getPosition().Y, mOwnShip->getPosition().Z, mOwnShip->getLength(), mOwnShip->getBreadth());

  //add water
  bool waterReflection = true;
  if (mModelParameters.vrMode == true) {
    waterReflection = false;
  }
  mWater->load(smgr,mOwnShip->getSceneNode(),mWeather,mModelParameters.disableShaders,waterReflection,mModelParameters.waterSegments);

  //To be replaced by getting information and passing into gui load method.
  //Tell gui to hide the second engine scroll bar if we have a single engine
  if (1 == mOwnShip->getNumberProp()) {
    gui->setSingleEngine();
  }

  //Tell gui to hide all ship controls if in secondary mode
  //if (mModelParameters.mode == OperatingMode::Secondary) {
  //gui->hideEngineAndRudder();
  //      TODO      gui->hideWheel();
  //	DEE_NOV22 todo hide schottels engine indicators etc
  //}

  //Tell the GUI what instruments to display - currently GPS and depth sounder
  //gui->setInstruments(mOwnShip->hasDepthSounder(),mOwnShip->getMaxSounderDepth(),mOwnShip->hasGPS());
  

  //Load the radar with config parameters
  mRadarCalculation->load(mOwnShip->getRadarConfigFile(),device);

  //set camera zoom to 1
  currentZoom = 1.0;
  zoomLevel = 7.0; //Default zoom of 7x

  //make a camera, setting parent and offset
  std::vector<irr::core::vector3df> views = mOwnShip->getCameraViews(); //Get the initial camera offset from the own ship model
  std::vector<bool> isHighView = mOwnShip->getCameraIsHighView(); //Are these special 'looking down' views
  float angleCorrection = mOwnShip->getAngleCorrection();
  mCamera->load(smgr,device->getLogger(),mOwnShip->getSceneNode(),views, isHighView,irr::core::degToRad(mModelParameters.viewAngle),mModelParameters.lookAngle,angleCorrection);
  mCamera->setNearValue(mModelParameters.cameraMinDistance);
  mCamera->setFarValue(mModelParameters.cameraMaxDistance);

  //make ambient light
  light.load(smgr,sunRise,sunSet, mCamera->getSceneNode());


  //Load other ships
  mOtherShips->load(scenarioData.otherShipsData,mScenarioTime,mModelParameters.mode,smgr,this,device);

  //Load buoys
  mBuoys->load(worldPath, smgr, this,device);

  //Load land objects
  landObjects.load(worldPath, smgr, this, mTerrain, device);

  //Load land lights
  landLights.load(worldPath, smgr, this, mTerrain);

  //Load tidal information
  mTide->load(worldPath, scenarioData);

        

  //Set up 3d engine/wheel controls/visualisation
  //portEngineVisual.load(smgr, mOwnShip->getSceneNode(), mOwnShip->getPortEngineControlPosition(), 1.0 / mOwnShip->getScaleFactor(), 0, 0); // 0 = regular throttle
  //stbdEngineVisual.load(smgr, mOwnShip->getSceneNode(), mOwnShip->getStbdEngineControlPosition(), 1.0 / mOwnShip->getScaleFactor(), 0, 0);
  //wheelVisual.load(smgr, mOwnShip->getSceneNode(), mOwnShip->getWheelControlPosition(), mOwnShip->getWheelControlScale() / mOwnShip->getScaleFactor(), 2, 1); // 1 = wheel

  //make a radar screen, setting parent and offset from own ship
  mRadarScreen->load(smgr,mOwnShip->getSceneNode(), mOwnShip->getRadarPosition(), mOwnShip->getRadarSize(), mOwnShip->getRadarTilt());

  //make radar image - one for the background render, and one with any 2d drawing on top
  //Make as big as the maximum screen display size (next power of 2), and then only use as much as is needed to get 1:1 image to screen pixel mapping
  irr::u32 radarTextureSize = driver->getScreenSize().Height*0.4; // Optimised for the small radar screen (Where 0.6*screen height is used for the 3d view). We should have a higher resolution for full radar view
  irr::u32 largeRadarTextureSize = driver->getScreenSize().Height; // Optimised for the large radar screen
  //Find next power of 2 size
  radarTextureSize = std::pow(2,std::ceil(std::log2(radarTextureSize)));
  largeRadarTextureSize = std::pow(2,std::ceil(std::log2(largeRadarTextureSize)));

  //In simulationModel, keep track of the used size, and pass this to gui etc.
  radarImage = driver->createImage (irr::video::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(radarTextureSize, radarTextureSize)); //Create image for radar calculation to work on
  radarImageOverlaid = driver->createImage (irr::video::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(radarTextureSize, radarTextureSize)); //Create image for radar calculation to work on
  radarImageLarge = driver->createImage (irr::video::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(largeRadarTextureSize, largeRadarTextureSize)); //Create image for radar calculation to work on
  radarImageOverlaidLarge = driver->createImage (irr::video::ECF_A8R8G8B8, irr::core::dimension2d<irr::u32>(largeRadarTextureSize, largeRadarTextureSize)); //Create image for radar calculation to work on
  //Images will be filled with background colour in RadarCalculation

  //make radar camera
  std::vector<irr::core::vector3df> radarViews; //Get the initial camera offset from the radar screen
  std::vector<bool> radarViewsLookDown; //Not needed for the radar camera, but needed for compatability
  float radarTilt = mOwnShip->getRadarTilt();
  radarViews.push_back(mOwnShip->getRadarPosition() + irr::core::vector3df(0,0.5*sin(irr::core::DEGTORAD*radarTilt)*mOwnShip->getRadarSize(),-0.5*cos(irr::core::DEGTORAD*radarTilt)*mOwnShip->getRadarSize()));
  radarViewsLookDown.push_back(false);
  mRadarCamera->load(smgr, device->getLogger(),mOwnShip->getSceneNode(),radarViews,radarViewsLookDown,irr::core::PI/2.0,0,0);
  mRadarCamera->setLookUp(-1.0 * radarTilt); //FIXME: Why doesn't simply -1.0*screenTilt work?
  mRadarCamera->updateViewport(1.0);
  mRadarCamera->setNearValue(0.8*0.5*mOwnShip->getRadarSize());
  mRadarCamera->setFarValue(1.2*0.5*mOwnShip->getRadarSize());
  
  //Hide the man overboard model
  mManOverboard->setVisible(false);

  //store time
  previousTime = device->getTimer()->getTime();

  guiData = new GUIData;

  // Initialise as paused to start with
  guiData->paused = true;

} //end of SimulationModel constructor

SimulationModel::~SimulationModel()
{
  radarImage->drop(); //We created this with 'create', so drop it when we're finished
  radarImageOverlaid->drop(); //We created this with 'create', so drop it when we're finished
  radarImageLarge->drop(); //We created this with 'create', so drop it when we're finished
  radarImageOverlaidLarge->drop(); //We created this with 'create', so drop it when we're finished

  delete guiData;
}



OwnShip* SimulationModel::getOwnShip(void)
{
  if(NULL != mOwnShip)
    return mOwnShip;
  else
    return NULL;
}

Terrain* SimulationModel::getTerrain(void)
{
 if(NULL != mTerrain)
    return mTerrain;
  else
    return NULL;
}

Water* SimulationModel::getWater(void)
{
 if(NULL != mWater)
    return mWater;
  else
    return NULL;
}

Tide* SimulationModel::getTide(void)
{
 if(NULL != mTide)
    return mTide;
  else
    return NULL;
}

Buoys* SimulationModel::getBuoys(void)
{
 if(NULL != mBuoys)
    return mBuoys;
  else
    return NULL;
}

OtherShips* SimulationModel::getOtherShips(void)
{
 if(NULL != mOtherShips)
    return mOtherShips;
  else
    return NULL;
}

Camera* SimulationModel::getCamera(void)
{
 if(NULL != mCamera)
    return mCamera;
  else
    return NULL;
}

RadarCalculation* SimulationModel::getRadarCalculation(void)
{
 if(NULL != mRadarCalculation)
    return mRadarCalculation;
  else
    return NULL;
}

RadarScreen* SimulationModel::getRadarScreen(void)
{
 if(NULL != mRadarScreen)
    return mRadarScreen;
  else
    return NULL;
}

Camera* SimulationModel::getRadarCamera(void)
{
 if(NULL != mRadarCamera)
    return mRadarCamera;
  else
    return NULL;
}

Lines* SimulationModel::getLines(void)
{
 if(NULL != mLines)
    return mLines;
  else
    return NULL;
}

Rain* SimulationModel::getRain(void)
{
 if(NULL != mRain)
    return mRain;
  else
    return NULL;
}

Sound* SimulationModel::getSound(void)
{
 if(NULL != mSound)
    return mSound;
  else
    return NULL;
}

ManOverboard* SimulationModel::getMoB(void)
{
 if(NULL != mManOverboard)
    return mManOverboard;
  else
    return NULL;
}

void SimulationModel::setAccelerator(float accelerator)
{
  device->getTimer()->setSpeed(accelerator);
}

float SimulationModel::getAccelerator() const
{
  return device->getTimer()->getSpeed();
}

/*Wether, Wind, Rain, Visibility*/
void SimulationModel::setWeather(float aWeather){mWeather = aWeather;}
float SimulationModel::getWeather() const {return mWeather;}
void SimulationModel::setRain(float aRainIntensity){mRainIntensity = aRainIntensity;}
float SimulationModel::getRain() const {return mRainIntensity;}
void SimulationModel::setVisibility(float aVisibilityNm){mVisibilityRange = aVisibilityNm;}
float SimulationModel::getVisibility() const{return mVisibilityRange;}
void SimulationModel::setWindDirection(float aWindDirection){mWindDirection = aWindDirection;}
float SimulationModel::getWindDirection() const{return mWindDirection;}
void SimulationModel::setWindSpeed(float aWindSpeed){mWindSpeed = aWindSpeed;}
float SimulationModel::getWindSpeed() const{return mWindSpeed;}
void SimulationModel::setApparentWindDir(float aApparentWindDir){mApparentWindDir = aApparentWindDir;}
void SimulationModel::setApparentWindSpd(float aApparentWindSpd){mApparentWindSpd = aApparentWindSpd;}
float SimulationModel::getApparentWindDir(void) const{return mApparentWindDir;}
float SimulationModel::getApparentWindSpd(void) const{return mApparentWindSpd;}

/*Time*/
unsigned long long SimulationModel::getTimestamp() const {return mAbsoluteTime;}
unsigned long long SimulationModel::getTimeOffset() const {return scenarioOffsetTime;} //The timestamp at the start of the first day of the scenario
void SimulationModel::setTimeDelta(float aScenarioTime){mScenarioTime = aScenarioTime;}
float SimulationModel::getTimeDelta() const {return mScenarioTime;} //The change in time (s) since the start of the start day of the scenario

/*Models Params*/
ModelParameters& SimulationModel::getModelParameters(void){return mModelParameters;}

void SimulationModel::setZoom(bool zoomOn) {
  if (zoomOn) {
    currentZoom = zoomLevel;
  }
  else {
    currentZoom = 1;
  }
  mCamera->setHFOV(irr::core::degToRad(mModelParameters.viewAngle) / currentZoom);
}
    
void SimulationModel::setZoom(bool zoomOn, float zoomLevel)
{
  this->zoomLevel = zoomLevel;
  setZoom(zoomOn);
}

void SimulationModel::setViewAngle(float viewAngle)
{
  mModelParameters.viewAngle = viewAngle;
  mCamera->setHFOV(irr::core::degToRad(mModelParameters.viewAngle) / currentZoom);
}

void SimulationModel::setMouseDown(bool isMouseDown)
{
  this->isMouseDown = isMouseDown;
}


irr::u32 SimulationModel::getLoopNumber() const
{
  return loopNumber;
}

std::string SimulationModel::getSerialisedScenario() const
{
  return serialisedScenarioData;
}

std::string SimulationModel::getScenarioName() const
{
  return mScenarioName;
}

std::string SimulationModel::getWorldName() const
{
  return worldName;
}

std::string SimulationModel::getWorldReadme() const
{
  return worldModelReadmeText;
}


bool SimulationModel::getMoveViewWithPrimary() const {
  return moveViewWithPrimary;
}

void SimulationModel::setMoveViewWithPrimary(bool moveView) {
  moveViewWithPrimary = moveView;
}


irr::scene::ISceneNode* SimulationModel::getContactFromRay(irr::core::line3d<float> ray, irr::s32 linesMode) {

  // Temporarily enable all required triangle selectors
  if (linesMode == 1) {
    // Start - on own ship
    mOwnShip->enableTriangleSelector(true);
  } else if (linesMode == 2) {
    // End - not on own ship
    mOtherShips->enableAllTriangleSelectors(); //This will be reset next time otherShips.update is called
    mBuoys->enableAllTriangleSelectors(); //This will be reset next time otherShips.update is called
    // TODO: Temporarily enable triangle selector for:
    //   Terrain
    //   Land objects
  } else {
    // Not start or end, return null;
    return 0;
  }

  irr::core::vector3df intersection;
  irr::core::triangle3df hitTriangle;

  irr::scene::ISceneNode * selectedSceneNode =
    smgr->getSceneCollisionManager()->getSceneNodeAndCollisionPointFromRay(
									   ray,
									   intersection, // This will be the position of the collision
									   hitTriangle, // This will be the triangle hit in the collision
									   IDFlag_IsPickable, // (bitmask), 0 for all
									   0); // Check all nodes

  irr::scene::ISceneNode* contactPointNode = 0;

  if (selectedSceneNode &&
      (
       ((linesMode == 1) && (selectedSceneNode == mOwnShip->getSceneNode())) || // Valid start node
       ((linesMode == 2) && (selectedSceneNode != mOwnShip->getSceneNode()))    // Valid end node
       )
      ) {

    // Add a 'sphere' scene node, with selectedSceneNode as parent.
    // Find local coordinates from the global one
    irr::core::vector3df localPosition(intersection);
    irr::core::matrix4 worldToLocal = selectedSceneNode->getAbsoluteTransformation();
    worldToLocal.makeInverse();
    worldToLocal.transformVect(localPosition);

    irr::core::vector3df sphereScale = irr::core::vector3df(1.0, 1.0, 1.0);
    if (selectedSceneNode && selectedSceneNode->getScale().X > 0) {
      sphereScale = irr::core::vector3df(1.0f/selectedSceneNode->getScale().X,
					 1.0f/selectedSceneNode->getScale().X,
					 1.0f/selectedSceneNode->getScale().X);
    }

    contactPointNode = smgr->addSphereSceneNode(0.25f,16,selectedSceneNode,-1,
						localPosition,
						irr::core::vector3df(0, 0, 0),
						sphereScale);

    // Set name to match parent for convenience
    contactPointNode->setName(selectedSceneNode->getName());
  }

  // Reset triangle selectors
  mOwnShip->enableTriangleSelector(false); // Own ship should not need triangle selectors at runtime (todo: for future robustness, check previous state and restore to this)
  // buoys and otherShips will be reset when their update() method is called

  return contactPointNode;
}


irr::scene::ISceneNode* SimulationModel::getLandObjectSceneNode(int number)
{
  return landObjects.getSceneNode(number);
}

void SimulationModel::updateCameraVRPos(irr::core::quaternion quat, irr::core::vector3df pos, irr::core::vector2df lensShift)
{
  mCamera->update(0, quat, pos, lensShift, true);
}

void SimulationModel::update()
{

#ifdef WITH_PROFILING
  IPROF_FUNC;
#endif
  // DEE vvvv debug I think that this is effectively the cycle

  //Declare here, so scope added as part of profiling isn't a problem
  irr::u32 lightLevel;
  float elevAngle;
  irr::core::vector2di cursorPositionRadar;
  std::vector<float> CPAs;
  std::vector<float> TCPAs;
  std::vector<float> headings;
  std::vector<float> speeds;
  bool paused;
  bool collided;

  { IPROF("Increment time");

    // move time along .. this goes before everything else in the cycle

    //get delta time
    currentTime = device->getTimer()->getTime();
    deltaTime = (currentTime - previousTime)/1000.f;
    //deltaTime = (currentTime - previousTime)/1000.f;
    previousTime = currentTime;

    //add this to the scenario time
    mScenarioTime += deltaTime;
    mAbsoluteTime = Utilities::round(mScenarioTime) + scenarioOffsetTime;

    //increment loop number
    loopNumber++;

    // end move time along
  }{ IPROF("Set radar display radius");


    //Ensure we have the right radar screen resolution
    mRadarCalculation->setRadarDisplayRadius(guiMain->getRadarPixelRadius());
    mRadarScreen->setRadarDisplayRadius(guiMain->getRadarPixelRadius());
    
  }{ IPROF("Update tide");

    //Update tide height and tidal stream here.
    mTide->update(mAbsoluteTime);
    mTideHeight = mTide->getTideHeight();

  }{ IPROF("Update lighting");

    //update ambient lighting
    light.update(mScenarioTime);
    //Note that linear fog is hardcoded into the water shader, so should be changed there if we use other fog types
    driver->setFog(light.getLightSColor(), irr::video::EFT_FOG_LINEAR , 0.01*mVisibilityRange*M_IN_NM, mVisibilityRange*M_IN_NM, 0.00003f /*exp fog parameter*/, true, true);
    lightLevel = light.getLightLevel();

  }{ IPROF("Update rain");
    //update rain
    //rain.setIntensity(rainIntensity);
    mRain->update(mOwnShip->getPosition().X, mOwnShip->getPosition().Y, mOwnShip->getPosition().Z, mRainIntensity);

  }{ IPROF("Update other ships");
    //update other ship positions etc
    mOtherShips->update(deltaTime,mScenarioTime,mTideHeight,lightLevel,mOwnShip->getPosition(),mOwnShip->getLength()); //Update other ship motion (based on leg information), and light visibility.

  }{ IPROF("Update buoys");
    //update buoys (for lights, floating, and if collision detection is turned on)
    mBuoys->update(deltaTime,mScenarioTime,mTideHeight,lightLevel,mOwnShip->getPosition(),mOwnShip->getLength());

  }{ IPROF("Update land lights");
    //Update land lights
    landLights.update(deltaTime,mScenarioTime,lightLevel);

  } { IPROF("Update lines");
    //update all lines, ready to be used for own ship force
    mLines->update(deltaTime);
  }{ IPROF("Update own ship");

    mSolver.SolveRk4(mOwnShip->getEta(), mOwnShip->getMu(), deltaTime);
    mOwnShip->setEta(mSolver.getEta());
    mOwnShip->setMu(mSolver.getMu());

    //std::cout << "eta : " << mOwnShip->getEta() << " - mu : " << mOwnShip->getMu();
    //update own ship
    mOwnShip->update(deltaTime, mScenarioTime, mTideHeight, mWeather, mLines->getOverallForceLocal(), mLines->getOverallTorqueLocal());

    if (mOwnShip->getNumberProp() > 1)
      mSound->setVolumeEngine(fabs(mOwnShip->getPortEngine())*0.5);
    else 
      mSound->setVolumeEngine((fabs(mOwnShip->getPortEngine()) + fabs(mOwnShip->getStbdEngine()))*0.5);
  
  }{ IPROF("Update MOB");
    //update man overboard
    mManOverboard->update(deltaTime, mTideHeight);

  }{ IPROF("Check for collisions");
    //Check for collisions
    collided = checkOwnShipCollision();

  }{ IPROF("Update water pos");
    //update water position
    mWater->update(mTideHeight,mCamera->getPosition(),light.getLightLevel(), mWeather);

  }{ IPROF("Update camera pos");

    //update the camera position
    mCamera->update(deltaTime);
  }{ IPROF("Update controls visualisation");
    portEngineVisual.update(45.0 * mOwnShip->getPortEngine());
    stbdEngineVisual.update(45.0 * mOwnShip->getStbdEngine());
    wheelVisual.update(-6.0 * mOwnShip->getWheel());
  }
  if (mRadarCalculation->isRadarOn()) {
    { IPROF("Update radar cursor position");
      //set radar screen position, and update it with a radar image from the radar calculation
      cursorPositionRadar = guiMain->getCursorPositionRadar();
    }{ IPROF("Update radar calculation");
      //Choose which radar images to use, depending on the size of the display being used
      if (2*guiMain->getRadarPixelRadius() > radarImage->getDimension().Width) {
	radarImageChosen = radarImageLarge;
	radarImageOverlaidChosen = radarImageOverlaidLarge;
      } else {
	radarImageChosen = radarImage;
	radarImageOverlaidChosen = radarImageOverlaid;
      }
      mRadarCalculation->update(radarImageChosen,radarImageOverlaidChosen,mTerrain,mOwnShip,mBuoys,mOtherShips,mWeather,mRainIntensity,mTideHeight,deltaTime,mAbsoluteTime,cursorPositionRadar,isMouseDown);
    }{ IPROF("Update radar screen");
      mRadarScreen->update(radarImageOverlaidChosen);
    }{ IPROF("Update radar camera");
      mRadarCamera->update();
    }
  } else {
    mRadarScreen->getSceneNode()->setVisible(false);
  }
  { IPROF("Check if paused ");
    //check if paused
    paused = device->getTimer()->getSpeed()==0.0;

  }{ IPROF("Get radar ARPA data for GUI");

    //get radar ARPA data to show
    irr::u32 numberOfARPATracks = mRadarCalculation->getARPATracksSize();
    guiData->arpaContactStates.clear();
    for(unsigned int i = 0; i<numberOfARPATracks; i++) {
      guiData->arpaContactStates.push_back(mRadarCalculation->getARPAContactFromTrackIndex(i).estimate);
    }
    guiData->arpaListSelection = mRadarCalculation->getArpaListSelection();
  
    }{ IPROF("Collate GUI data ");

    float posZ = mOwnShip->getPosition().Z;
    float posX = mOwnShip->getPosition().X;
    
    //Collate data to show in gui
    guiData->lat = mTerrain->zToLat(posZ);
    guiData->longitude = mTerrain->xToLong(posX);
    guiData->hdg = mOwnShip->getHeading();
        
    irr::core::vector3df cameraForwardVector = mCamera->getForwardVector(); 
    guiData->viewAngle = atan2(cameraForwardVector.X, cameraForwardVector.Z) * irr::core::RADTODEG;
    guiData->viewElevationAngle = asin(cameraForwardVector.Y) * irr::core::RADTODEG;

    guiData->spd = mOwnShip->getSpeedThroughWater();
    guiData->portEng = mOwnShip->getPortEngine();
    guiData->stbdEng = mOwnShip->getStbdEngine();
    guiData->rudder = mOwnShip->getRudder().getDelta();  // inner workings of this will be modified in model DEE
    guiData->wheel = mOwnShip->getWheel();    // inner workings of this will be modified in model DEE
    guiData->depth = mOwnShip->getDepth(getTerrain());
    guiData->weather = mWeather;
    guiData->rain = mRainIntensity;
    guiData->visibility = mVisibilityRange;
    guiData->windDirection = mWindDirection;
    guiData->windSpeed = mWindSpeed;
    guiData->streamDirection = mTide->getStreamOverrideDirection();
    guiData->streamSpeed = mTide->getStreamOverrideSpeed();
    guiData->streamOverride = mTide->getStreamOverride();
    guiData->radarRangeNm = mRadarCalculation->getRangeNm();
    guiData->radarGain = mRadarCalculation->getGain();
    guiData->radarClutter = mRadarCalculation->getClutter();
    guiData->radarRain = mRadarCalculation->getRainClutter();
    guiData->guiRadarEBLBrg = mRadarCalculation->getEBLBrg();
    guiData->guiRadarEBLRangeNm = mRadarCalculation->getEBLRangeNm();
    guiData->guiRadarCursorBrg = mRadarCalculation->getCursorBrg();
    guiData->guiRadarCursorRangeNm = mRadarCalculation->getCursorRangeNm();
    guiData->currentTime = Utilities::timestampToString(mAbsoluteTime);
    guiData->paused = paused;
    guiData->collided = collided;
    guiData->headUp = mRadarCalculation->getHeadUp();
    guiData->radarOn = mRadarCalculation->isRadarOn();
    //guiData->pump1On = mOwnShip->getRudderPumpState(1);
    //guiData->pump2On = mOwnShip->getRudderPumpState(2);

    // DEE_NOV22 ^^^^

    // DEE FEB 23 vvv
    guiData->tideHeight = mTideHeight;
    // DEE FEB 23 ^^^

    // DEE vvvv units are rad per second
    guiData->RateOfTurn = mOwnShip->getRateOfTurn();
    // DEE ^^^^
  }{ IPROF("Update gui data");
    //send data to gui
    guiMain->updateGuiData(guiData); //Set GUI heading in degrees and speed (in m/s)
  }
}

void SimulationModel::updateFromNetwork(eCmdMsg aMsgType, void* aDataCmd)
{
  switch(aMsgType)
    {
    case E_CMD_MESSAGE_UPDATE_LEG:
      {
	sUpLeg *dataUpdateLeg = (sUpLeg*)aDataCmd;
	mOtherShips->changeLeg(dataUpdateLeg->shipNo, dataUpdateLeg->legNo, dataUpdateLeg->bearing, dataUpdateLeg->speed, dataUpdateLeg->dist, mScenarioTime);
	break;
      }
    case E_CMD_MESSAGE_DELETE_LEG:
      {
	sDelLeg *dataDeleteLeg = (sDelLeg*)aDataCmd;
	mOtherShips->deleteLeg(dataDeleteLeg->shipNo, dataDeleteLeg->legNo, mScenarioTime);
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
	    mOtherShips->resetLegs(dataResetLegs->shipNo, dataResetLegs->cog, dataResetLegs->sog, 1, mScenarioTime);
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
	if(dataWeather->windSpeed > 0) {setWindSpeed(dataWeather->windSpeed);}
	if(dataWeather->windDirection > 0) {setWindDirection(dataWeather->windDirection);}
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
			    startParent = getLandObjectSceneNode(dataMasterCmds->lines.lineStartID);
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
			    endParent = getLandObjectSceneNode(dataMasterCmds->lines.lineEndID);
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
			irr::scene::ISceneNode* startNode = device->getSceneManager()->addSphereSceneNode(0.25f,16,startParent,-1,irr::core::vector3df(dataMasterCmds->lines.lineStartX, dataMasterCmds->lines.lineStartY, dataMasterCmds->lines.lineStartZ),irr::core::vector3df(0, 0, 0),sphereScale);
			sphereScale = irr::core::vector3df(1.0, 1.0, 1.0);
			if(endParent && endParent->getScale().X > 0)
			  {
			    sphereScale = irr::core::vector3df(1.0f/endParent->getScale().X,1.0f/endParent->getScale().X,1.0f/endParent->getScale().X);
			  }
			irr::scene::ISceneNode* endNode = device->getSceneManager()->addSphereSceneNode(0.25f,16,endParent,-1,irr::core::vector3df(dataMasterCmds->lines.lineEndX, dataMasterCmds->lines.lineEndY, dataMasterCmds->lines.lineEndZ),irr::core::vector3df(0, 0, 0),sphereScale);

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
	setWindSpeed(dataMasterCmds->weather.windSpeed);
	setWindDirection(dataMasterCmds->weather.windDirection);
	mTide->setStreamOverrideDirection(dataMasterCmds->weather.streamDirection);
	mTide->setStreamOverrideSpeed(dataMasterCmds->weather.streamSpeed);
	mTide->setStreamOverride(dataMasterCmds->weather.streamOverrideInt);
	
	/************************************************************************/
	mCamera->setView(dataMasterCmds->view.view);

	/************************************************************************/
        mOwnShip->setWheel(dataMasterCmds->controls.wheel);
        //setRudder(dataMasterCmds->controls.rudder);
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
	device->closeDevice();

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

	if(dataWeather->windSpeed > 0) {setWindSpeed(dataWeather->windSpeed);}
	if(dataWeather->windDirection > 0) {setWindDirection(dataWeather->windDirection);}

	break;
      }
    case E_CMD_MESSAGE_UNKNOWN:
    default:
      {	
	break;
      }
    }
}

bool SimulationModel::checkOwnShipCollision()
{

  return (mOwnShip->isBuoyCollision() || mOwnShip->isOtherShipCollision());

  /*

    irr::u32 numberOfOtherShips = otherShips.getNumber();
    irr::u32 numberOfBuoys = buoys.getNumber();

    irr::core::vector3df thisShipPosition = mOwnShip->getPosition();
    float thisShipLength = mOwnShip->getLength();
    float thisShipWidth = mOwnShip->getWidth();
    float thisShipHeading = mOwnShip->getHeading();

    for (irr::u32 i = 0; i<numberOfOtherShips; i++) {
    irr::core::vector3df otherPosition = otherShips.getPosition(i);
    float otherShipLength = otherShips.getLength(i);
    float otherShipWidth = otherShips.getWidth(i);
    float otherShipHeading = otherShips.getHeading(i);

    irr::core::vector3df relPosition = otherPosition - thisShipPosition;
    float distanceToShip = relPosition.getLength();
    float bearingToOtherShipDeg = irr::core::radToDeg(atan2(relPosition.X, relPosition.Z));

    //Bearings relative to ship's head (from this ship and from other)
    float relativeBearingOwnShip = bearingToOtherShipDeg - thisShipHeading;
    float relativeBearingOtherShip = 180 + bearingToOtherShipDeg - otherShipHeading;

    //Find the minimum distance before a collision occurs
    float minDistanceOwn = 0.5*fabs(thisShipWidth*sin(irr::core::degToRad(relativeBearingOwnShip))) + 0.5*fabs(thisShipLength*cos(irr::core::degToRad(relativeBearingOwnShip)));
    float minDistanceOther = 0.5*fabs(otherShipWidth*sin(irr::core::degToRad(relativeBearingOtherShip))) + 0.5*fabs(otherShipLength*cos(irr::core::degToRad(relativeBearingOtherShip)));
    float minDistance = minDistanceOther + minDistanceOwn;

    if (distanceToShip < minDistance) {
    return true;
    }
    }

    for (irr::u32 i = 0; i<numberOfBuoys; i++) { //Collision with buoy
    irr::core::vector3df otherPosition = buoys.getPosition(i);

    irr::core::vector3df relPosition = otherPosition - thisShipPosition;
    float distanceToBuoy = relPosition.getLength();
    float bearingToBuoyDeg = irr::core::radToDeg(atan2(relPosition.X, relPosition.Z));

    //Bearings relative to ship's head (from this ship and from other)
    float relativeBearingOwnShip = bearingToBuoyDeg - thisShipHeading;

    //Find the minimum distance before a collision occurs
    float minDistanceOwn = 0.5*fabs(thisShipWidth*sin(irr::core::degToRad(relativeBearingOwnShip))) + 0.5*fabs(thisShipLength*cos(irr::core::degToRad(relativeBearingOwnShip)));

    if (distanceToBuoy < minDistanceOwn) {
    return true;
    }
    }

    return false; //If no collision has been found
  */
}

