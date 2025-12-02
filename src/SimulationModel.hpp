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

#include "MessageMisc.hpp"
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
#include "Solver.hpp"
#include "Sound.hpp"
#include "Collision.hpp"
#include "Wind.hpp"
#include "Time.hpp"
#include "ModelParams.hpp"

class SimulationModel //Start of the 'Model' part of MVC
{

public:

  SimulationModel();
  SimulationModel(irr::IrrlichtDevice* aDev, GUIMain* aGui, Sound* aSound, ScenarioData aScenarioData, ModelParameters aModelParameters);
  ~SimulationModel();

  /*Time*/
  void setAccelerator(float aAccelerator); //Set simulation time compression
  float getAccelerator() const;
  unsigned long long getTimestamp() const; //The unix timestamp in s
  unsigned long long getTimeOffset() const; //The timestamp at the start of the first day of the scenario
  float getTimeDelta() const; //The change in time (s) since the start of the start day of the scenario
  void  setTimeDelta(float aScenarioTime);
  irr::u32 getLoopNumber() const;
  
  /*Weather*/
  void setWeather(float aWeather); 
  float getWeather() const;
  void setRain(float aRainIntensity); 
  float getRain() const;
  void setVisibility(float aVisibilityNm);
  float getVisibility() const;

  /*Views*/
  void setZoom(bool zoomOn);
  void setZoom(bool zoomOn, float zoomLevel);
  void setViewAngle(float viewAngle);
  bool getMoveViewWithPrimary() const;
  void setMoveViewWithPrimary(bool moveView);
  void updateCameraVRPos(irr::core::quaternion quat, irr::core::vector3df pos, irr::core::vector2df lensShift);
  
  /*Scenario*/
  std::string getSerialisedScenario() const;
  std::string getScenarioName() const;
  std::string getWorldName() const;
  std::string getWorldReadme() const;

  /*Model Params*/
  ModelParameters& getModelParameters();

  /*Collision*/
  irr::scene::ISceneNode* getContactFromRay(irr::core::line3d<float> ray, irr::s32 linesMode);
    
  /*Class pointer*/
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
  Collision* getCollision(void);
  Wind* getWind(void);
  
  /*Update*/
  void update(void);
  void updateFromNetwork(eCmdMsg aMsgType, void* aDataCmd);
  
private:

  /*Irrlicht handler*/
  irr::IrrlichtDevice *mDevice;
  irr::video::IVideoDriver *mDriver;
  irr::scene::ISceneManager *mSmgr;

  /*Model Params*/
  ModelParameters mModelParameters;
    
  /*Weather*/
  float mTideHeight;
  float mWeather; //0-12.0
  float mRainIntensity; //0-10
  float mVisibilityRange; //Nm

  /*Time*/
  sTime mTime;
  
  /*Scenario*/
  std::string mScenarioName;
  std::string mWorldName;
  std::string mSerialisedScenarioData;
  std::string mWorldModelReadmeText;

  /*Class pointer*/
  Solver *mSolver;
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
  GUIData* mGuiData;
  GUIMain* mGuiMain;
  LandObjects* mLandObjects;
  LandLights* mLandLights;
  Light* mLight;
  Collision* mCollision;
  Wind* mWind;
  
  /*Views*/
  float mCurrentZoom; // Zoom currently in use
  float mZoomLevel; // Zoom level that should be used if binos are on
  bool mMoveViewWithPrimary;
};
#endif
