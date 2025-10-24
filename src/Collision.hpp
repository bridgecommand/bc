#ifndef __COLLISION_HPP_INCLUDED__
#define __COLLISION_HPP_INCLUDED__

#include "irrlicht.h"
#include <vector>

class SimulationModel;

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

  void load(irr::scene::ISceneManager *aSmgr, irr::scene::IMeshSceneNode *aShipScene, irr::IrrlichtDevice *aDev, SimulationModel *aModel, float aHeightCorr);
  void addContactPointFromRay(irr::core::line3d<irr::f32> aRay, irr::f32 aContactArea);
  void enableTriangleSelector(bool aSelectorEnabled);
  void DetectAndRespond(irr::f32 &reaction, irr::f32 &lateralReaction, irr::f32 &turnReaction);
  irr::scene::ISceneNode* getContactFromRay(irr::core::line3d<float> ray, irr::s32 linesMode);
  bool getBuoyCollision(void){return mBuoyCollision;}
  bool getOtherShipCollision(void){return mOtherShipCollision;}
  
private:

  irr::scene::ISceneManager *mSmgr;
  irr::IrrlichtDevice *mDevice;
  irr::scene::IMeshSceneNode *mShipScene;
  irr::scene::ITriangleSelector *mSelector;
  bool mTriangleSelectorEnabled; 
  std::vector<ContactPoint> contactPoints;  
  SimulationModel *mModel;
  bool mBuoyCollision;
  bool mOtherShipCollision;
  float mHeightCorr;

};

#endif
