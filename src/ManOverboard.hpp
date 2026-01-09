/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2017 James Packer

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

#ifndef __MANOVERBOARD_HPP_INCLUDED__
#define __MANOVERBOARD_HPP_INCLUDED__

#include "irrlicht.h"
#include "Time.hpp"

//Forward declarations
class Terrain;
class Tide;
class Wind;
class Water;

class ManOverboard
{
public:
  ManOverboard();
  ~ManOverboard();
  void load(const irr::core::vector3df& aLocation, Terrain* aTerrain, Water* aWater, Wind* aWind, Tide* aTide, irr::IrrlichtDevice* aDev);
  irr::core::vector3df getPosition() const;
  
  void moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ);
  void update(sTime& aTime, irr::f32 tideHeight);
  irr::scene::ISceneNode* getSceneNode() const;

  void releaseManOverboard(irr::core::vector3df aOwnShipPos, float aBreadth, float aHeading);
  void retrieveManOverboard();
  bool getVisible() const;
  float getPosX() const;
  float getPosZ() const;
  void setVisible(bool visible); 
  void setPos(float positionX, float positionZ);   


protected:
private:
  irr::scene::IMeshSceneNode* man; //The scene node for the man overboard model.
  Terrain *mTerrain;
  Water *mWater;
  Wind *mWind;
  Tide *mTide;
};

#endif
