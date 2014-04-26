#include "irrlicht.h"

#include "RadarCalculation.hpp"

#include "Terrain.hpp"

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

void RadarCalculation::updateRadarCalculation(irr::video::IImage * radarImage, Terrain& terrain, irr::f32 x, irr::f32 z)//FIXME: Should be const Terrain& to ensure we don't change terrain
{
    radarImage->fill(video::SColor(255, 0, 0, 255));
    for(int i = 0;i<64;i++) { //FIXME: Hardcoded to work with 64px, should check size of the radarImage
        for(int j=0;j<64;j++) {
            f32 localPosX = (i-32)*30.0 + x;
            f32 localPosZ = (32-j)*30.0 + z;
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
