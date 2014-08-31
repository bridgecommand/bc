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
        void update(irr::video::IImage * radarImage, const Terrain& terrain, const OwnShip& ownShip, const Buoys& buoys, const OtherShips& otherShips, irr::f32 tideHeight, irr::f32 deltaTime);

    private:
        std::vector<std::vector<irr::f32> > scanArray;
        irr::s32 currentScanAngle;
        irr::u32 scanAngleStep;
        irr::u32 rangeResolution;
        irr::f32 radarRangeNm;
        void scan(const Terrain& terrain, const OwnShip& ownShip, const Buoys& buoys, const OtherShips& otherShips, irr::f32 tideHeight, irr::f32 deltaTime);
        void render(irr::video::IImage * radarImage, irr::f32 amplification);
        irr::f32 rangeAtAngle(irr::f32 checkAngle,irr::f32 centreX, irr::f32 centreZ, irr::f32 heading);
        void drawSector(irr::video::IImage * radarImage,irr::f32 centreX, irr::f32 centreY, irr::f32 innerRadius, irr::f32 outerRadius, irr::f32 startAngle, irr::f32 endAngle, irr::u32 alpha, irr::u32 red, irr::u32 green, irr::u32 blue);

};

#endif
