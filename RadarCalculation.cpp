#include "irrlicht.h"

#include "RadarCalculation.hpp"

#include "Terrain.hpp"
#include "OwnShip.hpp"
#include "Buoys.hpp"
#include "OtherShips.hpp"
#include "RadarData.hpp"
#include "Angles.hpp"

#include <iostream>
#include <cmath>

using namespace irr;

RadarCalculation::RadarCalculation()
{
    //initialise scanArray size (360 x rangeResolution points per scan)
    rangeResolution = 64;
    scanArray.resize(360,std::vector<f32>(rangeResolution,0.0));

    currentScanAngle=0;
    scanAngleStep=3;
}

RadarCalculation::~RadarCalculation()
{
    //dtor
}

void RadarCalculation::update(irr::video::IImage * radarImage, const Terrain& terrain, const OwnShip& ownShip, const Buoys& buoys, const OtherShips& otherShips)
{
    scan(terrain, ownShip, buoys, otherShips); // scan into scanArray[row (angle)][column (step)]
    render(radarImage); //From scanArray[row (angle)][column (step)], render to radarImage
}

void RadarCalculation::scan(const Terrain& terrain, const OwnShip& ownShip, const Buoys& buoys, const OtherShips& otherShips)
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
        for (int currentStep = 1; currentStep<rangeResolution; currentStep++) { //Fixme: hardcoding
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
        currentScanAngle += scanAngleStep;
        if (currentScanAngle>=360) {
            currentScanAngle=0;
        }
    } //End of repeatable scan section
}

void RadarCalculation::render(irr::video::IImage * radarImage)
{
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
    f32 centrePixel = ((float)bitmapWidth-1.0)/2.0; //The centre of the bitmap. Normally this will be a fractional number (##.5)
    for (int scanAngle = 0; scanAngle <360; scanAngle+=scanAngleStep) {
        for (int currentStep = 1; currentStep<rangeResolution; currentStep++) {

            irr::f32 cellMinAngle = scanAngle - (float)scanAngleStep/2.0;
            irr::f32 cellMaxAngle = scanAngle + (float)scanAngleStep/2.0;
            irr::f32 cellMinRange = (((float)currentStep-0.5)*((float)bitmapWidth*0.5/(float)rangeResolution));//Range in pixels from centre
            irr::f32 cellMaxRange = (((float)currentStep+0.5)*((float)bitmapWidth*0.5/(float)rangeResolution));//Fixme: Check rounding etc

            u32 pixelColour=255*scanArray[scanAngle][currentStep];
            if (pixelColour>255) {pixelColour = 255;}

            drawSector(radarImage,centrePixel,centrePixel,cellMinRange,cellMaxRange,cellMinAngle,cellMaxAngle,255,pixelColour,pixelColour,0);

        }
    }
}

void RadarCalculation::drawSector(irr::video::IImage * radarImage,irr::f32 centreX, irr::f32 centreY, irr::f32 innerRadius, irr::f32 outerRadius, irr::f32 startAngle, irr::f32 endAngle, irr::u32 alpha, irr::u32 red, irr::u32 green, irr::u32 blue)
//draw a bounded sector
{
    //find the corner points (Fixme: Not quite right when the extreme point is on the outer curve)
    irr::f32 point1X = centreX + std::sin(irr::core::DEGTORAD*startAngle)*innerRadius;
    irr::f32 point1Y = centreY - std::cos(irr::core::DEGTORAD*startAngle)*innerRadius;
    irr::f32 point2X = centreX + std::sin(irr::core::DEGTORAD*startAngle)*outerRadius;
    irr::f32 point2Y = centreY - std::cos(irr::core::DEGTORAD*startAngle)*outerRadius;
    irr::f32 point3X = centreX + std::sin(irr::core::DEGTORAD*endAngle)*outerRadius;
    irr::f32 point3Y = centreY - std::cos(irr::core::DEGTORAD*endAngle)*outerRadius;

    irr::f32 point4X = centreX + std::sin(irr::core::DEGTORAD*endAngle)*innerRadius;
    irr::f32 point4Y = centreY - std::cos(irr::core::DEGTORAD*endAngle)*innerRadius;

    //find the 'bounding box'
    irr::f32 minX = std::min(std::min(point1X,point2X),std::min(point3X,point4X));
    irr::f32 maxX = std::max(std::max(point1X,point2X),std::max(point3X,point4X));
    irr::f32 minY = std::min(std::min(point1Y,point2Y),std::min(point3Y,point4Y));
    irr::f32 maxY = std::max(std::max(point1Y,point2Y),std::max(point3Y,point4Y));

    irr::f32 innerRadiusSqr = std::pow(innerRadius,2);
    irr::f32 outerRadiusSqr = std::pow(outerRadius,2);

    //draw the points
    for (int i = minX;i<=maxX;i++) {
        for (int j = minY;j<=maxY;j++) {

            irr::f32 localX = i - centreX; //position referred to centre
            irr::f32 localY = j - centreY; //position referred to centre

            irr::f32 localRadiusSqr = std::pow(localX,2) + std::pow(localY,2); //check radius of points
            irr::f32 localAngle = irr::core::RADTODEG*std::atan2(localX,-1*localY); //check angle of point

            //if the point is within the limits, plot it
            if (localRadiusSqr >= innerRadiusSqr && localRadiusSqr <= outerRadiusSqr) {
                if (Angles::isAngleBetween(localAngle,startAngle,endAngle)) {
                    //Plot i,j
                    radarImage->setPixel(i,j,video::SColor(alpha,red,green,blue));
                }
            }
        }
    }
}

irr::s32 RadarCalculation::round(irr::f32 numberIn)
{
    irr::s32 result = numberIn + 0.5; //Fixme: Check for negative numbers!
    return result;
}
