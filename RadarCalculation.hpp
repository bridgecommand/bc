#ifndef __RADARCALCULATION_HPP_INCLUDED__
#define __RADARCALCULATION_HPP_INCLUDED__

#include "irrlicht.h"

#include <vector>

class Terrain;
class OwnShip;

class RadarCalculation
{
    public:
        RadarCalculation();
        virtual ~RadarCalculation();
        //void loadRadarCalculation();
        void update(irr::video::IImage * radarImage, const Terrain& terrain, const OwnShip& ownShip);

    private:
        std::vector<std::vector<irr::f32> > scanArray;


};

#endif
