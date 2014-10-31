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

#ifndef __RADARCALCULATION_HPP_INCLUDED__
#define __RADARCALCULATION_HPP_INCLUDED__

#include "irrlicht.h"

#include <vector>

class Terrain;
class OwnShip;
class Buoys;
class OtherShips;

class RadarCalculation
{
    public:
        RadarCalculation();
        virtual ~RadarCalculation();
        void increaseRange();
        void decreaseRange();
        irr::f32 getRangeNm() const;
        void update(irr::video::IImage * radarImage, const Terrain& terrain, const OwnShip& ownShip, const Buoys& buoys, const OtherShips& otherShips, irr::f32 tideHeight, irr::f32 deltaTime, irr::f32 radarRainClutterReduction, irr::f32 radarSeaClutterReduction, irr::f32 radarGain);

    private:
        std::vector<std::vector<irr::f32> > scanArray;
        std::vector<std::vector<irr::f32> > scanArrayAmplified;
        std::vector<std::vector<irr::f32> > scanArrayAmplifiedPrevious;
        irr::s32 currentScanAngle;
        irr::u32 scanAngleStep;
        irr::u32 rangeResolution;
        irr::f32 radarRangeNm;
        void scan(const Terrain& terrain, const OwnShip& ownShip, const Buoys& buoys, const OtherShips& otherShips, irr::f32 tideHeight, irr::f32 deltaTime, irr::f32 radarRainClutterReduction, irr::f32 radarSeaClutterReduction, irr::f32 radarGain);
        irr::f32 radarNoise(irr::f32 radarNoiseLevel, irr::f32 radarSeaClutter, irr::f32 radarRainClutter, irr::f32 weather, irr::f32 radarRange,irr::f32 radarBrgDeg, irr::f32 windDirectionDeg, irr::f32 radarInclinationAngle, irr::f32 rainIntensity);
        void render(irr::video::IImage * radarImage);
        irr::f32 rangeAtAngle(irr::f32 checkAngle,irr::f32 centreX, irr::f32 centreZ, irr::f32 heading);
        void drawSector(irr::video::IImage * radarImage,irr::f32 centreX, irr::f32 centreY, irr::f32 innerRadius, irr::f32 outerRadius, irr::f32 startAngle, irr::f32 endAngle, irr::u32 alpha, irr::u32 red, irr::u32 green, irr::u32 blue);

};

#endif
