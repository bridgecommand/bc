#ifndef __COLLISION_HPP_INCLUDED__
#define __COLLISION_HPP_INCLUDED__

#include "irrlicht.h"
#include <vector>
#include "ModelParams.hpp"

class Terrain;
class OtherShips;
class OwnShip;
class Buoys;

struct ContactPoint
{
  irr::core::vector3df position; // position of the point on the ship's hull/outer surface
  irr::core::vector3df normal;
  irr::core::vector3df internalPosition; // Position within the ship, for use as a starting point for ray intersection checks
  irr::f32 torqueEffect;                 // From cross product, how much a unit force along the contact vector gives a torque around the vertical axis
  irr::f32 effectiveArea;                // Contact area represented (in m2)
};


class Collision
{
public:

  void load(irr::scene::IMeshSceneNode *aShipScene, Terrain *aTerrain, OtherShips *aOtherShips, OwnShip *aOwnShip, Buoys *aBuoys, ModelParameters aModelParameters, irr::IrrlichtDevice *aDev);
  void addContactPointFromRay(irr::core::line3d<irr::f32> aRay, irr::f32 aContactArea);
  void enableTriangleSelector(bool aSelectorEnabled);
  void DetectAndRespond(irr::f32 &reaction, irr::f32 &lateralReaction, irr::f32 &turnReaction);
  irr::scene::ISceneNode* getContactFromRay(irr::core::line3d<float> ray, irr::s32 linesMode);
  bool getBuoyCollision(void){return mBuoyCollision;}
  bool getOtherShipCollision(void){return mOtherShipCollision;}
  
private:

  irr::IrrlichtDevice *mDevice;
  irr::scene::IMeshSceneNode *mShipScene;
  irr::scene::ITriangleSelector *mSelector;
  bool mTriangleSelectorEnabled; 
  std::vector<ContactPoint> contactPoints;  
  bool mBuoyCollision;
  bool mOtherShipCollision;
  irr::scene::ISceneManager *mSmgr;
  Terrain   *mTerrain;
  OtherShips *mOtherShips;
  OwnShip *mOwnShip;
  Buoys *mBuoys;
  ModelParameters mModelParameters;


};

#endif
