#include "irrlicht.h"

#include "RadarCalculation.hpp"

#include "Terrain.hpp"
#include "OwnShip.hpp"

using namespace irr;

RadarCalculation::RadarCalculation()
{

}

RadarCalculation::~RadarCalculation()
{
    //dtor
}

/*
void RadarCalculation::loadRadarCalculation()
{

}
*/

void RadarCalculation::updateRadarCalculation(irr::video::IImage * radarImage, const Terrain& terrain, const OwnShip& ownShip)
{
    radarImage->fill(video::SColor(255, 0, 0, 255));
    for(int i = 0;i<64;i++) { //FIXME: Hardcoded to work with 64px, should check size of the radarImage
        for(int j=0;j<64;j++) {
            core::vector3df position = ownShip.getPosition();
            f32 localPosX = (i-32)*50.0 + position.X; //Hardcoded to 50m/px
            f32 localPosZ = (32-j)*50.0 + position.Z;
            s32 pixelColour = terrain.getHeight(localPosX,localPosZ);
            if (pixelColour > 255)
                {pixelColour = 255;}
            if (pixelColour < 0)
                {pixelColour = 255;}
            pixelColour = 255-pixelColour;
            radarImage->setPixel(i,j,video::SColor(255,pixelColour,pixelColour,0));
        }
    }
    radarImage->setPixel(32,32,video::SColor(255,255,0,0)); //'centre' point
}
