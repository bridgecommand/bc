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
    //Fill with background colour
    radarImage->fill(video::SColor(255, 0, 0, 255));

    //Get image size
    int bitmapWidth = radarImage->getDimension().Width;
    if (radarImage->getDimension().Height != bitmapWidth)
        {return;} //Check image is square, and return without action if not

    for(int i = 0;i<bitmapWidth;i++) { //FIXME: Hardcoded to work with 64px, should check size of the radarImage
        for(int j=0;j<bitmapWidth;j++) {
            core::vector3df position = ownShip.getPosition();
            f32 localPosX = ((float)i-(float)(bitmapWidth-1)/2.0)*50.0 + position.X; //Distance from ship (Hardcoded to 50m/px)
            f32 localPosZ = ((float)(bitmapWidth-1)/2.0-(float)j)*50.0 + position.Z;
            s32 pixelColour = terrain.getHeight(localPosX,localPosZ);
            if (pixelColour > 255)
                {pixelColour = 255;}
            if (pixelColour < 0)
                {pixelColour = 255;}
            pixelColour = 255-pixelColour;
            radarImage->setPixel(i,j,video::SColor(255,pixelColour,pixelColour,0));
        }
    }
    radarImage->setPixel(bitmapWidth/2,bitmapWidth/2,video::SColor(255,255,0,0)); //'centre' point //Fixme: Remove this from final, as it isn't exactly in centre
}
