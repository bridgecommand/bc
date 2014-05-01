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

    //Scan out
    for (int scanAngle = 0; scanAngle <360; scanAngle+=1) {
        f32 scanSlope = 0.0; //Slope at start of scan
        f32 cellLength = 50;
        for (int currentStep = 1; currentStep<bitmapWidth/2; currentStep++) {
            core::vector3df position = ownShip.getPosition();
            f32 localPosX = position.X + cellLength*currentStep*sin(scanAngle*core::DEGTORAD); //Distance from ship
            f32 localPosZ = position.Z + cellLength*currentStep*cos(scanAngle*core::DEGTORAD);
            s32 pixelX = (float)(bitmapWidth)/2.0 + currentStep*sin(scanAngle*core::DEGTORAD);
            s32 pixelY = (float)(bitmapWidth)/2.0 - currentStep*cos(scanAngle*core::DEGTORAD);

            f32 localSlope = terrain.getHeight(localPosX,localPosZ)/(cellLength*currentStep);
            f32 slopeChange = localSlope - scanSlope;
            if (slopeChange>0) {
                scanSlope = localSlope; //Highest so far on scan
            }

            if (pixelX >= 0 && pixelX < bitmapWidth && pixelY >= 0 && pixelY < bitmapWidth ) {

                s32 pixelColour=0;

                if (slopeChange > 0) {
                    pixelColour = 255;
                }

                radarImage->setPixel(pixelX,pixelY,video::SColor(255,pixelColour,pixelColour,0));
            }

        }
    }

    radarImage->setPixel(bitmapWidth/2,bitmapWidth/2,video::SColor(255,255,0,0)); //'centre' point //Fixme: Remove this from final, as it isn't exactly in centre
}
