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

#ifndef __SIMULATIONMODEL_HPP_INCLUDED__
#define __SIMULATIONMODEL_HPP_INCLUDED__

#include <iostream> //For debugging
#include <string>
#include <vector>
#include <stdint.h> //for uint64_t

#include "irrlicht.h"

//Forward declarations
class ScenarioData;
class GUIMain;
class GUIData;


#include "Terrain.hpp"
#include "Light.hpp"
#include "Water.hpp"
#include "Rain.hpp"
#include "Tide.hpp"
#include "Buoys.hpp"
#include "OtherShips.hpp"
#include "LandObjects.hpp"
#include "LandLights.hpp"
#include "OwnShip.hpp"
#include "ManOverboard.hpp"
#include "Camera.hpp"
#include "RadarCalculation.hpp"
#include "RadarScreen.hpp"
#include "ControlVisualiser.hpp"
#include "Lines.hpp"
#include "OperatingModeEnum.hpp"
#include "Network.hpp"
#include "Solver.hpp"
#include "Sound.hpp"


class SimulationModel //Start of the 'Model' part of MVC
{

public:
    
  SimulationModel(irr::IrrlichtDevice* dev,
		  irr::scene::ISceneManager* scene,
		  GUIMain* gui,
		  Sound* sound,
		  ScenarioData scenarioData,
		  ModelParameters aModelParameters);
  ~SimulationModel();

  void setAccelerator(float accelerator); //Set simulation time compression
  float getAccelerator() const;

  unsigned long long getTimestamp() const; //The unix timestamp in s
  unsigned long long getTimeOffset() const; //The timestamp at the start of the first day of the scenario
  float getTimeDelta() const; //The change in time (s) since the start of the start day of the scenario
  void  setTimeDelta(float scenarioTime);

  void setWeather(float aWeather); 
  float getWeather() const;
  void setRain(float aRainIntensity); 
  float getRain() const;
  void setVisibility(float aVisibilityNm);
  float getVisibility() const;
  void setWindDirection(float aWindDirection);
  float getWindDirection() const;
  void setWindSpeed(float aWindSpeed); 
  float getWindSpeed() const;
  void setApparentWindDir(float aApparentWindDir);
  float getApparentWindDir(void) const;
  void setApparentWindSpd(float aApparentWindSpd);
  float getApparentWindSpd(void) const;

  void setMouseDown(bool isMouseDown);
  void setZoom(bool zoomOn);
  void setZoom(bool zoomOn, float zoomLevel);
  void setViewAngle(float viewAngle);
  irr::u32 getLoopNumber() const;
  std::string getSerialisedScenario() const;
  std::string getScenarioName() const;
  std::string getWorldName() const;
  std::string getWorldReadme() const;

  bool getMoveViewWithPrimary() const;
  void setMoveViewWithPrimary(bool moveView);

  ModelParameters& getModelParameters();


  irr::scene::ISceneNode* getContactFromRay(irr::core::line3d<float> ray, irr::s32 linesMode);
    
  irr::scene::ISceneNode* getLandObjectSceneNode(int number);


  Rain* getRain(void); 
  Lines* getLines(void); 
  OwnShip* getOwnShip(void);
  Terrain* getTerrain(void);
  Water* getWater(void);
  Tide* getTide(void);
  Buoys* getBuoys(void);
  OtherShips* getOtherShips(void);
  Camera* getCamera(void);
  RadarCalculation* getRadarCalculation(void);
  RadarScreen* getRadarScreen(void);
  Camera* getRadarCamera(void);
  Sound* getSound(void);
  ManOverboard* getMoB(void);
  
  void updateCameraVRPos(irr::core::quaternion quat, irr::core::vector3df pos, irr::core::vector2df lensShift);
  void update();
  void updateFromNetwork(eCmdMsg aMsgType, void* aDataCmd);  
private:
  irr::IrrlichtDevice* device;
  irr::video::IVideoDriver* driver;
  irr::scene::ISceneManager* smgr;

  ModelParameters mModelParameters;
    
  irr::video::IImage* radarImage; //Basic radar image
  irr::video::IImage* radarImageOverlaid; //WIth any 2d overlay
  irr::video::IImage* radarImageLarge; //Basic radar image, for full screen display
  irr::video::IImage* radarImageOverlaidLarge; //WIth any 2d overlay, for full screen display
  irr::video::IImage* radarImageChosen; //Should point to one of radarImage or radarImageLarge
  irr::video::IImage* radarImageOverlaidChosen; //Should point to one of radarImageOverlaid or radarImageOverlaidLarge
  //float accelerator;
  float mTideHeight;
  float mWeather; //0-12.0
  float mRainIntensity; //0-10
  float mVisibilityRange; //Nm
  float mWindDirection; //0-360
  float mWindSpeed; //Nm
  float mApparentWindDir;
  float mApparentWindSpd;

  irr::u32 loopNumber; //u32 should be up to 4,294,967,295, so over 2 years at 60 fps
  float currentZoom; // Zoom currently in use
  float zoomLevel; // Zoom level that should be used if binos are on

  
  LandObjects landObjects;
  LandLights landLights;
  Light light;
  Solver mSolver;

  OwnShip *mOwnShip;
  OtherShips *mOtherShips;
  Buoys *mBuoys;
  Camera *mCamera;
  Water *mWater;
  Tide *mTide;
  Terrain *mTerrain;
  RadarCalculation *mRadarCalculation;
  RadarScreen *mRadarScreen;
  Camera *mRadarCamera;
  Lines *mLines;  
  Rain *mRain;
  Sound* mSound;
  ManOverboard *mManOverboard;

  
  ControlVisualiser portEngineVisual;
  ControlVisualiser stbdEngineVisual;
  ControlVisualiser portAzimuthThrottleVisual;
  ControlVisualiser stbdAzimuthThrottleVisual;
  ControlVisualiser wheelVisual;
  GUIMain* guiMain;
  
  bool isMouseDown; //Updated by the event receiver, used by radar
  bool moveViewWithPrimary;

  //Simulation time handling
  irr::u32 currentTime; //Computer clock time
  irr::u32 previousTime; //Computer clock time
  float deltaTime;
  float mScenarioTime; //Simulation internal time, starting at zero at 0000h on start day of simulation
  unsigned long long scenarioOffsetTime; //Simulation day's start time from unix epoch (1 Jan 1970)
  unsigned long long mAbsoluteTime; //Unix timestamp for current time, including start day. Calculated from scenarioTime and scenarioOffsetTime

  //utility function to check for collision
  bool checkOwnShipCollision();


  //store useful information
  std::string mScenarioName;
  std::string worldName;
  std::string serialisedScenarioData;
  std::string worldModelReadmeText;

  //Structure to pass data to gui
  GUIData* guiData;
};
#endif
