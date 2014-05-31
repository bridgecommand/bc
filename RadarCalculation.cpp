#include "irrlicht.h"

#include "RadarCalculation.hpp"

#include "Terrain.hpp"
#include "OwnShip.hpp"
#include "Buoys.hpp"
#include "OtherShips.hpp"
#include "RadarData.hpp"

#include <iostream>
#include <cmath>

using namespace irr;

RadarCalculation::RadarCalculation()
{
    //initialise scanArray size (360x64 points per scan)
    u32 rows = 360;
    u32 columns = 64; //Fixme: Hardcoding
    scanArray.resize(rows,std::vector<f32>(columns,0.0));

    currentScanAngle=0;
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

void RadarCalculation::update(irr::video::IImage * radarImage, const Terrain& terrain, const OwnShip& ownShip, const Buoys& buoys, const OtherShips& otherShips)
{

    core::vector3df position = ownShip.getPosition();

    //Load radar data for other contacts
    std::vector<RadarData> radarData;
    //For other ships
    for (std::vector<RadarData>::size_type contactID=1; contactID<=otherShips.getNumber(); contactID++) {
        radarData.push_back(otherShips.getRadarData(contactID,position));
    }
    //For buoys
    for (std::vector<RadarData>::size_type contactID=1; contactID<=buoys.getNumber(); contactID++) {
        radarData.push_back(buoys.getRadarData(contactID,position));
    }

    irr::u32 scansPerLoop = 3; //Fixme: Change this to get a constant configurable scan rate (within reason)
    for(int i = 0; i<scansPerLoop;i++) { //Start of repeatable scan section
        f32 scanSlope = 0.0; //Slope at start of scan
        f32 cellLength = 25; //Fixme: This needs to change with radar range
        for (int currentStep = 1; currentStep<64; currentStep++) { //Fixme: hardcoding
            //scan into array, accessed as  scanArray[row (angle)][column (step)]

            f32 relX = cellLength*currentStep*sin(currentScanAngle*core::DEGTORAD); //Distance from ship
            f32 relZ = cellLength*currentStep*cos(currentScanAngle*core::DEGTORAD);
            f32 localX = position.X + relX;
            f32 localZ = position.Z + relZ;

            f32 localSlope = terrain.getHeight(localX,localZ)/(cellLength*currentStep);
            f32 slopeChange = localSlope - scanSlope;

            //Fixme: also check other contacts here (ships,buoys)
            //For each buoy/own ship, we need to know:
            //Position (relative), length, heading.

            if (slopeChange>0) {
                scanSlope = localSlope; //Highest so far on scan
                scanArray[currentScanAngle][currentStep] = 1.0;
            } else {
                scanArray[currentScanAngle][currentStep] = 0.0;
            }

            //Fixme: trial implementation
            for(std::vector<RadarData>::iterator it = radarData.begin(); it != radarData.end(); ++it) {
                if( std::abs(it->relX-relX)<50.0 && std::abs(it->relZ-relZ)<50.0 ) {scanArray[currentScanAngle][currentStep] = 1.0;}
            }

            //Debugging simple version (for checking terrain height calc)
            //if (terrain.getHeight(localPosX,localPosZ)>0) { scanArray[currentScanAngle][currentStep] = 1.0;} else {scanArray[currentScanAngle][currentStep] = 0.0;}
        } //End of for loop scanning out

        //Increment scan angle for next time
        currentScanAngle += 1; //Fixme: Hardcoding for scan angle
        if (currentScanAngle >=360) {
            currentScanAngle -= 360;
        }
    } //End of repeatable scan section

    //*************************
    //generate image from array
    //*************************

    //Fill with background colour
    radarImage->fill(video::SColor(255, 0, 0, 255));

    //Get image size
    int bitmapWidth = radarImage->getDimension().Width;
    if (radarImage->getDimension().Height != bitmapWidth)
        {return;} //Check image is square, and return without action if not

    //draw from array to image
    for (int scanAngle = 0; scanAngle <360; scanAngle+=1) {
        for (int currentStep = 1; currentStep<64; currentStep++) { //Fixme: hardcoding

            s32 pixelX = (float)(bitmapWidth)/2.0 + currentStep*sin(scanAngle*core::DEGTORAD); //Fixme: This isn't exactly correct - think about centre location.
            s32 pixelY = (float)(bitmapWidth)/2.0 - currentStep*cos(scanAngle*core::DEGTORAD); //Fixme: This isn't exactly correct

            if (pixelX >= 0 && pixelX < bitmapWidth && pixelY >= 0 && pixelY < bitmapWidth ) {

                s32 pixelColour=255*scanArray[scanAngle][currentStep];

                radarImage->setPixel(pixelX,pixelY,video::SColor(255,pixelColour,pixelColour,0));
            }
        }
    }
}
