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

#ifndef __BUOYS_HPP_INCLUDED__
#define __BUOYS_HPP_INCLUDED__

#include "irrlicht.h"
#include <vector>
#include <string>

//Forward declarations
class Water;
class Terrain;
class Buoy;
class NavLight;
struct sTime;
struct RadarData;

class Buoys
{
    public:
        Buoys();
        virtual ~Buoys();
        void load(const std::string& aWorldName,Terrain *aTerrain, Water *aWater, irr::IrrlichtDevice* aDev);
        void update(sTime& aTime, float tideHeight, unsigned int lightLevel, irr::core::vector3df ownShipPosition, float ownShipLength);
        RadarData getRadarData(unsigned int number, irr::core::vector3df scannerPosition) const;
        unsigned int getNumber() const;
        irr::core::vector3df getPosition(int number) const;
        void moveNode(float deltaX, float deltaY, float deltaZ);
        void enableAllTriangleSelectors();
        irr::scene::ISceneNode* getSceneNode(int number);

    private:
        std::vector<Buoy> buoys;
        std::vector<NavLight*> buoysLights;
        Water* mWater;
};

#endif
