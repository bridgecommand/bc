/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2014 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY Or FITNESS For A PARTICULAR PURPOSE.  See the
     GNU General Public License For more details.

     You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#include "RadarCalculation.hpp"

#include "Terrain.hpp"
#include "OwnShip.hpp"
#include "Buoys.hpp"
#include "OtherShips.hpp"
#include "RadarData.hpp"
#include "Angles.hpp"
#include "Constants.hpp"
#include "IniFile.hpp"
#include "NumberToImage.hpp"
#include "Utilities.hpp"

#include <iostream>
#include <cmath>
#include <cstdlib> //For rand()
#include <algorithm> //For sort()

using namespace irr;

RadarCalculation::RadarCalculation() : rangeResolution(64)
{
    //Initial values for controls, all 0-100:
    radarGain = 50;
    radarRainClutterReduction=0;
    radarSeaClutterReduction=0;

    EBLRangeNm=0;
    EBLBrg=0;

    EBLLastUpdated = clock();

    //Radar modes: North up (false, false). Course up (true, true). Head up (true, false)
    headUp = false;
    stabilised = false;
    trueVectors = true;
    vectorLengthMinutes = 6;
    arpaOn = false;
    largestARPADisplayId = 0;

    //initialise scanArray size (360 x rangeResolution points per scan)
    //rangeResolution = 64; now set initialiser list
    scanArray.resize(360,std::vector<f32>(rangeResolution,0.0));
    scanArrayAmplified.resize(360,std::vector<f32>(rangeResolution,0.0));
    scanArrayAmplifiedPrevious.resize(360,std::vector<f32>(rangeResolution,0.0));

    //initialise arrays
    for(u32 i = 0; i<360; i++) {
        for(u32 j = 0; j<rangeResolution; j++) {
            scanArray[i][j] = 0.0;
            scanArrayAmplified[i][j] = 0.0;
        }
    }

    //Hard coded in GUI and here for 10 parallel index lines
    for(u32 i=0; i<10; i++) {
        piBearings.push_back(0.0);
        piRanges.push_back(0.0);
    }

    radarScreenStale = true;
    radarRadiusPx = 10; //Set to an arbitrary value initially, will be set later.

    currentScanAngle=0;
}

RadarCalculation::~RadarCalculation()
{
    //dtor
}

void RadarCalculation::load(std::string radarConfigFile, irr::IrrlichtDevice* dev)
{
    device = dev;

    //Load parameters from the radarConfig file (if it exists)
    irr::u32 numberOfRadarRanges = IniFile::iniFileTou32(radarConfigFile,"NumberOfRadarRanges");
    if (numberOfRadarRanges==0) {
        //Assume file doesn't exist, and load defaults

        //Set radar ranges:
        radarRangeNm.push_back(0.5);
        radarRangeNm.push_back(1.0);
        radarRangeNm.push_back(1.5);
        radarRangeNm.push_back(3.0);
        radarRangeNm.push_back(6.0);
        radarRangeNm.push_back(12.0);
        radarRangeNm.push_back(24.0);
        //Initial radar range
        radarRangeIndex=3;

        scanAngleStep=2; //Radar angular resolution (integer degree)
        radarScannerHeight = 2.0;
        radarNoiseLevel = 0.000000000005;
        radarSeaClutter = 0.000000001;
        radarRainClutter = 0.00001;

        rangeSensitivity = 20;

        radarBackgroundColour.setAlpha(255);
        radarBackgroundColour.setRed(0);
        radarBackgroundColour.setGreen(0);
        radarBackgroundColour.setBlue(200);

        radarForegroundColour.setAlpha(255);
        radarForegroundColour.setRed(255);
        radarForegroundColour.setGreen(220);
        radarForegroundColour.setBlue(0);

    } else {
        //Load from file, but check plausibility where required

        //Use numberOfRadarRanges, which we know is at least 1, to fill the radarRangeNm vector
        for (unsigned int i = 1; i <= numberOfRadarRanges; i++) {
            irr::f32 thisRadarRange = IniFile::iniFileTof32(radarConfigFile,IniFile::enumerate1("RadarRange",i));
            if (thisRadarRange<=0) {thisRadarRange = 1;} //Check value is reasonable
            radarRangeNm.push_back(thisRadarRange);
        }
        std::sort(radarRangeNm.begin(), radarRangeNm.end());

        //Initial radar range
        radarRangeIndex=numberOfRadarRanges/2;

        //Radar angular resolution (integer degree)
        scanAngleStep=IniFile::iniFileTou32(radarConfigFile,"radar_sensitivity");
        if (scanAngleStep < 1 || scanAngleStep > 180) {scanAngleStep = 2;}

        //Radar scanner height (Metres)
        radarScannerHeight = IniFile::iniFileTof32(radarConfigFile,"radar_height");

        //Noise parameters
        radarNoiseLevel = IniFile::iniFileTof32(radarConfigFile,"radar_noise");
        radarSeaClutter = IniFile::iniFileTof32(radarConfigFile,"radar_sea_clutter");
        radarRainClutter=IniFile::iniFileTof32(radarConfigFile,"radar_rain_clutter");
        rangeSensitivity = IniFile::iniFileTof32(radarConfigFile,"radar_range_sensitivity");
        if (radarNoiseLevel < 0) {radarNoiseLevel = 0.000000000005;}
        if (radarSeaClutter < 0) {radarSeaClutter = 0.000000001;}
        if (radarRainClutter< 0) {radarRainClutter= 0.00001;}
        if (rangeSensitivity< 0) {rangeSensitivity=20;}
        radarBackgroundColour.setAlpha(255);
        radarBackgroundColour.setRed(IniFile::iniFileTou32(radarConfigFile,"radar_bg_red"));
        radarBackgroundColour.setGreen(IniFile::iniFileTou32(radarConfigFile,"radar_bg_green"));
        radarBackgroundColour.setBlue(IniFile::iniFileTou32(radarConfigFile,"radar_bg_blue"));

        radarForegroundColour.setAlpha(255);
        radarForegroundColour.setRed(IniFile::iniFileTou32(radarConfigFile,"radar1_red"));
        radarForegroundColour.setGreen(IniFile::iniFileTou32(radarConfigFile,"radar1_green"));
        radarForegroundColour.setBlue(IniFile::iniFileTou32(radarConfigFile,"radar1_blue"));

    }
}

void RadarCalculation::decreaseRange()
{
    if (radarRangeIndex>0) {
        radarRangeIndex--;
    }
}

void RadarCalculation::increaseRange()
{
    if (radarRangeIndex<radarRangeNm.size()-1) {
        radarRangeIndex++;
    }
}

irr::f32 RadarCalculation::getRangeNm() const
{
    return radarRangeNm.at(radarRangeIndex); //Assume that radarRangeIndex is in bounds
}

void RadarCalculation::setGain(irr::f32 value)
{
    radarGain = value;
}

void RadarCalculation::setClutter(irr::f32 value)
{
    radarSeaClutterReduction = value;
}

void RadarCalculation::setRainClutter(irr::f32 value)
{
    radarRainClutterReduction = value;
}

irr::f32 RadarCalculation::getGain() const
{
    return radarGain;
}

irr::f32 RadarCalculation::getClutter() const
{
    return radarSeaClutterReduction;
}

irr::f32 RadarCalculation::getRainClutter() const
{
    return radarRainClutterReduction;
}

irr::f32 RadarCalculation::getEBLRangeNm() const
{
    return EBLRangeNm;
}

irr::f32 RadarCalculation::getEBLBrg() const
{
    return EBLBrg;
}

void RadarCalculation::setPIData(irr::s32 PIid, irr::f32 PIbearing, irr::f32 PIrange)
{
    if (PIid >= 0 && PIid < (s32)piBearings.size() && PIid < (s32)piRanges.size()) {
        piBearings.at(PIid) = PIbearing;
        piRanges.at(PIid) = PIrange;
    }
}

irr::f32 RadarCalculation::getPIbearing(irr::s32 PIid) const
{
    if (PIid >= 0 && PIid < (s32)piBearings.size()) {
        return piBearings.at(PIid);
    } else {
        return 0;
    }
}

irr::f32 RadarCalculation::getPIrange(irr::s32 PIid) const
{
    if (PIid >= 0 && PIid < (s32)piRanges.size()) {
        return piRanges.at(PIid);
    } else {
        return 0;
    }
}

void RadarCalculation::increaseEBLRange()
{
    //Only trigger this if there's been enough time since the last update.
    clock_t clockNow = clock();
    float elapsed = (float)(clockNow - EBLLastUpdated)/CLOCKS_PER_SEC;
    if (elapsed > 0.03) {
        EBLLastUpdated = clockNow;

        EBLRangeNm += getRangeNm()/100;

    }
}

void RadarCalculation::decreaseEBLRange()
{
    //Only trigger this if there's been enough time since the last update.
    clock_t clockNow = clock();
    float elapsed = (float)(clockNow - EBLLastUpdated)/CLOCKS_PER_SEC;
    if (elapsed > 0.03) {
        EBLLastUpdated = clockNow;

        EBLRangeNm -= getRangeNm()/100;
        if (EBLRangeNm<0) {
            EBLRangeNm = 0;
        }
    }
}

void RadarCalculation::increaseEBLBrg()
{
    //Only trigger this if there's been enough time since the last update.
    clock_t clockNow = clock();
    float elapsed = (float)(clockNow - EBLLastUpdated)/CLOCKS_PER_SEC;
    if (elapsed > 0.03) {
        EBLLastUpdated = clockNow;

        EBLBrg++;
        if (EBLBrg >= 360) {
            EBLBrg -= 360;
        }
    }
}

void RadarCalculation::decreaseEBLBrg()
{
    //Only trigger this if there's been enough time since the last update.
    clock_t clockNow = clock();
    float elapsed = (float)(clockNow - EBLLastUpdated)/CLOCKS_PER_SEC;
    if (elapsed > 0.03) {
        EBLLastUpdated = clockNow;

        EBLBrg--;
        if (EBLBrg < 0) {
            EBLBrg += 360;
        }
    }
}

void RadarCalculation::setNorthUp()
{
    //Radar modes: North up (false, false). Course up (true, true). Head up (true, false)
    headUp = false;
    stabilised = false;
}

void RadarCalculation::setCourseUp()
{
    //Radar modes: North up (false, false). Course up (true, true). Head up (true, false)
    headUp = true;
    stabilised = true;
}

void RadarCalculation::setHeadUp()
{
    //Radar modes: North up (false, false). Course up (true, true). Head up (true, false)
    headUp = true;
    stabilised = false;
}

bool RadarCalculation::getHeadUp() const//Head or course up
{
    return headUp;
}

void RadarCalculation::setArpaOn(bool on)
{
    arpaOn = on;
    if (!arpaOn) {
        //Clear arpa scans
        arpaContacts.clear();
        largestARPADisplayId = 0;
    }
}

void RadarCalculation::setRadarARPARel()
{
    trueVectors = false;
}

void RadarCalculation::setRadarARPATrue()
{
    trueVectors = true;
}

void RadarCalculation::setRadarARPAVectors(irr::f32 vectorMinutes)
{
    vectorLengthMinutes = vectorMinutes;
}

void RadarCalculation::setRadarDisplayRadius(u32 radiusPx)
{
    if (radarRadiusPx != radiusPx) { //If changed
        radarRadiusPx = radiusPx;
        radarScreenStale=true;
    }
}

irr::u32 RadarCalculation::getARPAContacts() const
{
    //Get number of ARPA contacts with a user display ID
    return largestARPADisplayId;
}

irr::f32 RadarCalculation::getARPACPA(irr::u32 contactID) const
{
    //Get information for a contact by its user display ID (if it exists), in Nm
    for (unsigned int i = 0; i<arpaContacts.size(); i++) {
        if (arpaContacts.at(i).estimate.displayID == contactID) {
            return arpaContacts.at(i).estimate.cpa;
        }
    }
    return NAN; //If nothing found
}

irr::f32 RadarCalculation::getARPATCPA(irr::u32 contactID) const
{
    //Get information for a contact by its user display ID (if it exists), in minutes
    for (unsigned int i = 0; i<arpaContacts.size(); i++) {
        if (arpaContacts.at(i).estimate.displayID == contactID) {
            return arpaContacts.at(i).estimate.tcpa;
        }
    }
    return NAN; //If nothing found
}

void RadarCalculation::update(irr::video::IImage * radarImage, irr::video::IImage * radarImageOverlaid, irr::core::vector3d<int64_t> offsetPosition, const Terrain& terrain, const OwnShip& ownShip, const Buoys& buoys, const OtherShips& otherShips, irr::f32 weather, irr::f32 rain, irr::f32 tideHeight, irr::f32 deltaTime, uint64_t absoluteTime, irr::core::vector2di mouseRelPosition, bool isMouseDown)
{

    //Reset screen if needed
    if(radarScreenStale) {
        radarImage->fill(video::SColor(255, 128, 128, 128)); //Fill with background colour
        //Reset 'previous' array so it will all get re-drawn
        for(u32 i = 0; i<360; i++) {
            for(u32 j = 0; j<rangeResolution; j++) {
                scanArrayAmplifiedPrevious[i][j] = -1.0;
            }
        }
        radarScreenStale = false;
    }

    //Find position of mouse cursor
    f32 cursorRangeXNm = (f32)mouseRelPosition.X/(f32)radarRadiusPx*radarRangeNm.at(radarRangeIndex);//Nm
    f32 cursorRangeYNm = -1.0*(f32)mouseRelPosition.Y/(f32)radarRadiusPx*radarRangeNm.at(radarRangeIndex);//Nm
    //Check if clicked and in range
    if (isMouseDown && pow(pow(cursorRangeXNm,2)+pow(cursorRangeYNm,2),0.5) <= radarRangeNm.at(radarRangeIndex) ) {
        std::cout << "Cursor E/W: " << cursorRangeXNm << " N/S:" << cursorRangeYNm << std::endl;
    }

    scan(offsetPosition, terrain, ownShip, buoys, otherShips, weather, rain, tideHeight, deltaTime, absoluteTime); // scan into scanArray[row (angle)][column (step)], and with filtering and amplification into scanArrayAmplified[][]
    updateARPA(offsetPosition, ownShip, absoluteTime); //From data in arpaContacts, updated in scan()
    render(radarImage, radarImageOverlaid, ownShip.getHeading(), ownShip.getSpeed()); //From scanArrayAmplified[row (angle)][column (step)], render to radarImage
}


void RadarCalculation::scan(irr::core::vector3d<int64_t> offsetPosition, const Terrain& terrain, const OwnShip& ownShip, const Buoys& buoys, const OtherShips& otherShips, irr::f32 weather, irr::f32 rain, irr::f32 tideHeight, irr::f32 deltaTime, uint64_t absoluteTime)
{

    const irr::u32 SECONDS_BETWEEN_SCANS = 20;

    core::vector3df position = ownShip.getPosition();
    //Get absolute position relative to SW corner of world model
    core::vector3d<int64_t> absolutePosition = offsetPosition;
    absolutePosition.X += position.X;
    absolutePosition.Y += position.Y;
    absolutePosition.Z += position.Z;

    //Some tuning constants
    irr::f32 radarFactorLand=2.0;
    irr::f32 radarFactorVessel=0.0001;

    //Convert range to cell size
    irr::f32 cellLength = M_IN_NM*radarRangeNm.at(radarRangeIndex)/rangeResolution; ; //Assume that radarRangeIndex is in bounds

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

    const irr::f32 RADAR_RPM = 25; //Todo: Make a ship parameter
    const irr::f32 RPMtoDEGPERSECOND = 6;
    irr::u32 scansPerLoop = RADAR_RPM*RPMtoDEGPERSECOND*deltaTime/(irr::f32)scanAngleStep + (irr::f32)rand()/RAND_MAX ; //Add random value (0-1, mean 0.5), so with rounding, we get the correct radar speed, even though we can only do an integer number of scans

    if (scansPerLoop > 10) {scansPerLoop=10;} //Limit to reasonable bounds
    for(u32 i = 0; i<scansPerLoop;i++) { //Start of repeatable scan section
        f32 scanSlope = -0.5; //Slope at start of scan (in metres/metre) - Make slightly negative so vessel contacts close in get detected
        for (u32 currentStep = 1; currentStep<rangeResolution; currentStep++) { //Note that currentStep starts as 1, not 0. This is used in anti-rain clutter filter, which checks element at currentStep-1
            //scan into array, accessed as  scanArray[row (angle)][column (step)]

            //Clear old value
            scanArray[currentScanAngle][currentStep] = 0.0;

            //Get location of area being scanned
            f32 localRange = cellLength*currentStep;
            f32 relX = localRange*sin(currentScanAngle*core::DEGTORAD); //Distance from ship
            f32 relZ = localRange*cos(currentScanAngle*core::DEGTORAD);
            f32 localX = position.X + relX;
            f32 localZ = position.Z + relZ;

            //get extents
            f32 minCellAngle = Angles::normaliseAngle(currentScanAngle - scanAngleStep/2.0);
            f32 maxCellAngle = Angles::normaliseAngle(currentScanAngle + scanAngleStep/2.0);
            f32 minCellRange = localRange - cellLength/2.0;
            f32 maxCellRange = localRange + cellLength/2.0;

            //get adjustment of height for earth's curvature
            f32 dropWithCurvature = std::pow(localRange,2)/(2*EARTH_RAD_M*EARTH_RAD_CORRECTION);

            //Calculate noise
            f32 localNoise = radarNoise(radarNoiseLevel,radarSeaClutter,radarRainClutter,weather,localRange,currentScanAngle,0,scanSlope,rain); //FIXME: Needs wind direction

            //Scan other contacts here
            for(unsigned int thisContact = 0; thisContact<radarData.size(); thisContact++) {
                f32 contactHeightAboveLine = (radarData.at(thisContact).height - radarScannerHeight - dropWithCurvature) - scanSlope*localRange;
                if (contactHeightAboveLine > 0) {
                    //Contact would be visible if in this cell. Check if it is
                    //Start of B3D code
                    if ((radarData.at(thisContact).range >= minCellRange && radarData.at(thisContact).range <= maxCellRange) || (radarData.at(thisContact).minRange >= minCellRange && radarData.at(thisContact).minRange <= maxCellRange) || (radarData.at(thisContact).maxRange >= minCellRange && radarData.at(thisContact).maxRange <= maxCellRange) || (radarData.at(thisContact).minRange < minCellRange && radarData.at(thisContact).maxRange > maxCellRange)) {//Check if centre of target within the cell. If not then check if Either min range or max range of contact is within the cell, or min and max span the cell
                        if ((Angles::isAngleBetween(radarData.at(thisContact).angle,minCellAngle,maxCellAngle)) || (Angles::isAngleBetween(radarData.at(thisContact).minAngle,minCellAngle,maxCellAngle)) || (Angles::isAngleBetween(radarData.at(thisContact).maxAngle,minCellAngle,maxCellAngle)) || ( Angles::normaliseAngle(radarData.at(thisContact).minAngle-minCellAngle) > 270 && Angles::normaliseAngle(radarData.at(thisContact).maxAngle-maxCellAngle) < 90)) {//Check if centre of target within the cell. If not then check if either min angle or max angle of contact is within the cell, or min and max span the cell

                            irr::f32 rangeAtCellMin = rangeAtAngle(minCellAngle,radarData.at(thisContact).relX,radarData.at(thisContact).relZ,radarData.at(thisContact).heading);
                            irr::f32 rangeAtCellMax = rangeAtAngle(maxCellAngle,radarData.at(thisContact).relX,radarData.at(thisContact).relZ,radarData.at(thisContact).heading);

                            //check if the contact intersects this exact cell, if its extremes overlap it
                            //Also check if the target centre is in the cell, or the extended target spans the cell (ie RangeAtCellMin less than minCellRange and rangeAtCellMax greater than maxCellRange and vice versa)
                            if ((((radarData.at(thisContact).range >= minCellRange && radarData.at(thisContact).range <= maxCellRange) && (Angles::isAngleBetween(radarData.at(thisContact).angle,minCellAngle,maxCellAngle))) || (rangeAtCellMin >= minCellRange && rangeAtCellMin <= maxCellRange) || (rangeAtCellMax >= minCellRange && rangeAtCellMax <= maxCellRange) || (rangeAtCellMin < minCellRange && rangeAtCellMax > maxCellRange) || (rangeAtCellMax < minCellRange && rangeAtCellMin > maxCellRange))){

                                irr::f32 radarEchoStrength = radarFactorVessel * std::pow(M_IN_NM/localRange,4) * radarData.at(thisContact).rcs;
                                scanArray[currentScanAngle][currentStep] += radarEchoStrength;

                                //Start ARPA section
                                if (arpaOn && radarEchoStrength*2 > localNoise) {
                                    //Contact is detectable in noise

                                    //Iterate through arpaContacts array, checking if this contact is in the list (by checking the if the 'contact' pointer is to the same underlying ship/buoy)
                                    int existingArpaContact=-1;
                                    for (unsigned int j = 0; j<arpaContacts.size(); j++) {
                                        if (arpaContacts.at(j).contact == radarData.at(thisContact).contact) {
                                            existingArpaContact = j;
                                        }
                                    }
                                    //If it doesn't exist, add it, and make existingArpaContact point to it
                                    if (existingArpaContact<0) {
                                        ARPAContact newContact;
                                        newContact.contact = radarData.at(thisContact).contact;
                                        newContact.contactType=CONTACT_NORMAL;
                                        //newContact.displayID = 0; //Initially not displayed
                                        newContact.totalXMovementEst = 0;
                                        newContact.totalZMovementEst = 0;

                                        //Zeros for estimated state
                                        newContact.estimate.displayID = 0;
                                        newContact.estimate.stationary = true;
                                        newContact.estimate.lost = false;
                                        newContact.estimate.absVectorX = 0;
                                        newContact.estimate.absVectorZ = 0;
                                        newContact.estimate.absHeading = 0;
                                        newContact.estimate.bearing = 0;
                                        newContact.estimate.range = 0;
                                        newContact.estimate.speed = 0;

                                        arpaContacts.push_back(newContact);
                                        existingArpaContact = arpaContacts.size()-1;
                                        //std::cout << "Adding contact " << existingArpaContact << std::endl;
                                    }
                                    //Add this scan (if not already scanned in the last X seconds
                                    size_t scansSize = arpaContacts.at(existingArpaContact).scans.size();
                                    if (scansSize==0 || absoluteTime > SECONDS_BETWEEN_SCANS + arpaContacts.at(existingArpaContact).scans.at(scansSize-1).timeStamp) {
                                        ARPAScan newScan;
                                        newScan.timeStamp = absoluteTime;

                                        //Add noise/uncertainty
                                        irr::f32 angleUncertainty = scanAngleStep/2.0 * (2.0*(irr::f32)rand()/RAND_MAX - 1);
                                        irr::f32 rangeUncertainty = rangeSensitivity * (2.0*(irr::f32)rand()/RAND_MAX - 1)/M_IN_NM;

                                        newScan.bearingDeg = angleUncertainty + radarData.at(thisContact).angle;
                                        newScan.rangeNm = rangeUncertainty + radarData.at(thisContact).range / M_IN_NM;

                                        newScan.x = absolutePosition.X + newScan.rangeNm*M_IN_NM * sin(newScan.bearingDeg*RAD_IN_DEG);
                                        newScan.z = absolutePosition.Z + newScan.rangeNm*M_IN_NM * cos(newScan.bearingDeg*RAD_IN_DEG);;
                                        newScan.estimatedRCS = 100;//Todo: Implement

                                        //Keep track of estimated total movement
                                        if (scansSize > 0 && arpaOn) {
                                            arpaContacts.at(existingArpaContact).totalXMovementEst += arpaContacts.at(existingArpaContact).scans.at(scansSize-1).x - newScan.x;
                                            arpaContacts.at(existingArpaContact).totalZMovementEst += arpaContacts.at(existingArpaContact).scans.at(scansSize-1).z - newScan.z;
                                        } else {
                                            arpaContacts.at(existingArpaContact).totalXMovementEst = 0;
                                            arpaContacts.at(existingArpaContact).totalZMovementEst = 0;
                                        }

                                        arpaContacts.at(existingArpaContact).scans.push_back(newScan);
                                        //std::cout << "ARPA update on " << existingArpaContact << std::endl;

                                    }


                                } //End ARPA Section
                                //Todo: Also check for contacts beyond the current scan range.

                                /*
                                ;check how visible against noise/clutter. If visible, record as detected for ARPA tracking
                                If radarEchoStrength#*2 > radarNoiseValueNoBlock(radarNoiseLevel#, radarSeaClutter#, radarRainClutter#, weather#, AllRadarTargets(i)\range, rainIntensity)

                                    ;DebugLog "Contact:"
                                    ;DebugLog Str(radarNoiseValueNoBlock(radarNoiseLevel#, radarSeaClutter#, radarRainClutter#, weather#, AllRadarTargets(i)\range, rainIntensity))
                                    ;DebugLog radarEchoStrength#*2

                                    contactLastDetected(i) = absolute_time
                                EndIf

                                RadarIntensity#(Int(radarBrg#),RadarCurrentStep) = RadarIntensity#(Int(radarBrg#),RadarCurrentStep) + radarEchoStrength# ;add target reflection to array

                                ;RACON code
                                ;make an echo line behind the contact
                                If AllRadarTargets(i)\racon <> ""

                                    If Float(time#+AllRadarTargets(i)\raconOffsetTime) Mod 60 <= RaconOnTime# ;Show for RaconOnTime# seconds per minute

                                        Local raconEchoStrength# = radarFactorRACON * (1852/radarRange#)^2;RACON/SART goes with inverse square law as we are receiving the direct signal, not echo

                                        ;set start point for racon echo (global variable)
                                        raconCurrentStep = RadarCurrentStep

                                        addRaconString(raconEchoStrength, radarBrg#, 750, radarStep#, AllRadarTargets(i)\racon$)

                                    EndIf

                                EndIf
                                */
                                //if a target entirely covers the angle of a cell, then use its blocking height and increase radarHeight, so it blocks reflections from behind
                                if ( Angles::normaliseAngle(radarData.at(thisContact).minAngle-minCellAngle) > 270 && Angles::normaliseAngle(radarData.at(thisContact).maxAngle-maxCellAngle) < 90) {
                                    //reset scanSlope to new value if the solid height is higher
                                    scanSlope = std::max(scanSlope,(radarData.at(thisContact).solidHeight-radarScannerHeight-dropWithCurvature)/localRange);

                                }
                            }


                        }
                    }
                    //End of B3D code
                }
            }

            //Add land scan
            f32 radarHeight = terrain.getHeight(localX,localZ) - dropWithCurvature - radarScannerHeight - tideHeight;
            f32 localSlope = radarHeight/localRange;
            f32 heightAboveLine = radarHeight - scanSlope*localRange; //Find height above previous maximum scan slope

            if (heightAboveLine>0 && localSlope>0) {
                f32 radarLocalGradient = heightAboveLine/cellLength;
                scanSlope = localSlope; //Highest so far on scan
                scanArray[currentScanAngle][currentStep] += radarFactorLand*std::atan(radarLocalGradient)*(2/PI)/std::pow(localRange/M_IN_NM,3); //make a reflection off a plane wall at 1nm have a magnitude of 1*radarFactorLand
            }

            //Add radar noise
            scanArray[currentScanAngle][currentStep] += localNoise;

            //Do amplification: scanArrayAmplified between 0 and 1 will set displayed intensity, values above 1 will be limited at max intensity

            //Calculate from parameters
            //localRange is range in metres
            irr::f32 rainFilter = pow(radarRainClutterReduction/100.0,0.1);
            irr::f32 maxSTCdistance = 8*M_IN_NM*radarSeaClutterReduction/100.0; //This sets the distance at which the swept gain control becomes 1, and is 8Nm at full reduction
            irr::f32 radarSTCGain;
            if(maxSTCdistance>0) {
                radarSTCGain = pow(localRange/maxSTCdistance,3);
                if (radarSTCGain > 1) {radarSTCGain=1;} //Gain should never be increased (above 1.0)
            } else {
                radarSTCGain = 1;
            }

            //calculate high pass filter
            irr::f32 intensityGradient = scanArray[currentScanAngle][currentStep] - scanArray[currentScanAngle][currentStep-1];
            if (intensityGradient<0) {intensityGradient=0;}

            irr::f32 filteredSignal = intensityGradient*rainFilter + scanArray[currentScanAngle][currentStep]*(1-rainFilter);
            irr::f32 radarLocalGain = 500000*(8*pow(radarGain/100.0,4)) * radarSTCGain ;

            //take log (natural) of signal
            scanArrayAmplified[currentScanAngle][currentStep] = log(filteredSignal*radarLocalGain);

        } //End of for loop scanning out

        //Increment scan angle for next time
        currentScanAngle += scanAngleStep;
        if (currentScanAngle>=360) {
            currentScanAngle=0;
        }
    } //End of repeatable scan section

}

void RadarCalculation::updateARPA(irr::core::vector3d<int64_t> offsetPosition, const OwnShip& ownShip, uint64_t absoluteTime)
{

    //Own ship absolute position
    core::vector3df position = ownShip.getPosition();
    //Get absolute position relative to SW corner of world model
    core::vector3d<int64_t> absolutePosition = offsetPosition;
    absolutePosition.X += position.X;
    absolutePosition.Y += position.Y;
    absolutePosition.Z += position.Z;

    //Based on scans data in arpaContacts, estimate current speed, heading and position
    for (unsigned int i = 0; i<arpaContacts.size(); i++) {

        if (!arpaOn) {
            //Set all contacts to zero (untracked)
            arpaContacts.at(i).estimate.displayID = 0;
            arpaContacts.at(i).estimate.stationary = true;
            arpaContacts.at(i).estimate.lost = false;
            arpaContacts.at(i).estimate.absVectorX = 0;
            arpaContacts.at(i).estimate.absVectorZ = 0;
            arpaContacts.at(i).estimate.absHeading = 0;
            arpaContacts.at(i).estimate.bearing = 0;
            arpaContacts.at(i).estimate.range = 0;
            arpaContacts.at(i).estimate.speed = 0;
        } else {
            //Check there are at least two scans, so we can estimate behaviour
            if (arpaContacts.at(i).scans.size() > 1) {

                //Check stationary contacts to see if they've got detectable motion: TODO: Make this better: Should weight based on current range?
                //TODO: Test this weighting
                if (arpaContacts.at(i).estimate.stationary) {
                    f32 latestRangeNm =  arpaContacts.at(i).scans.back().rangeNm;
                    if (latestRangeNm < 1) {
                        latestRangeNm = 1;
                    }
                    f32 weightedMotionX = fabs(arpaContacts.at(i).totalXMovementEst/latestRangeNm);
                    f32 weightedMotionZ = fabs(arpaContacts.at(i).totalZMovementEst/latestRangeNm);
                    if (weightedMotionX >= 100 || weightedMotionZ >= 100) {
                        arpaContacts.at(i).estimate.stationary = false;
                    }
                }

                //Check if contact lost, if last scanned more than 60 seconds ago
                if ( absoluteTime - arpaContacts.at(i).scans.back().timeStamp > 60) {
                    arpaContacts.at(i).estimate.lost=true;
                    //std::cout << "Contact " << i << " lost" << std::endl;
                } else if (!arpaContacts.at(i).estimate.stationary) {
                    /* Update contact tracking: Initially based on latest scan, and 6 scans back if available, or earliest otherwise
                    TODO: Improve the logic of this, probably getting longest time possible before the behaviour was significantly
                    different */

                    //If ID is 0 (unassigned), set id and increment
                    if (arpaContacts.at(i).estimate.displayID==0) {
                        arpaContacts.at(i).estimate.displayID = ++largestARPADisplayId;
                    }

                    s32 currentScanIndex = arpaContacts.at(i).scans.size() - 1;
                    s32 referenceScanIndex = currentScanIndex - 6;
                    if (referenceScanIndex < 0) {
                        referenceScanIndex = 0;
                    }

                    ARPAScan currentScanData = arpaContacts.at(i).scans.at(currentScanIndex);
                    ARPAScan referenceScanData = arpaContacts.at(i).scans.at(referenceScanIndex);

                    //Find difference in time, position x, position z
                    f32 deltaTime = currentScanData.timeStamp - referenceScanData.timeStamp;
                    if (deltaTime>0) {
                        f32 deltaX = currentScanData.x - referenceScanData.x;
                        f32 deltaZ = currentScanData.z - referenceScanData.z;

                        //Absolute vector
                        arpaContacts.at(i).estimate.absVectorX = deltaX/deltaTime; //m/s
                        arpaContacts.at(i).estimate.absVectorZ = deltaZ/deltaTime; //m/s
                        arpaContacts.at(i).estimate.absHeading = std::atan2(deltaX,deltaZ)/RAD_IN_DEG;
                        if (arpaContacts.at(i).estimate.absHeading < 0 ) {
                            arpaContacts.at(i).estimate.absHeading += 360;
                        }
                        //Relative vector:
                        arpaContacts.at(i).estimate.relVectorX = arpaContacts.at(i).estimate.absVectorX - ownShip.getSpeed() * sin((ownShip.getHeading())*core::DEGTORAD);
                        arpaContacts.at(i).estimate.relVectorZ = arpaContacts.at(i).estimate.absVectorZ - ownShip.getSpeed() * cos((ownShip.getHeading())*core::DEGTORAD); //ownShipSpeed in m/s
                        arpaContacts.at(i).estimate.relHeading = std::atan2(arpaContacts.at(i).estimate.relVectorX,arpaContacts.at(i).estimate.relVectorZ)/RAD_IN_DEG;
                        if (arpaContacts.at(i).estimate.relHeading < 0 ) {
                            arpaContacts.at(i).estimate.relHeading += 360;
                        }

                        //Estimated current position:
                        f32 relX = currentScanData.x - absolutePosition.X + arpaContacts.at(i).estimate.absVectorX * (absoluteTime - currentScanData.timeStamp);
                        f32 relZ = currentScanData.z - absolutePosition.Z + arpaContacts.at(i).estimate.absVectorZ * (absoluteTime - currentScanData.timeStamp);
                        arpaContacts.at(i).estimate.bearing = std::atan2(relX,relZ)/RAD_IN_DEG;
                        if (arpaContacts.at(i).estimate.bearing < 0 ) {
                            arpaContacts.at(i).estimate.bearing += 360;
                        }
                        arpaContacts.at(i).estimate.range =  std::sqrt(pow(relX,2)+pow(relZ,2))/M_IN_NM; //Nm
                        arpaContacts.at(i).estimate.speed = std::sqrt(pow(arpaContacts.at(i).estimate.absVectorX,2) + pow(arpaContacts.at(i).estimate.absVectorZ,2))*MPS_TO_KTS;

                        //TODO: CPA AND TCPA here: Need checking/testing
                        f32 contactRelAngle = arpaContacts.at(i).estimate.relHeading - (180+arpaContacts.at(i).estimate.bearing);
                        f32 contactRange = arpaContacts.at(i).estimate.range; //(Nm)
                        f32 relDistanceToCPA = contactRange * cos(contactRelAngle*RAD_IN_DEG); //Distance along the other ship's relative motion line
                        f32 relativeSpeed = std::sqrt(pow(arpaContacts.at(i).estimate.relVectorX,2) + pow(arpaContacts.at(i).estimate.relVectorZ,2))*MPS_TO_KTS;
                        if (fabs(relativeSpeed) < 0.001) {relativeSpeed = 0.001;} //Avoid division by zero

                        arpaContacts.at(i).estimate.cpa = contactRange * sin(contactRelAngle*RAD_IN_DEG);
                        arpaContacts.at(i).estimate.tcpa = 60*relDistanceToCPA/relativeSpeed; // (nm / (nm/hr)), so time in hours, converted to minutes
                        //std::cout << "Contact " << arpaContacts.at(i).estimate.displayID << " CPA: " <<  arpaContacts.at(i).estimate.cpa << " nm in " << arpaContacts.at(i).estimate.tcpa << " minutes" << std::endl;


                    } //If time between scans > 0
                } //Contact not lost
            } //If at least 2 scans
        } //If ARPA is on
    } //For loop through arpa contacts
}

void RadarCalculation::render(irr::video::IImage * radarImage, irr::video::IImage * radarImageOverlaid, irr::f32 ownShipHeading, irr::f32 ownShipSpeed)
{
    //*************************
    //generate image from array
    //*************************

    //Render background radar picture into radarImage, then copy to radarImageOverlaid and do any 2d drawing on top (so we don't have to redraw all pixels each time
    u32 bitmapWidth = radarRadiusPx*2; //Set width to use - to map GUI radar display diameter in screen pixels
    if (radarImage->getDimension().Width < bitmapWidth) //Check the image we're rendering into is big enough
        {return;}

    //draw from array to image
    f32 centrePixel = (bitmapWidth-1.0)/2.0; //The centre of the bitmap. Normally this will be a fractional number (##.5)

    //precalculate cell max/min range for speed outside nested loop
	std::vector<irr::f32> cellMinRange;
	std::vector<irr::f32> cellMaxRange;
	//irr::f32 cellMinRange [rangeResolution];
    //irr::f32 cellMaxRange [rangeResolution];
	cellMinRange.push_back(0);
	cellMaxRange.push_back(0);
    for (u32 currentStep = 1; currentStep<rangeResolution; currentStep++) { //Note that we start with the element at 1, so we've already pushed in a dummy entry at 0
        cellMinRange.push_back((currentStep-0.5)*(bitmapWidth*0.5/(float)rangeResolution));//Range in pixels from centre
        cellMaxRange.push_back((currentStep+0.5)*(bitmapWidth*0.5/(float)rangeResolution));
    }

    for (int scanAngle = 0; scanAngle <360; scanAngle+=scanAngleStep) {

        irr::f32 cellMinAngle = scanAngle - scanAngleStep/2.0;
        irr::f32 cellMaxAngle = scanAngle + scanAngleStep/2.0;

        for (u32 currentStep = 1; currentStep<rangeResolution; currentStep++) {

            //If the sector has changed, draw it. If we're stabilising the picture, need to re-draw all in case the ship's head has changed
            if(scanArrayAmplified[scanAngle][currentStep]!=scanArrayAmplifiedPrevious[scanAngle][currentStep] || stabilised)
            {

                f32 pixelColour=scanArrayAmplified[scanAngle][currentStep];

                if (pixelColour>1.0) {pixelColour = 1.0;}
                if (pixelColour<0)   {pixelColour =   0;}

                //Interpolate colour between foreground and background
                irr::video::SColor thisColour = radarForegroundColour.getInterpolated(radarBackgroundColour, pixelColour);

                drawSector(radarImage,centrePixel,centrePixel,cellMinRange[currentStep],cellMaxRange[currentStep],cellMinAngle,cellMaxAngle,thisColour.getAlpha(),thisColour.getRed(),thisColour.getGreen(),thisColour.getBlue(), ownShipHeading);

                //Store what we've just plotted, so we don't need to re-plot if unchanged
                scanArrayAmplifiedPrevious[scanAngle][currentStep]=scanArrayAmplified[scanAngle][currentStep];
            }
        }
    }

    //Copy image into overlaid
    radarImage->copyTo(radarImageOverlaid);

    //Adjust for head up/course up
    f32 radarOffsetAngle = 0;
    if (headUp) {
        radarOffsetAngle = -1*ownShipHeading;
    }

    //Draw parallel indexes on here
    if (piRanges.size() == piBearings.size()) {
        for(unsigned int i = 0; i< piRanges.size(); i++) {
            f32 thisPIrange = piRanges.at(i);
            f32 thisPIbrg = piBearings.at(i);
            if(fabs(thisPIrange) > 0.0001 && fabs(thisPIrange) < getRangeNm()) {
                //Not zero range or off screen

                f32 piRangePX = (f32)bitmapWidth/2.0 * thisPIrange / getRangeNm(); //Find range in Px

                //find sin and cos of PI angle (so we only need once)
                f32 sinPIbrg=sin(-1*(thisPIbrg + radarOffsetAngle)*RAD_IN_DEG);
				f32 cosPIbrg=cos(-1*(thisPIbrg + radarOffsetAngle)*RAD_IN_DEG);

				//find central point on line
                f32 x_a = -1*piRangePX * sin( (90-(-1*(thisPIbrg + radarOffsetAngle)))*RAD_IN_DEG ) + centrePixel;
                f32 z_a =    piRangePX * cos( (90-(-1*(thisPIbrg + radarOffsetAngle)))*RAD_IN_DEG ) + centrePixel;

                //find half chord length (length of PI line)
                f32 halfChord = pow(pow((f32)bitmapWidth/2.0,2) - pow(piRangePX,2),0.5); //already checked that PIRange is smaller, so should be valid

                //calculate end points of line
                f32 x_1=x_a - halfChord * sinPIbrg;
                f32 z_1=z_a - halfChord * cosPIbrg;
                f32 x_2=x_a + halfChord * sinPIbrg;
				f32 z_2=z_a + halfChord * cosPIbrg;

				drawLine(radarImageOverlaid,x_1,z_1,x_2,z_2,255,255,255,255);

				//Show line number
				//Find point towards centre from the line
				f32 xDirection = centrePixel-x_a;
				f32 yDirection = centrePixel-z_a;
				if (x_a!=0 && z_a !=0) {

                    f32 mag = pow(pow(xDirection,2)+pow(yDirection,2),0.5);
                    xDirection/=mag;
                    yDirection/=mag;

                    s32 xTextPos = x_a + 15*xDirection;
                    s32 yTextPos = z_a + 15*yDirection;

                    video::IImage* idNumberImage = NumberToImage::getImage(i+1,device);
                    if (idNumberImage) {
                        core::rect<s32> sourceRect = core::rect<s32>(0,0,idNumberImage->getDimension().Width,idNumberImage->getDimension().Height);
                        idNumberImage->copyToWithAlpha(radarImageOverlaid,core::position2d<s32>(xTextPos,yTextPos),sourceRect,video::SColor(255,255,255,255));
                        idNumberImage->drop();
                    }
				}



            }
        }
    }

    //Draw ARPA stuff here from arpaContacts, into radarImage
    for(unsigned int i = 0; i < arpaContacts.size(); i++) {
        ARPAEstimatedState thisEstimate = arpaContacts.at(i).estimate;


        if (!thisEstimate.stationary && thisEstimate.range <= getRangeNm()*M_IN_NM && thisEstimate.range != 0) {
            //Contact is in range, and not exactly zero, i.e. not valid

            //range in pixels
            f32 contactRangePx = (f32)bitmapWidth/2.0 * thisEstimate.range/getRangeNm();

            //Find estimated screen location of contact
            s32 deltaX = centrePixel + contactRangePx * sin((thisEstimate.bearing+radarOffsetAngle)*RAD_IN_DEG);
            s32 deltaY = centrePixel - contactRangePx * cos((thisEstimate.bearing+radarOffsetAngle)*RAD_IN_DEG);

            //Show contact on screen
            drawCircle(radarImageOverlaid,deltaX,deltaY,radarRadiusPx/40,255,255,255,255); //Draw circle around contact

            //Draw contact's display ID :
            video::IImage* idNumberImage = NumberToImage::getImage(thisEstimate.displayID,device);

            if (idNumberImage) {
                core::rect<s32> sourceRect = core::rect<s32>(0,0,idNumberImage->getDimension().Width,idNumberImage->getDimension().Height);
                idNumberImage->copyToWithAlpha(radarImageOverlaid,core::position2d<s32>(deltaX-10,deltaY-10),sourceRect,video::SColor(255,255,255,255));
                idNumberImage->drop();
            }

            //draw a vector
            f32 adjustedVectorX;
            f32 adjustedVectorZ;
            if (trueVectors) {
                adjustedVectorX = thisEstimate.absVectorX;
                adjustedVectorZ = thisEstimate.absVectorZ;
            } else {
                adjustedVectorX = thisEstimate.relVectorX;
                adjustedVectorZ = thisEstimate.relVectorZ;
            }

            //Rotate if in head/course up mode
            if (headUp) {
                f32 cosOffsetAngle = cos(-1*radarOffsetAngle*core::DEGTORAD);
                f32 sinOffsetAngle = sin(-1*radarOffsetAngle*core::DEGTORAD);

                //Implement rotation here
                f32 newX = adjustedVectorX*cosOffsetAngle - adjustedVectorZ*sinOffsetAngle;
                f32 newZ = adjustedVectorX*sinOffsetAngle + adjustedVectorZ*cosOffsetAngle;

                adjustedVectorX = newX;
                adjustedVectorZ = newZ;
            }

            s32 headingVectorX = Utilities::round(((f32)bitmapWidth/2.0)   * adjustedVectorX * 60 * vectorLengthMinutes / (M_IN_NM * getRangeNm())); //Vector length in pixels
            s32 headingVectorY = Utilities::round(((f32)bitmapWidth/2.0)*-1* adjustedVectorZ * 60 * vectorLengthMinutes / (M_IN_NM * getRangeNm()));

            //std::cout << headingVectorX << " " << headingVectorY << std::endl;

            drawLine(radarImageOverlaid,deltaX,deltaY,deltaX + headingVectorX,deltaY+headingVectorY,255,255,255,255);

        }
    }


}

void RadarCalculation::drawSector(irr::video::IImage * radarImage,irr::f32 centreX, irr::f32 centreY, irr::f32 innerRadius, irr::f32 outerRadius, irr::f32 startAngle, irr::f32 endAngle, irr::u32 alpha, irr::u32 red, irr::u32 green, irr::u32 blue, irr::f32 ownShipHeading)
//draw a bounded sector
{
    if (headUp) {
        startAngle -= ownShipHeading;
        endAngle -= ownShipHeading;
    }

    //find the corner points (Fixme: Not quite right when the extreme point is on the outer curve)
    irr::f32 sinStartAngle = std::sin(irr::core::DEGTORAD*startAngle);
    irr::f32 cosStartAngle = std::cos(irr::core::DEGTORAD*startAngle);
    irr::f32 sinEndAngle = std::sin(irr::core::DEGTORAD*endAngle);
    irr::f32 cosEndAngle = std::cos(irr::core::DEGTORAD*endAngle);

    irr::f32 point1X = centreX + sinStartAngle*innerRadius;
    irr::f32 point1Y = centreY - cosStartAngle*innerRadius;
    irr::f32 point2X = centreX + sinStartAngle*outerRadius;
    irr::f32 point2Y = centreY - cosStartAngle*outerRadius;
    irr::f32 point3X = centreX + sinEndAngle*outerRadius;
    irr::f32 point3Y = centreY - cosEndAngle*outerRadius;
    irr::f32 point4X = centreX + sinEndAngle*innerRadius;
    irr::f32 point4Y = centreY - cosEndAngle*innerRadius;

    //find the 'bounding box'
    irr::f32 minX = std::min(std::min(point1X,point2X),std::min(point3X,point4X));
    irr::f32 maxX = std::max(std::max(point1X,point2X),std::max(point3X,point4X));
    irr::f32 minY = std::min(std::min(point1Y,point2Y),std::min(point3Y,point4Y));
    irr::f32 maxY = std::max(std::max(point1Y,point2Y),std::max(point3Y,point4Y));

    irr::f32 innerRadiusSqr = std::pow(innerRadius,2);
    irr::f32 outerRadiusSqr = std::pow(outerRadius,2);

    //draw the points
    for (int i = minX;i<=maxX;i++) {
        irr::f32 localX = i - centreX; //position referred to centre
        irr::f32 localXSq = localX*localX;

        for (int j = minY;j<=maxY;j++) {

            irr::f32 localY = j - centreY; //position referred to centre

            irr::f32 localRadiusSqr = localXSq + localY*localY; //check radius of points
            //irr::f32 localAngle = irr::core::RADTODEG*std::atan2(localX,-1*localY); //check angle of point
            //irr::f32 localAngle = irr::core::RADTODEG*fast_atan2f(localX,-1*localY);

            //if the point is within the limits, plot it
            if (localRadiusSqr >= innerRadiusSqr && localRadiusSqr <= outerRadiusSqr) {
                //if (Angles::isAngleBetween(localAngle,startAngle,endAngle)) {
                if (Angles::isAngleBetween(core::vector2df(localX,-1*localY),core::vector2df(sinStartAngle,cosStartAngle),core::vector2df(sinEndAngle,cosEndAngle))) {
                    //Plot i,j
                    if (i >= 0 && j >= 0) {radarImage->setPixel(i,j,video::SColor(alpha,red,green,blue));}
                }
            }
        }
    }
}

void RadarCalculation::drawLine(irr::video::IImage * radarImage, irr::f32 startX, irr::f32 startY, irr::f32 endX, irr::f32 endY, irr::u32 alpha, irr::u32 red, irr::u32 green, irr::u32 blue)//Try with f32 as inputs so we can do interpolation based on the theoretical start and end
{

    f32 deltaX = endX - startX;
    f32 deltaY = endY - startY;

    f32 lengthSum = std::abs(deltaX) + std::abs(deltaY);

    u32 radiusSquared = pow(radarRadiusPx,2);

    if (lengthSum > 0) {
        for (f32 i = 0; i<=1; i += 1/lengthSum) {
            s32 thisX = Utilities::round(startX + deltaX * i);
            s32 thisY = Utilities::round(startY + deltaY * i);
            //Find distance from centre
            s32 centreToX = thisX - radarRadiusPx;
            s32 centreToY = thisY - radarRadiusPx;
            if (pow(centreToX,2) + pow(centreToY,2) <= radiusSquared) {
                if (thisX >= 0 && thisY >= 0) {
                    radarImage->setPixel(thisX,thisY,video::SColor(alpha,red,green,blue));
                }
            }
        }
    } else {
        s32 thisX = Utilities::round(startX);
        s32 thisY = Utilities::round(startY);
        //Find distance from centre
        s32 centreToX = thisX - radarRadiusPx;
        s32 centreToY = thisY - radarRadiusPx;
        if (pow(centreToX,2) + pow(centreToY,2) <= radiusSquared) {
            if (thisX >= 0 && thisY >= 0) {radarImage->setPixel(thisX,thisY,video::SColor(alpha,red,green,blue));}
        }
    }
}

void RadarCalculation::drawCircle(irr::video::IImage * radarImage, irr::f32 centreX, irr::f32 centreY, irr::f32 radius, irr::u32 alpha, irr::u32 red, irr::u32 green, irr::u32 blue)//Try with f32 as inputs so we can do interpolation based on the theoretical start and end
{
    f32 circumference = 2.0 * PI * radius;

    u32 radiusSquared = pow(radarRadiusPx,2);

    if (circumference > 0) {
        for (f32 i = 0; i<=1; i += 1/circumference) {
            s32 thisX = Utilities::round(centreX + radius * sin(i*2*PI));
            s32 thisY = Utilities::round(centreY + radius * cos(i*2*PI));
            //Find distance from centre
            s32 centreToX = thisX - radarRadiusPx;
            s32 centreToY = thisY - radarRadiusPx;
            if (pow(centreToX,2) + pow(centreToY,2) <= radiusSquared) {
                if (thisX >= 0 && thisY >= 0) {
                    radarImage->setPixel(thisX,thisY,video::SColor(alpha,red,green,blue));
                }
            }
        }
    } else {
        s32 thisX = Utilities::round(centreX);
        s32 thisY = Utilities::round(centreY);
        //Find distance from centre
        s32 centreToX = thisX - radarRadiusPx;
        s32 centreToY = thisY - radarRadiusPx;
        if (pow(centreToX,2) + pow(centreToY,2) <= radiusSquared) {
            if (thisX >= 0 && thisY >= 0) {
                radarImage->setPixel(thisX,thisY,video::SColor(alpha,red,green,blue));
            }
        }
    }
}

irr::f32 RadarCalculation::rangeAtAngle(irr::f32 checkAngle,irr::f32 centreX, irr::f32 centreZ, irr::f32 heading)
{
	//Special case is if heading and checkAngle are identical. In this case, return the centre point if it lies on the angle, and 0 if not
	if (std::abs(Angles::normaliseAngle(checkAngle-heading)) < 0.001) {
		if (Angles::normaliseAngle(irr::core::RADTODEG*std::atan2(centreX,centreZ)-checkAngle) < 0.1) {
			return std::sqrt(std::pow(centreX,2) + std::pow(centreZ,2));
		} else {
			return 0;
		}
	}

	irr::f32 lambda; //This is the distance from the centre of the contact

	lambda = (centreX - centreZ*tan(irr::core::DEGTORAD*checkAngle))/(cos(irr::core::DEGTORAD*heading)*tan(irr::core::DEGTORAD*checkAngle) - sin(irr::core::DEGTORAD*heading));

	irr::f32 distanceSqr = std::pow(lambda,2) + lambda*(2*centreX*sin(irr::core::DEGTORAD*heading) + 2*centreZ*cos(irr::core::DEGTORAD*heading)) + (std::pow(centreX,2) + std::pow(centreZ,2));

	irr::f32 distance = 0;

	if (distanceSqr > 0) {
		distance = std::sqrt(distanceSqr);
	}

	return distance;

}

irr::f32 RadarCalculation::radarNoise(irr::f32 radarNoiseLevel, irr::f32 radarSeaClutter, irr::f32 radarRainClutter, irr::f32 weather, irr::f32 radarRange,irr::f32 radarBrgDeg, irr::f32 windDirectionDeg, irr::f32 radarInclinationAngle, irr::f32 rainIntensity)
//radarRange in metres
{
	irr::f32 radarNoiseVal = 0;

	if (radarRange != 0) {

		irr::f32 randomValue = (irr::f32)rand()/RAND_MAX; //store this so we can manipulate the random distribution;
		irr::f32 randomValueSea = (irr::f32)rand()/RAND_MAX; //different value for sea clutter;

		//reshape the uniform random distribution into one with an infinite tail up to high values
		irr::f32 randomValueWithTail=0;
		if (randomValue > 0) {
            //3rd power is to shape distribution so sufficient high energy returns are generated
			randomValueWithTail = randomValue * pow( (1/randomValue) - 1, 3);
		}

		//same for sea clutter noise
		irr::f32 randomValueWithTailSea=0;
		if (randomValueSea > 0) {
            if (radarInclinationAngle > 0) {
                randomValueWithTailSea = 0; //if radar is scanning upwards, must be above sea surface, so don't add clutter
            } else {
                //3rd power is to shape distribution so sufficient high energy returns are generated
                randomValueWithTailSea = randomValueSea * pow((1/randomValueSea) - 1, 3);
            }
		}

		//less high power returns for rain clutter - roughly gaussian, so get an average of independent random numbers
		irr::f32 randomValueWithTailRain = ((irr::f32)rand()/RAND_MAX + (irr::f32)rand()/RAND_MAX + (irr::f32)rand()/RAND_MAX + (irr::f32)rand()/RAND_MAX)/4.0;

		//Apply directional correction to the clutter, so most is upwind, some is downwind. Mean value = 1
		irr::f32 relativeWindAngle = (windDirectionDeg - radarBrgDeg)*RAD_IN_DEG;
		irr::f32 windCorrectionFactor = 2.5*(0.5*(cos(2*relativeWindAngle)+1))*(0.5+sin(relativeWindAngle/2.0)*0.5);
		randomValueWithTailSea = randomValueWithTailSea * windCorrectionFactor;

		//noise is constant
		radarNoiseVal = radarNoiseLevel * randomValueWithTail;
		//clutter falls off with distance^3, and is normalised for weather#=6
		radarNoiseVal += radarSeaClutter * randomValueWithTailSea * (weather/6.0) * pow((M_IN_NM/radarRange),3);
		//rain clutter falls off with distance^2, and is normalised for rainIntensity#=10
		radarNoiseVal += radarRainClutter * randomValueWithTailRain * (rainIntensity/10.0)*(rainIntensity/10.0) * pow((M_IN_NM/radarRange),2);
	}

	return radarNoiseVal;
}
