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

////using namespace irr;

RadarCalculation::RadarCalculation() : rangeResolution(128), angularResolution(360)
{
    
    //Initial values for controls, all 0-100:
    radarGain = 50;
    radarRainClutterReduction=0;
    radarSeaClutterReduction=0;

    currentRadarColourChoice=0;

    EBLRangeNm=0;
    EBLBrg=0;

    CursorRangeNm = 0;
    CursorBrg = 0;

    EBLLastUpdated = clock();

    radarOn = true;

    //Radar modes: North up (false, false). Course up (true, true). Head up (true, false)
    headUp = false;
    stabilised = false;
    trueVectors = true;
    vectorLengthMinutes = 6;
    arpaOn = false;

    //Hard coded in GUI and here for 10 parallel index lines
    for(irr::u32 i=0; i<10; i++) {
        piBearings.push_back(0.0);
        piRanges.push_back(0.0);
    }

    radarScreenStale = true;
    radarRadiusPx = 10; //Set to an arbitrary value initially, will be set later.

    currentScanAngle = 0;
    currentScanLine  = 0;
}

RadarCalculation::~RadarCalculation()
{
    //dtor
}

void RadarCalculation::load(std::string radarConfigFile, irr::IrrlichtDevice* dev)
{
    device = dev;

    // Radar resolution defaults from bc5.ini
    std::string userFolder = Utilities::getUserDir();
    std::string iniFilename = "bc5.ini";
    if (Utilities::pathExists(userFolder + iniFilename)) {
        iniFilename = userFolder + iniFilename;
    }
    rangeResolution = IniFile::iniFileTou32(iniFilename, "RADAR_RangeRes", rangeResolution);
    angularResolution = IniFile::iniFileTou32(iniFilename, "RADAR_AngularRes", angularResolution);
    irr::u32 rangeResolution_max = IniFile::iniFileTou32(iniFilename, "RADAR_RangeRes_Max");
    irr::u32 angularResolution_max = IniFile::iniFileTou32(iniFilename, "RADAR_AngularRes_Max");
    
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

        radarScannerHeight = 2.0;
        radarNoiseLevel = 0.000000000005;
        radarSeaClutter = 0.000000001;
        radarRainClutter = 0.00001;

        rangeSensitivity = 20;

        marpaContacts = 10;

        irr::video::SColor radarBackgroundColour;
        irr::video::SColor radarForegroundColour;

        radarBackgroundColour.setAlpha(255);
        radarBackgroundColour.setRed(0);
        radarBackgroundColour.setGreen(0);
        radarBackgroundColour.setBlue(200);

        radarForegroundColour.setAlpha(255);
        radarForegroundColour.setRed(255);
        radarForegroundColour.setGreen(220);
        radarForegroundColour.setBlue(0);
        
        radarBackgroundColours.push_back(radarBackgroundColour);
        radarForegroundColours.push_back(radarForegroundColour);

        // Check in case resolution is set to an invalid value
        if (rangeResolution < 1) {rangeResolution = 128;}
        if (angularResolution < 1) {angularResolution = 360;}
        // Limit to maximum (if set)
        if (rangeResolution_max > 0 && rangeResolution > rangeResolution_max) {
            rangeResolution = rangeResolution_max;
        }
        if (angularResolution_max > 0 && angularResolution > angularResolution_max) {
            angularResolution = angularResolution_max;
        }
    
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
        
        marpaContacts = IniFile::iniFileTou32(radarConfigFile,"MARPAContacts");

        irr::u32 numberOfRadarColourSets = IniFile::iniFileTof32(radarConfigFile,"NumberOfRadarColourSets");
        if (numberOfRadarColourSets == 0) {
            //legacy loading
            irr::video::SColor radarBackgroundColour;
            irr::video::SColor radarForegroundColour;

            radarBackgroundColour.setAlpha(255);
            radarBackgroundColour.setRed(IniFile::iniFileTou32(radarConfigFile,"radar_bg_red"));
            radarBackgroundColour.setGreen(IniFile::iniFileTou32(radarConfigFile,"radar_bg_green"));
            radarBackgroundColour.setBlue(IniFile::iniFileTou32(radarConfigFile,"radar_bg_blue"));

            radarForegroundColour.setAlpha(255);
            radarForegroundColour.setRed(IniFile::iniFileTou32(radarConfigFile,"radar1_red"));
            radarForegroundColour.setGreen(IniFile::iniFileTou32(radarConfigFile,"radar1_green"));
            radarForegroundColour.setBlue(IniFile::iniFileTou32(radarConfigFile,"radar1_blue"));

            radarBackgroundColours.push_back(radarBackgroundColour);
            radarForegroundColours.push_back(radarForegroundColour);
        } else {
            for (unsigned int i = 1; i <= numberOfRadarColourSets; i++) {
                irr::video::SColor radarBackgroundColour;
                irr::video::SColor radarForegroundColour;

                radarBackgroundColour.setAlpha(255);
                radarBackgroundColour.setRed(IniFile::iniFileTou32(radarConfigFile,IniFile::enumerate1("radar_bg_red",i)));
                radarBackgroundColour.setGreen(IniFile::iniFileTou32(radarConfigFile,IniFile::enumerate1("radar_bg_green",i)));
                radarBackgroundColour.setBlue(IniFile::iniFileTou32(radarConfigFile,IniFile::enumerate1("radar_bg_blue",i)));

                radarForegroundColour.setAlpha(255);
                radarForegroundColour.setRed(IniFile::iniFileTou32(radarConfigFile,IniFile::enumerate1("radar1_red",i)));
                radarForegroundColour.setGreen(IniFile::iniFileTou32(radarConfigFile,IniFile::enumerate1("radar1_green",i)));
                radarForegroundColour.setBlue(IniFile::iniFileTou32(radarConfigFile,IniFile::enumerate1("radar1_blue",i)));

                radarBackgroundColours.push_back(radarBackgroundColour);
                radarForegroundColours.push_back(radarForegroundColour);    
            }
        }

        // Resolution settings:

        // Override resolution if set in own ship's radar.ini file
        rangeResolution = IniFile::iniFileTou32(radarConfigFile, "RADAR_RangeRes", rangeResolution);
        angularResolution = IniFile::iniFileTou32(radarConfigFile, "RADAR_AngularRes", angularResolution);
        // Check in case resolution is still not valid
        if (rangeResolution < 1) {rangeResolution = 128;}
        if (angularResolution < 1) {angularResolution = 360;}
        // Limit to maximum (if set)
        if (rangeResolution_max > 0 && rangeResolution > rangeResolution_max) {
            rangeResolution = rangeResolution_max;
        }
        if (angularResolution_max > 0 && angularResolution > angularResolution_max) {
            angularResolution = angularResolution_max;
        }

    }

    //initialise scanArray size (angularResolution x rangeResolution points per scan)
    scanArray.resize(angularResolution,std::vector<irr::f32>(rangeResolution,0.0));
    scanArrayAmplified.resize(angularResolution,std::vector<irr::f32>(rangeResolution,0.0));
    scanArrayToPlot.resize(angularResolution,std::vector<irr::f32>(rangeResolution,0.0));
    scanArrayToPlotPrevious.resize(angularResolution,std::vector<irr::f32>(rangeResolution,0.0));
    toReplot.resize(angularResolution);

    //initialise arrays
    for(irr::u32 i = 0; i<angularResolution; i++) {
        for(irr::u32 j = 0; j<rangeResolution; j++) {
            scanArray[i][j] = 0.0;
            scanArrayAmplified[i][j] = 0.0;
            scanArrayToPlot[i][j] = 0.0;
            scanArrayToPlotPrevious[i][j] = -1.0;
        }
    }

    scanAngleStep = 360.0f / (irr::f32) angularResolution;
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

irr::f32 RadarCalculation::getCursorBrg() const
{
    return CursorBrg;
}

irr::f32 RadarCalculation::getCursorRangeNm() const
{
    return CursorRangeNm;
}

irr::f32 RadarCalculation::getEBLBrg() const
{
    return EBLBrg;
}

void RadarCalculation::setPIData(irr::s32 PIid, irr::f32 PIbearing, irr::f32 PIrange)
{
    if (PIid >= 0 && PIid < (irr::s32)piBearings.size() && PIid < (irr::s32)piRanges.size()) {
        piBearings.at(PIid) = PIbearing;
        piRanges.at(PIid) = PIrange;
    }
}

irr::f32 RadarCalculation::getPIbearing(irr::s32 PIid) const
{
    if (PIid >= 0 && PIid < (irr::s32)piBearings.size()) {
        return piBearings.at(PIid);
    } else {
        return 0;
    }
}

irr::f32 RadarCalculation::getPIrange(irr::s32 PIid) const
{
    if (PIid >= 0 && PIid < (irr::s32)piRanges.size()) {
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
        while (EBLBrg >= 360) {
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
        while (EBLBrg < 0) {
            EBLBrg += 360;
        }
    }
}

void RadarCalculation::setNorthUp()
{
    //Radar modes: North up (false, false). Course up (true, true). Head up (true, false)
    headUp = false;
    stabilised = false;
    radarScreenStale = true;
}

void RadarCalculation::setCourseUp()
{
    //Radar modes: North up (false, false). Course up (true, true). Head up (true, false)
    headUp = true;
    stabilised = true;
    radarScreenStale = true;
}

void RadarCalculation::setHeadUp()
{
    //Radar modes: North up (false, false). Course up (true, true). Head up (true, false)
    headUp = true;
    stabilised = false;
    radarScreenStale = true;
}

bool RadarCalculation::getHeadUp() const//Head or course up
{
    return headUp;
}

void RadarCalculation::toggleRadarOn()
{
	radarOn = !radarOn;

	if (!radarOn) {
		//Reset array to empty
		for (irr::u32 i = 0; i < angularResolution; i++) {
			for (irr::u32 j = 0; j < rangeResolution; j++) {
				scanArrayToPlot[i][j] = 0.0;
			}
		}
        radarScreenStale = true;
	}
}

bool RadarCalculation::isRadarOn() const
{
    return radarOn;
}

void RadarCalculation::setArpaOn(bool on)
{
    arpaOn = on;
    if (!arpaOn) {
        //Clear arpa scans
        arpaContacts.clear(); //TODO: Clear ones that aren't MARPA?
        arpaTracks.clear(); //TODO: Clear ones that aren't MARPA?
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

void RadarCalculation::setRadarDisplayRadius(irr::u32 radiusPx)
{
    if (radarRadiusPx != radiusPx) { //If changed
        radarRadiusPx = radiusPx;
        radarScreenStale=true;
    }
}

irr::u32 RadarCalculation::getARPATracks() const
{
    return arpaTracks.size();
}

ARPAContact RadarCalculation::getARPATrack(irr::u32 index) const
{
    return arpaContacts.at(arpaTracks.at(index));
}

void RadarCalculation::changeRadarColourChoice() 
{
    currentRadarColourChoice++;
    if (currentRadarColourChoice >= radarBackgroundColours.size()) {
        //Assume radarBackgroundColours and radarForegroundColours are the same size
        //We check size of both before using currentRadarColourChoice
        currentRadarColourChoice = 0;
    }
    radarScreenStale = true;
}

void RadarCalculation::update(irr::video::IImage * radarImage, irr::video::IImage * radarImageOverlaid, irr::core::vector3d<int64_t> offsetPosition, const Terrain& terrain, const OwnShip& ownShip, const Buoys& buoys, const OtherShips& otherShips, irr::f32 weather, irr::f32 rain, irr::f32 tideHeight, irr::f32 deltaTime, uint64_t absoluteTime, irr::core::vector2di mouseRelPosition, bool isMouseDown)
{

    //IPROF_FUNC;

    //Reset screen if needed
    if(radarScreenStale) {
        radarImage->fill(irr::video::SColor(255, 128, 128, 128)); //Fill with background colour
        //Reset 'previous' array so it will all get re-drawn
        for(irr::u32 i = 0; i<angularResolution; i++) {
            toReplot[i] = true;
            for(irr::u32 j = 0; j<rangeResolution; j++) {
                scanArrayToPlotPrevious[i][j] = -1.0;
            }
        }
        radarScreenStale = false;
    }

    //Find position of mouse cursor
    if (isMouseDown) {
        irr::f32 mouseCursorRangeXNm = (irr::f32)mouseRelPosition.X/(irr::f32)radarRadiusPx*radarRangeNm.at(radarRangeIndex);//Nm
        irr::f32 mouseCursorRangeYNm = -1.0*(irr::f32)mouseRelPosition.Y/(irr::f32)radarRadiusPx*radarRangeNm.at(radarRangeIndex);//Nm
        irr::f32 mouseCursorRange = pow(pow(mouseCursorRangeXNm,2)+pow(mouseCursorRangeYNm,2),0.5);
        irr::f32 mouseCursorBearing = irr::core::RADTODEG*std::atan2(mouseCursorRangeXNm,mouseCursorRangeYNm);
        //Check if in range
        if (mouseCursorRange <= radarRangeNm.at(radarRangeIndex) ) {
            
            //Adjust angle if needed
            if (headUp) {
                mouseCursorBearing += ownShip.getHeading();
            }
            mouseCursorBearing = Angles::normaliseAngle(mouseCursorBearing);
            
            // Set the radar cursor
            CursorRangeNm = mouseCursorRange;
            CursorBrg = mouseCursorBearing;
        }
    }

    scan(offsetPosition, terrain, ownShip, buoys, otherShips, weather, rain, tideHeight, deltaTime, absoluteTime); // scan into scanArray[row (angle)][column (step)], and with filtering and amplification into scanArrayAmplified[][]
	updateARPA(offsetPosition, ownShip, absoluteTime); //From data in arpaContacts, updated in scan()
	render(radarImage, radarImageOverlaid, ownShip.getHeading(), ownShip.getSpeed()); //From scanArrayAmplified[row (angle)][column (step)], render to radarImage

}


void RadarCalculation::scan(irr::core::vector3d<int64_t> offsetPosition, const Terrain& terrain, const OwnShip& ownShip, const Buoys& buoys, const OtherShips& otherShips, irr::f32 weather, irr::f32 rain, irr::f32 tideHeight, irr::f32 deltaTime, uint64_t absoluteTime)
{

    //IPROF_FUNC;
    const irr::u32 SECONDS_BETWEEN_SCANS = 2;

    irr::core::vector3df position = ownShip.getPosition();
    //Get absolute position relative to SW corner of world model
    irr::core::vector3d<int64_t> absolutePosition = offsetPosition;
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
    irr::u32 scansPerLoop = RADAR_RPM * RPMtoDEGPERSECOND * deltaTime / (irr::f32) scanAngleStep + (irr::f32) rand() / RAND_MAX ; //Add random value (0-1, mean 0.5), so with rounding, we get the correct radar speed, even though we can only do an integer number of scans

    if (scansPerLoop > 30) {scansPerLoop = 30;} //Limit to reasonable bounds
    for(irr::u32 i = 0; i<scansPerLoop;i++) { //Start of repeatable scan section

        // the actual angle we want to work with has to be determined here
        currentScanAngle = ((irr::f32) currentScanLine / (irr::f32) angularResolution) * 360.0f;

        irr::f32 scanSlope = -0.5; //Slope at start of scan (in metres/metre) - Make slightly negative so vessel contacts close in get detected
        for (irr::u32 currentStep = 1; currentStep<rangeResolution; currentStep++) { //Note that currentStep starts as 1, not 0. This is used in anti-rain clutter filter, which checks element at currentStep-1
            //scan into array, accessed as  scanArray[row (angle)][column (step)]

            //Clear old value
            scanArray[currentScanLine][currentStep] = 0.0;

            //Get location of area being scanned
            irr::f32 localRange = cellLength*currentStep;
            irr::f32 relX = localRange*sin(currentScanAngle*irr::core::DEGTORAD); //Distance from ship
            irr::f32 relZ = localRange*cos(currentScanAngle*irr::core::DEGTORAD);
            irr::f32 localX = position.X + relX;
            irr::f32 localZ = position.Z + relZ;

            //get extents
            irr::f32 minCellAngle = Angles::normaliseAngle(currentScanAngle - scanAngleStep/2.0);
            irr::f32 maxCellAngle = Angles::normaliseAngle(currentScanAngle + scanAngleStep/2.0);
            irr::f32 minCellRange = localRange - cellLength/2.0;
            irr::f32 maxCellRange = localRange + cellLength/2.0;

            //get adjustment of height for earth's curvature
            irr::f32 dropWithCurvature = std::pow(localRange,2)/(2*EARTH_RAD_M*EARTH_RAD_CORRECTION);

            //Calculate noise
            irr::f32 localNoise = radarNoise(radarNoiseLevel,radarSeaClutter,radarRainClutter,weather,localRange,currentScanAngle,0,scanSlope,rain); //FIXME: Needs wind direction

            //Scan other contacts here
            for(unsigned int thisContact = 0; thisContact<radarData.size(); thisContact++) {
                irr::f32 contactHeightAboveLine = (radarData.at(thisContact).height - radarScannerHeight - dropWithCurvature) - scanSlope*localRange;
                if (contactHeightAboveLine > 0) {
                    //Contact would be visible if in this cell. Check if it is
                    //Start of B3D code
                    //Check if centre of target within the cell. If not then check if Either min range or max range of contact is within the cell, or min and max span the cell
                    if ((radarData.at(thisContact).range >= minCellRange && radarData.at(thisContact).range <= maxCellRange) 
                            || (radarData.at(thisContact).minRange >= minCellRange && radarData.at(thisContact).minRange <= maxCellRange)
                            || (radarData.at(thisContact).maxRange >= minCellRange && radarData.at(thisContact).maxRange <= maxCellRange) 
                            || (radarData.at(thisContact).minRange < minCellRange && radarData.at(thisContact).maxRange > maxCellRange)) {

                        //Check if centre of target within the cell. If not then check if either min angle or max angle of contact is within the cell, or min and max span the cell
                        if ((Angles::isAngleBetween(radarData.at(thisContact).angle,minCellAngle,maxCellAngle)) 
                                || (Angles::isAngleBetween(radarData.at(thisContact).minAngle,minCellAngle,maxCellAngle))
                                || (Angles::isAngleBetween(radarData.at(thisContact).maxAngle,minCellAngle,maxCellAngle))
                                || (Angles::normaliseAngle(radarData.at(thisContact).minAngle-minCellAngle) > 270 && Angles::normaliseAngle(radarData.at(thisContact).maxAngle-maxCellAngle) < 90)) {

                            irr::f32 rangeAtCellMin = rangeAtAngle(minCellAngle,radarData.at(thisContact).relX,radarData.at(thisContact).relZ,radarData.at(thisContact).heading);
                            irr::f32 rangeAtCellMax = rangeAtAngle(maxCellAngle,radarData.at(thisContact).relX,radarData.at(thisContact).relZ,radarData.at(thisContact).heading);

                            //check if the contact intersects this exact cell, if its extremes overlap it
                            //Also check if the target centre is in the cell, or the extended target spans the cell (ie RangeAtCellMin less than minCellRange and rangeAtCellMax greater than maxCellRange and vice versa)
                            if ((((radarData.at(thisContact).range >= minCellRange && radarData.at(thisContact).range <= maxCellRange) 
                                            && (Angles::isAngleBetween(radarData.at(thisContact).angle,minCellAngle,maxCellAngle)))
                                        || (rangeAtCellMin >= minCellRange && rangeAtCellMin <= maxCellRange)
                                        || (rangeAtCellMax >= minCellRange && rangeAtCellMax <= maxCellRange)
                                        || (rangeAtCellMin < minCellRange && rangeAtCellMax > maxCellRange)
                                        || (rangeAtCellMax < minCellRange && rangeAtCellMin > maxCellRange))) {

                                irr::f32 radarEchoStrength = radarFactorVessel * std::pow(M_IN_NM/localRange,4) * radarData.at(thisContact).rcs;
                                scanArray[currentScanLine][currentStep] += radarEchoStrength;

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
                                        newContact.estimate.contactType = newContact.contactType; //Redundant here, but useful to pass to the GUI later

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
                                        //newScan.estimatedRCS = 100;//Todo: Implement

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
                                        //Todo: should we limit the size of this, so it doesn't continue accumulating?

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
            irr::f32 terrainHeightAboveSea = terrain.getHeight(localX,localZ) - tideHeight;
            irr::f32 radarHeight = terrainHeightAboveSea - dropWithCurvature - radarScannerHeight;
            irr::f32 localSlope = radarHeight/localRange;
            irr::f32 heightAboveLine = radarHeight - scanSlope*localRange; //Find height above previous maximum scan slope

            if (heightAboveLine>0 && terrainHeightAboveSea>0) {
                irr::f32 radarLocalGradient = heightAboveLine/cellLength;
                scanSlope = localSlope; //Highest so far on scan
                scanArray[currentScanLine][currentStep] += radarFactorLand*std::atan(radarLocalGradient)*(2/PI)/std::pow(localRange/M_IN_NM,3); //make a reflection off a plane wall at 1nm have a magnitude of 1*radarFactorLand
            }

            //Add radar noise
            scanArray[currentScanLine][currentStep] += localNoise;

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
            irr::f32 intensityGradient = scanArray[currentScanLine][currentStep] - scanArray[currentScanLine][currentStep-1];
            if (intensityGradient<0) {intensityGradient=0;}

            irr::f32 filteredSignal = intensityGradient*rainFilter + scanArray[currentScanLine][currentStep]*(1-rainFilter);
            irr::f32 radarLocalGain = 500000*(8*pow(radarGain/100.0,4)) * radarSTCGain ;

            //take log (natural) of signal
            irr::f32 logSignal = log(filteredSignal*radarLocalGain);
            scanArrayAmplified[currentScanLine][currentStep] = std::max(0.0f,logSignal);

            //Generate a filtered version, based on the angles around. Lag behind by (for example) 3 steps, so we can filter on what's ahead, as well as what's behind
            irr::s32 filterAngle = (irr::s32)currentScanLine - 3;
                while(filterAngle < 0) {filterAngle+=angularResolution;}
                while(filterAngle >= angularResolution) {filterAngle-=angularResolution;}
            irr::s32 filterAngle_1 = filterAngle - 3;
                while(filterAngle_1 < 0) {filterAngle_1+=angularResolution;}
                while(filterAngle_1 >= angularResolution) {filterAngle_1-=angularResolution;}
            irr::s32 filterAngle_2 = filterAngle - 2;
                while(filterAngle_2 < 0) {filterAngle_2+=angularResolution;}
                while(filterAngle_2 >= angularResolution) {filterAngle_2-=angularResolution;}
            irr::s32 filterAngle_3 = filterAngle - 1;
                while(filterAngle_3 < 0) {filterAngle_3+=angularResolution;}
                while(filterAngle_3 >= angularResolution) {filterAngle_3-=angularResolution;}
            irr::s32 filterAngle_4 = filterAngle;
                while(filterAngle_4 < 0) {filterAngle_4+=angularResolution;}
                while(filterAngle_4 >= angularResolution) {filterAngle_4-=angularResolution;}
            irr::s32 filterAngle_5 = filterAngle + 1;
                while(filterAngle_5 < 0) {filterAngle_5+=angularResolution;}
                while(filterAngle_5 >= angularResolution) {filterAngle_5-=angularResolution;}
            irr::s32 filterAngle_6 = filterAngle + 2;
                while(filterAngle_6 < 0) {filterAngle_6+=angularResolution;}
                while(filterAngle_6 >= angularResolution) {filterAngle_6-=angularResolution;}
            irr::s32 filterAngle_7 = filterAngle + 3;
                while(filterAngle_7 < 0) {filterAngle_7+=angularResolution;}
                while(filterAngle_7 >= angularResolution) {filterAngle_7-=angularResolution;}
            if (currentStep < rangeResolution * 0.1) {
                scanArrayToPlot[filterAngle][currentStep] = std::max({
                    scanArrayAmplified[filterAngle_1][currentStep],
                    scanArrayAmplified[filterAngle_2][currentStep],
                    scanArrayAmplified[filterAngle_3][currentStep],
                    scanArrayAmplified[filterAngle_4][currentStep],
                    scanArrayAmplified[filterAngle_5][currentStep],
                    scanArrayAmplified[filterAngle_6][currentStep],
                    scanArrayAmplified[filterAngle_7][currentStep]
                });

            } else if (currentStep < rangeResolution * 0.2) {
                scanArrayToPlot[filterAngle][currentStep] = std::max({
                    scanArrayAmplified[filterAngle_2][currentStep],
                    scanArrayAmplified[filterAngle_3][currentStep],
                    scanArrayAmplified[filterAngle_4][currentStep],
                    scanArrayAmplified[filterAngle_5][currentStep],
                    scanArrayAmplified[filterAngle_6][currentStep]
                });
            } else if (currentStep < rangeResolution * 0.3) {
                scanArrayToPlot[filterAngle][currentStep] = std::max({
                    scanArrayAmplified[filterAngle_3][currentStep],
                    scanArrayAmplified[filterAngle_4][currentStep],
                    scanArrayAmplified[filterAngle_5][currentStep],
                });
            } else {
                scanArrayToPlot[filterAngle][currentStep] = scanArrayAmplified[filterAngle_4][currentStep];
            }
            toReplot[filterAngle] = true;

            //Clamp between 0 and 1
            if (scanArrayToPlot[filterAngle][currentStep] < 0) {
                scanArrayToPlot[filterAngle][currentStep] = 0;
            }
            if (scanArrayToPlot[filterAngle][currentStep] > 1) {
                scanArrayToPlot[filterAngle][currentStep] = 1;
            }

        } //End of for loop scanning out



        //Increment scan line for next time
        currentScanLine++;
        if (currentScanLine >= angularResolution) {
            currentScanLine = 0;
        }
    } //End of repeatable scan section



}

void RadarCalculation::updateARPA(irr::core::vector3d<int64_t> offsetPosition, const OwnShip& ownShip, uint64_t absoluteTime)
{

    //IPROF_FUNC;
    //Own ship absolute position
    irr::core::vector3df position = ownShip.getPosition();
    //Get absolute position relative to SW corner of world model
    irr::core::vector3d<int64_t> absolutePosition = offsetPosition;
    absolutePosition.X += position.X;
    absolutePosition.Y += position.Y;
    absolutePosition.Z += position.Z;

    //Based on scans data in arpaContacts, estimate current speed, heading and position
    for (unsigned int i = 0; i<arpaContacts.size(); i++) {
        updateArpaEstimate(arpaContacts.at(i), i, ownShip, absolutePosition, absoluteTime); //This will update the estimate etc. TODO: Passing in the ID will need to change for MARPA
    } //For loop through arpa contacts
}

void RadarCalculation::updateArpaEstimate(ARPAContact& thisArpaContact, int contactID, const OwnShip& ownShip, irr::core::vector3d<int64_t> absolutePosition, uint64_t absoluteTime) 
{
    if (!arpaOn) {
        //Set all contacts to zero (untracked) if normal type
        if (thisArpaContact.contactType == CONTACT_NORMAL) {
            thisArpaContact.estimate.displayID = 0;
            thisArpaContact.estimate.stationary = true;
            thisArpaContact.estimate.lost = false;
            thisArpaContact.estimate.absVectorX = 0;
            thisArpaContact.estimate.absVectorZ = 0;
            thisArpaContact.estimate.absHeading = 0;
            thisArpaContact.estimate.bearing = 0;
            thisArpaContact.estimate.range = 0;
            thisArpaContact.estimate.speed = 0;
            thisArpaContact.estimate.contactType = CONTACT_NONE;
        }
    } else {
        //Check there are at least two scans, so we can estimate behaviour
        if (thisArpaContact.scans.size() > 1) {

            //Record the contact type in the estimate
            thisArpaContact.estimate.contactType = thisArpaContact.contactType;
            
            //Check stationary contacts to see if they've got detectable motion: TODO: Make this better: Should weight based on current range?
            //TODO: Test this weighting
            if (thisArpaContact.estimate.stationary) {
                irr::f32 latestRangeNm =  thisArpaContact.scans.back().rangeNm;
                if (latestRangeNm < 1) {
                    latestRangeNm = 1;
                }
                irr::f32 weightedMotionX = fabs(thisArpaContact.totalXMovementEst/latestRangeNm);
                irr::f32 weightedMotionZ = fabs(thisArpaContact.totalZMovementEst/latestRangeNm);
                if (weightedMotionX >= 100 || weightedMotionZ >= 100) {
                    thisArpaContact.estimate.stationary = false;
                }
            }

            //Check if contact lost, if last scanned more than 60 seconds ago
            if ( absoluteTime - thisArpaContact.scans.back().timeStamp > 60) {
                thisArpaContact.estimate.lost=true;
                //std::cout << "Contact " << i << " lost" << std::endl;
            } else if (!thisArpaContact.estimate.stationary) {
                //If ID is 0 (unassigned), set id and increment
                if (thisArpaContact.estimate.displayID==0) {
                    arpaTracks.push_back(contactID);
                    thisArpaContact.estimate.displayID = getARPATracks(); // The display ID is the current size of thr arpaTracks list. TODO: Will need update for MARPA

                }

                irr::s32 stepsBack = 60; //Default time for tracking (time = stepsBack * SECONDS_BETWEEN_SCANS)
                irr::s32 recentStepsBack = 10; //Shorter time for tracking (if motion has changed significantly)

                irr::s32 currentScanIndex = thisArpaContact.scans.size() - 1;
                irr::s32 referenceScanIndex = currentScanIndex - stepsBack;
                if (referenceScanIndex < 0) {
                    referenceScanIndex = 0;
                }

                ARPAScan currentScanData = thisArpaContact.scans.at(currentScanIndex);
                ARPAScan referenceScanData = thisArpaContact.scans.at(referenceScanIndex);

                //Check if heading/speed has changed dramatically, by taking last 10 scans. If so, reduce steps back to 10
                irr::s32 actualStepsBack = currentScanIndex - referenceScanIndex;
                if (actualStepsBack > recentStepsBack) {
                    ARPAScan recentScanData = thisArpaContact.scans.at(currentScanIndex-recentStepsBack);
                    irr::f32 deltaTimeRecent = currentScanData.timeStamp - recentScanData.timeStamp;
                    irr::f32 deltaTimeFull   = currentScanData.timeStamp - referenceScanData.timeStamp;
                    if (deltaTimeRecent>0 && deltaTimeFull > 0) {
                        irr::f32 deltaXRecent = currentScanData.x - recentScanData.x;
                        irr::f32 deltaZRecent = currentScanData.z - recentScanData.z;
                        irr::f32 deltaXFull   = currentScanData.x - referenceScanData.x;
                        irr::f32 deltaZFull   = currentScanData.z - referenceScanData.z;

                        //Absolute vector
                        irr::f32 absVectorXRecent = deltaXRecent/deltaTimeRecent; //m/s
                        irr::f32 absVectorZRecent = deltaZRecent/deltaTimeRecent; //m/s
                        irr::f32 absVectorXFull = deltaXFull/deltaTimeFull; //m/s
                        irr::f32 absVectorZFull = deltaZFull/deltaTimeFull; //m/s

                        //Difference in estimation
                        irr::f32 changeX = absVectorXRecent - absVectorXFull;
                        irr::f32 changeZ = absVectorZRecent - absVectorZFull;

                        //If speed estimates differ by more than 1m/s in either direction, prefer the more recent estimate. Otherwise, leave unchanged
                        if (std::abs(changeX) > 1.0 || std::abs(changeZ) > 1.0) {
                            referenceScanData = recentScanData; //It seems like the course or speed has changed significantly
                        }
                    }
                }

                //Find difference in time, position x, position z
                irr::f32 deltaTime = currentScanData.timeStamp - referenceScanData.timeStamp;
                if (deltaTime>0) {
                    irr::f32 deltaX = currentScanData.x - referenceScanData.x;
                    irr::f32 deltaZ = currentScanData.z - referenceScanData.z;

                    //Absolute vector
                    thisArpaContact.estimate.absVectorX = deltaX/deltaTime; //m/s
                    thisArpaContact.estimate.absVectorZ = deltaZ/deltaTime; //m/s
                    thisArpaContact.estimate.absHeading = std::atan2(deltaX,deltaZ)/RAD_IN_DEG;
                    while (thisArpaContact.estimate.absHeading < 0 ) {
                        thisArpaContact.estimate.absHeading += 360;
                    }
                    //Relative vector:
                    thisArpaContact.estimate.relVectorX = thisArpaContact.estimate.absVectorX - ownShip.getSpeed() * sin((ownShip.getHeading())*irr::core::DEGTORAD);
                    thisArpaContact.estimate.relVectorZ = thisArpaContact.estimate.absVectorZ - ownShip.getSpeed() * cos((ownShip.getHeading())*irr::core::DEGTORAD); //ownShipSpeed in m/s
                    thisArpaContact.estimate.relHeading = std::atan2(thisArpaContact.estimate.relVectorX,thisArpaContact.estimate.relVectorZ)/RAD_IN_DEG;
                    while (thisArpaContact.estimate.relHeading < 0 ) {
                        thisArpaContact.estimate.relHeading += 360;
                    }

                    //Estimated current position:
                    irr::f32 relX = currentScanData.x - absolutePosition.X + thisArpaContact.estimate.absVectorX * (absoluteTime - currentScanData.timeStamp);
                    irr::f32 relZ = currentScanData.z - absolutePosition.Z + thisArpaContact.estimate.absVectorZ * (absoluteTime - currentScanData.timeStamp);
                    thisArpaContact.estimate.bearing = std::atan2(relX,relZ)/RAD_IN_DEG;
                    while (thisArpaContact.estimate.bearing < 0 ) {
                        thisArpaContact.estimate.bearing += 360;
                    }
                    thisArpaContact.estimate.range =  std::sqrt(pow(relX,2)+pow(relZ,2))/M_IN_NM; //Nm
                    thisArpaContact.estimate.speed = std::sqrt(pow(thisArpaContact.estimate.absVectorX,2) + pow(thisArpaContact.estimate.absVectorZ,2))*MPS_TO_KTS;

                    //TODO: CPA AND TCPA here: Need checking/testing
                    irr::f32 contactRelAngle = thisArpaContact.estimate.relHeading - (180+thisArpaContact.estimate.bearing);
                    irr::f32 contactRange = thisArpaContact.estimate.range; //(Nm)
                    irr::f32 relDistanceToCPA = contactRange * cos(contactRelAngle*RAD_IN_DEG); //Distance along the other ship's relative motion line
                    irr::f32 relativeSpeed = std::sqrt(pow(thisArpaContact.estimate.relVectorX,2) + pow(thisArpaContact.estimate.relVectorZ,2))*MPS_TO_KTS;
                    if (fabs(relativeSpeed) < 0.001) {relativeSpeed = 0.001;} //Avoid division by zero

                    thisArpaContact.estimate.cpa = contactRange * sin(contactRelAngle*RAD_IN_DEG);
                    thisArpaContact.estimate.tcpa = 60*relDistanceToCPA/relativeSpeed; // (nm / (nm/hr)), so time in hours, converted to minutes
                    //std::cout << "Contact " << thisArpaContact.estimate.displayID << " CPA: " <<  thisArpaContact.estimate.cpa << " nm in " << thisArpaContact.estimate.tcpa << " minutes" << std::endl;


                } //If time between scans > 0
            } //Contact not lost
        } //If at least 2 scans
    } //If ARPA is on
}

void RadarCalculation::render(irr::video::IImage * radarImage, irr::video::IImage * radarImageOverlaid, irr::f32 ownShipHeading, irr::f32 ownShipSpeed)
{

    //IPROF_FUNC;
    //*************************
    //generate image from array
    //*************************

    //Render background radar picture into radarImage, then copy to radarImageOverlaid and do any 2d drawing on top (so we don't have to redraw all pixels each time
    irr::u32 bitmapWidth = radarRadiusPx*2; //Set width to use - to map GUI radar display diameter in screen pixels

    //If the image is smaller than ideal, render anyway
    if (radarImage->getDimension().Width < bitmapWidth) {
        bitmapWidth = radarImage->getDimension().Width;
    }
    //if (radarImage->getDimension().Width < bitmapWidth) //Check the image we're rendering into is big enough
    //    {return;}

    //draw from array to image
    irr::f32 centrePixel = (bitmapWidth-1.0)/2.0; //The centre of the bitmap. Normally this will be a fractional number (##.5)

    //precalculate cell max/min range for speed outside nested loop
	std::vector<irr::f32> cellMinRange;
	std::vector<irr::f32> cellMaxRange;
	//irr::f32 cellMinRange [rangeResolution];
    //irr::f32 cellMaxRange [rangeResolution];
	cellMinRange.push_back(0);
	cellMaxRange.push_back(0);
    for (irr::u32 currentStep = 1; currentStep<rangeResolution; currentStep++) { //Note that we start with the element at 1, so we've already pushed in a dummy entry at 0
        cellMinRange.push_back((currentStep-0.5)*(bitmapWidth*0.5/(float)rangeResolution));//Range in pixels from centre
        cellMaxRange.push_back((currentStep+0.5)*(bitmapWidth*0.5/(float)rangeResolution));
    }

    irr::f32 scanAngle;

    for (int scanLine = 0; scanLine < angularResolution; scanLine++) {

        scanAngle = ((irr::f32) scanLine / (irr::f32) angularResolution) * 360.0f;

        irr::f32 cellMinAngle = scanAngle - scanAngleStep / 2.0;
        irr::f32 cellMaxAngle = scanAngle + scanAngleStep / 2.0;

        for (irr::u32 currentStep = 1; currentStep<rangeResolution; currentStep++) {

            //If the sector has changed, draw it. If we're stabilising the picture, need to re-draw all in case the ship's head has changed
            if(toReplot[scanLine] || stabilised)
            {

                if (headUp || scanArrayToPlotPrevious[scanLine][currentStep] != scanArrayToPlot[scanLine][currentStep]) { //If north up, we only need to replot if the previous plot to this sector was different
                    irr::f32 pixelColour=scanArrayToPlot[scanLine][currentStep];

                    if (pixelColour>1.0) {pixelColour = 1.0;}
                    if (pixelColour<0)   {pixelColour =   0;}

                    if (currentRadarColourChoice < radarForegroundColours.size() && currentRadarColourChoice < radarBackgroundColours.size()) {
                        //Interpolate colour between foreground and background
                        irr::video::SColor thisColour = radarForegroundColours.at(currentRadarColourChoice).getInterpolated(radarBackgroundColours.at(currentRadarColourChoice), pixelColour);
                        drawSector(radarImage,
                                centrePixel,
                                centrePixel,
                                cellMinRange[currentStep],
                                cellMaxRange[currentStep],
                                cellMinAngle,
                                cellMaxAngle,
                                thisColour.getAlpha(),
                                thisColour.getRed(),
                                thisColour.getGreen(),
                                thisColour.getBlue(),
                                ownShipHeading);
                    }

                    scanArrayToPlotPrevious[scanLine][currentStep] = scanArrayToPlot[scanLine][currentStep]; //Record what we have plotted
                }
            }
        }
        //We don't need to replot this line
        toReplot[scanLine]=false;
    }

    //Copy image into overlaid
    radarImage->copyTo(radarImageOverlaid);

    //Adjust for head up/course up
    irr::f32 radarOffsetAngle = 0;
    if (headUp) {
        radarOffsetAngle = -1*ownShipHeading;
    }

    //Draw parallel indexes on here
    if (piRanges.size() == piBearings.size()) {
        for(unsigned int i = 0; i< piRanges.size(); i++) {
            irr::f32 thisPIrange = piRanges.at(i);
            irr::f32 thisPIbrg = piBearings.at(i);
            if(fabs(thisPIrange) > 0.0001 && fabs(thisPIrange) < getRangeNm()) {
                //Not zero range or off screen

                irr::f32 piRangePX = (irr::f32)bitmapWidth/2.0 * thisPIrange / getRangeNm(); //Find range in Px

                //find sin and cos of PI angle (so we only need once)
                irr::f32 sinPIbrg=sin(-1*(thisPIbrg + radarOffsetAngle)*RAD_IN_DEG);
				irr::f32 cosPIbrg=cos(-1*(thisPIbrg + radarOffsetAngle)*RAD_IN_DEG);

				//find central point on line
                irr::f32 x_a = -1*piRangePX * sin( (90-(-1*(thisPIbrg + radarOffsetAngle)))*RAD_IN_DEG ) + centrePixel;
                irr::f32 z_a =    piRangePX * cos( (90-(-1*(thisPIbrg + radarOffsetAngle)))*RAD_IN_DEG ) + centrePixel;

                //find half chord length (length of PI line)
                irr::f32 halfChord = pow(pow((irr::f32)bitmapWidth/2.0,2) - pow(piRangePX,2),0.5); //already checked that PIRange is smaller, so should be valid

                //calculate end points of line
                irr::f32 x_1=x_a - halfChord * sinPIbrg;
                irr::f32 z_1=z_a - halfChord * cosPIbrg;
                irr::f32 x_2=x_a + halfChord * sinPIbrg;
				irr::f32 z_2=z_a + halfChord * cosPIbrg;

				drawLine(radarImageOverlaid,x_1,z_1,x_2,z_2,255,255,255,255);

				//Show line number
				//Find point towards centre from the line
				irr::f32 xDirection = centrePixel-x_a;
				irr::f32 yDirection = centrePixel-z_a;
				if (x_a!=0 && z_a !=0) {

                    irr::f32 mag = pow(pow(xDirection,2)+pow(yDirection,2),0.5);
                    xDirection/=mag;
                    yDirection/=mag;

                    irr::s32 xTextPos = x_a + 15*xDirection;
                    irr::s32 yTextPos = z_a + 15*yDirection;

                    irr::video::IImage* idNumberImage = NumberToImage::getImage(i+1,device);
                    if (idNumberImage) {
                        irr::core::rect<irr::s32> sourceRect = irr::core::rect<irr::s32>(0,0,idNumberImage->getDimension().Width,idNumberImage->getDimension().Height);
                        idNumberImage->copyToWithAlpha(radarImageOverlaid,irr::core::position2d<irr::s32>(xTextPos,yTextPos),sourceRect,irr::video::SColor(255,255,255,255));
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
            irr::f32 contactRangePx = (irr::f32)bitmapWidth/2.0 * thisEstimate.range/getRangeNm();

            //Find estimated screen location of contact
            irr::s32 deltaX = centrePixel + contactRangePx * sin((thisEstimate.bearing+radarOffsetAngle)*RAD_IN_DEG);
            irr::s32 deltaY = centrePixel - contactRangePx * cos((thisEstimate.bearing+radarOffsetAngle)*RAD_IN_DEG);

            //Show contact on screen
            drawCircle(radarImageOverlaid,deltaX,deltaY,radarRadiusPx/40,255,255,255,255); //Draw circle around contact

            //Draw contact's display ID :
            irr::video::IImage* idNumberImage = NumberToImage::getImage(thisEstimate.displayID,device);

            if (idNumberImage) {
                irr::core::rect<irr::s32> sourceRect = irr::core::rect<irr::s32>(0,0,idNumberImage->getDimension().Width,idNumberImage->getDimension().Height);
                idNumberImage->copyToWithAlpha(radarImageOverlaid,irr::core::position2d<irr::s32>(deltaX-10,deltaY-10),sourceRect,irr::video::SColor(255,255,255,255));
                idNumberImage->drop();
            }

            //draw a vector
            irr::f32 adjustedVectorX;
            irr::f32 adjustedVectorZ;
            if (trueVectors) {
                adjustedVectorX = thisEstimate.absVectorX;
                adjustedVectorZ = thisEstimate.absVectorZ;
            } else {
                adjustedVectorX = thisEstimate.relVectorX;
                adjustedVectorZ = thisEstimate.relVectorZ;
            }

            //Rotate if in head/course up mode
            if (headUp) {
                irr::f32 cosOffsetAngle = cos(-1*radarOffsetAngle*irr::core::DEGTORAD);
                irr::f32 sinOffsetAngle = sin(-1*radarOffsetAngle*irr::core::DEGTORAD);

                //Implement rotation here
                irr::f32 newX = adjustedVectorX*cosOffsetAngle - adjustedVectorZ*sinOffsetAngle;
                irr::f32 newZ = adjustedVectorX*sinOffsetAngle + adjustedVectorZ*cosOffsetAngle;

                adjustedVectorX = newX;
                adjustedVectorZ = newZ;
            }

            irr::s32 headingVectorX = Utilities::round(((irr::f32)bitmapWidth/2.0)   * adjustedVectorX * 60 * vectorLengthMinutes / (M_IN_NM * getRangeNm())); //Vector length in pixels
            irr::s32 headingVectorY = Utilities::round(((irr::f32)bitmapWidth/2.0)*-1* adjustedVectorZ * 60 * vectorLengthMinutes / (M_IN_NM * getRangeNm()));

            //std::cout << headingVectorX << " " << headingVectorY << std::endl;

            drawLine(radarImageOverlaid,deltaX,deltaY,deltaX + headingVectorX,deltaY+headingVectorY,255,255,255,255);

        }
    }


}

void RadarCalculation::drawSector(irr::video::IImage * radarImage,irr::f32 centreX, irr::f32 centreY, irr::f32 innerRadius, irr::f32 outerRadius, irr::f32 startAngle, irr::f32 endAngle, irr::u32 alpha, irr::u32 red, irr::u32 green, irr::u32 blue, irr::f32 ownShipHeading)
//draw a bounded sector
{

    //IPROF_FUNC;

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
    irr::s32 minX = std::min(std::min(point1X,point2X),std::min(point3X,point4X));
    irr::s32 maxX = std::max(std::max(point1X,point2X),std::max(point3X,point4X));
    irr::s32 minY = std::min(std::min(point1Y,point2Y),std::min(point3Y,point4Y));
    irr::s32 maxY = std::max(std::max(point1Y,point2Y),std::max(point3Y,point4Y));

    irr::f32 innerRadiusSqr = innerRadius*innerRadius;
    irr::f32 outerRadiusSqr = outerRadius*outerRadius;

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
                if (Angles::isAngleBetween(irr::core::vector2df(localX,-1*localY),irr::core::vector2df(sinStartAngle,cosStartAngle),irr::core::vector2df(sinEndAngle,cosEndAngle))) {
                    //Plot i,j
                    if (i >= 0 && j >= 0) {radarImage->setPixel(i,j,irr::video::SColor(alpha,red,green,blue));}
                }
            }
        }
    }

}

void RadarCalculation::drawLine(irr::video::IImage * radarImage, irr::f32 startX, irr::f32 startY, irr::f32 endX, irr::f32 endY, irr::u32 alpha, irr::u32 red, irr::u32 green, irr::u32 blue)//Try with irr::f32 as inputs so we can do interpolation based on the theoretical start and end
{

    irr::f32 deltaX = endX - startX;
    irr::f32 deltaY = endY - startY;

    irr::f32 lengthSum = std::abs(deltaX) + std::abs(deltaY);

    irr::u32 radiusSquared = pow(radarRadiusPx,2);

    if (lengthSum > 0) {
        for (irr::f32 i = 0; i<=1; i += 1/lengthSum) {
            irr::s32 thisX = Utilities::round(startX + deltaX * i);
            irr::s32 thisY = Utilities::round(startY + deltaY * i);
            //Find distance from centre
            irr::s32 centreToX = thisX - radarRadiusPx;
            irr::s32 centreToY = thisY - radarRadiusPx;
            if (pow(centreToX,2) + pow(centreToY,2) <= radiusSquared) {
                if (thisX >= 0 && thisY >= 0) {
                    radarImage->setPixel(thisX,thisY,irr::video::SColor(alpha,red,green,blue));
                }
            }
        }
    } else {
        irr::s32 thisX = Utilities::round(startX);
        irr::s32 thisY = Utilities::round(startY);
        //Find distance from centre
        irr::s32 centreToX = thisX - radarRadiusPx;
        irr::s32 centreToY = thisY - radarRadiusPx;
        if (pow(centreToX,2) + pow(centreToY,2) <= radiusSquared) {
            if (thisX >= 0 && thisY >= 0) {radarImage->setPixel(thisX,thisY,irr::video::SColor(alpha,red,green,blue));}
        }
    }
}

void RadarCalculation::drawCircle(irr::video::IImage * radarImage, irr::f32 centreX, irr::f32 centreY, irr::f32 radius, irr::u32 alpha, irr::u32 red, irr::u32 green, irr::u32 blue)//Try with irr::f32 as inputs so we can do interpolation based on the theoretical start and end
{
    irr::f32 circumference = 2.0 * PI * radius;

    irr::u32 radiusSquared = pow(radarRadiusPx,2);

    if (circumference > 0) {
        for (irr::f32 i = 0; i<=1; i += 1/circumference) {
            irr::s32 thisX = Utilities::round(centreX + radius * sin(i*2*PI));
            irr::s32 thisY = Utilities::round(centreY + radius * cos(i*2*PI));
            //Find distance from centre
            irr::s32 centreToX = thisX - radarRadiusPx;
            irr::s32 centreToY = thisY - radarRadiusPx;
            if (pow(centreToX,2) + pow(centreToY,2) <= radiusSquared) {
                if (thisX >= 0 && thisY >= 0) {
                    radarImage->setPixel(thisX,thisY,irr::video::SColor(alpha,red,green,blue));
                }
            }
        }
    } else {
        irr::s32 thisX = Utilities::round(centreX);
        irr::s32 thisY = Utilities::round(centreY);
        //Find distance from centre
        irr::s32 centreToX = thisX - radarRadiusPx;
        irr::s32 centreToY = thisY - radarRadiusPx;
        if (pow(centreToX,2) + pow(centreToY,2) <= radiusSquared) {
            if (thisX >= 0 && thisY >= 0) {
                radarImage->setPixel(thisX,thisY,irr::video::SColor(alpha,red,green,blue));
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
