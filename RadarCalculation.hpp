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
        //void loadRadarCalculation();
        void update(irr::video::IImage * radarImage, const Terrain& terrain, const OwnShip& ownShip, const Buoys& buoys, const OtherShips& otherShips);

    private:
        std::vector<std::vector<irr::f32> > scanArray;
        irr::s32 currentScanAngle;


};

#endif
