#ifndef __RADARCALCULATION_HPP_INCLUDED__
#define __RADARCALCULATION_HPP_INCLUDED__

#include "irrlicht.h"

class Terrain;
class OwnShip;

class RadarCalculation
{
    public:
        RadarCalculation();
        virtual ~RadarCalculation();
        //void loadRadarCalculation();
        void updateRadarCalculation(irr::video::IImage * radarImage, const Terrain& terrain, const OwnShip& ownShip);

    private:


};

#endif
