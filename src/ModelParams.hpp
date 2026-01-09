#ifndef MODELPARAMS_HPP
#define MODELPARAMS_HPP

#include "irrlicht.h"
#include "OperatingModeEnum.hpp"


struct ModelParameters
{
  OperatingMode::Mode mode;
  bool vrMode;
  float viewAngle;
  float lookAngle;
  float cameraMinDistance;
  float cameraMaxDistance;
  unsigned int disableShaders;
  unsigned int waterSegments;
  irr::core::vector3di numberOfContactPoints;
  float minContactPointSpacing;
  float contactStiffnessFactor;
  float contactDampingFactor;
  float lineStiffnessFactor;
  float lineDampingFactor;
  float frictionCoefficient;
  float tanhFrictionFactor;
  unsigned int limitTerrainResolution;
  bool secondaryControlWheel;
  bool secondaryControlPortEngine;
  bool secondaryControlStbdEngine;
  bool secondaryControlBowThruster;
  bool secondaryControlSternThruster;
  bool debugMode;
};

#endif
