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

#ifdef WITH_PROFILING
#include "iprof.hpp"
#else
#define IPROF(a) //intentionally empty placeholder
#endif

////using namespace irr;

namespace {
    inline irr::core::vector3df toIrrVec(const bc::graphics::Vec3& v) { return {v.x, v.y, v.z}; }
}

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
    cursorRangeXNm = 0;
    cursorRangeYNm = 0;

    radarCursorsLastUpdated = clock();

    radarOn = true;

    //Radar modes: North up (false, false). Course up (true, true). Head up (true, false)
    headUp = false;
    stabilised = false;
    trueVectors = true;
    vectorLengthMinutes = 6;
    arpaMode = 0;

    // What's selected in the GUI arpa list
    arpaListSelection = -1;

    //Hard coded in GUI and here for 10 parallel index lines
    for(uint32_t i=0; i<10; i++) {
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
    uint32_t rangeResolution_max = IniFile::iniFileTou32(iniFilename, "RADAR_RangeRes_Max");
    uint32_t angularResolution_max = IniFile::iniFileTou32(iniFilename, "RADAR_AngularRes_Max");
    
    //Load parameters from the radarConfig file (if it exists)
    uint32_t numberOfRadarRanges = IniFile::iniFileTou32(radarConfigFile,"NumberOfRadarRanges");
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

        irr::video::SColor radarBackgroundColour;
        irr::video::SColor radarForegroundColour;
        irr::video::SColor radarSurroundColour;

        radarBackgroundColour.setAlpha(255);
        radarBackgroundColour.setRed(0);
        radarBackgroundColour.setGreen(0);
        radarBackgroundColour.setBlue(200);

        radarForegroundColour.setAlpha(255);
        radarForegroundColour.setRed(255);
        radarForegroundColour.setGreen(220);
        radarForegroundColour.setBlue(0);

        radarSurroundColour.setAlpha(255);
        radarSurroundColour.setRed(128);
        radarSurroundColour.setGreen(128);
        radarSurroundColour.setBlue(128);

        
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
            float thisRadarRange = IniFile::iniFileTof32(radarConfigFile,IniFile::enumerate1("RadarRange",i));
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

        uint32_t numberOfRadarColourSets = IniFile::iniFileTof32(radarConfigFile,"NumberOfRadarColourSets");
        if (numberOfRadarColourSets == 0) {
            //legacy loading
            irr::video::SColor radarBackgroundColour;
            irr::video::SColor radarForegroundColour;
            irr::video::SColor radarSurroundColour;

            radarBackgroundColour.setAlpha(255);
            radarBackgroundColour.setRed(IniFile::iniFileTou32(radarConfigFile,"radar_bg_red"));
            radarBackgroundColour.setGreen(IniFile::iniFileTou32(radarConfigFile,"radar_bg_green"));
            radarBackgroundColour.setBlue(IniFile::iniFileTou32(radarConfigFile,"radar_bg_blue"));

            radarForegroundColour.setAlpha(255);
            radarForegroundColour.setRed(IniFile::iniFileTou32(radarConfigFile,"radar1_red"));
            radarForegroundColour.setGreen(IniFile::iniFileTou32(radarConfigFile,"radar1_green"));
            radarForegroundColour.setBlue(IniFile::iniFileTou32(radarConfigFile,"radar1_blue"));

            radarSurroundColour.setAlpha(255);
            radarSurroundColour.setRed(IniFile::iniFileTou32(radarConfigFile, "radar_surround_red", 128));
            radarSurroundColour.setGreen(IniFile::iniFileTou32(radarConfigFile, "radar1_green", 128));
            radarSurroundColour.setBlue(IniFile::iniFileTou32(radarConfigFile, "radar1_blue", 128));

            radarBackgroundColours.push_back(radarBackgroundColour);
            radarForegroundColours.push_back(radarForegroundColour);
            radarSurroundColours.push_back(radarSurroundColour);
        } else {
            for (unsigned int i = 1; i <= numberOfRadarColourSets; i++) {
                irr::video::SColor radarBackgroundColour;
                irr::video::SColor radarForegroundColour;
                irr::video::SColor radarSurroundColour;

                radarBackgroundColour.setAlpha(255);
                radarBackgroundColour.setRed(IniFile::iniFileTou32(radarConfigFile,IniFile::enumerate1("radar_bg_red",i)));
                radarBackgroundColour.setGreen(IniFile::iniFileTou32(radarConfigFile,IniFile::enumerate1("radar_bg_green",i)));
                radarBackgroundColour.setBlue(IniFile::iniFileTou32(radarConfigFile,IniFile::enumerate1("radar_bg_blue",i)));

                radarForegroundColour.setAlpha(255);
                radarForegroundColour.setRed(IniFile::iniFileTou32(radarConfigFile,IniFile::enumerate1("radar1_red",i)));
                radarForegroundColour.setGreen(IniFile::iniFileTou32(radarConfigFile,IniFile::enumerate1("radar1_green",i)));
                radarForegroundColour.setBlue(IniFile::iniFileTou32(radarConfigFile,IniFile::enumerate1("radar1_blue",i)));

                radarSurroundColour.setAlpha(255);
                radarSurroundColour.setRed(IniFile::iniFileTou32(radarConfigFile, IniFile::enumerate1("radar_surround_red", i), 128));
                radarSurroundColour.setGreen(IniFile::iniFileTou32(radarConfigFile, IniFile::enumerate1("radar_surround_green", i), 128));
                radarSurroundColour.setBlue(IniFile::iniFileTou32(radarConfigFile, IniFile::enumerate1("radar_surround_blue", i), 128));

                radarBackgroundColours.push_back(radarBackgroundColour);
                radarForegroundColours.push_back(radarForegroundColour);    
                radarSurroundColours.push_back(radarSurroundColour);
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
    scanArray.resize(angularResolution,std::vector<float>(rangeResolution,0.0));
    scanArrayAmplified.resize(angularResolution,std::vector<float>(rangeResolution,0.0));
    scanArrayToPlot.resize(angularResolution,std::vector<float>(rangeResolution,0.0));
    scanArrayToPlotPrevious.resize(angularResolution,std::vector<float>(rangeResolution,0.0));
    toReplot.resize(angularResolution);

    //initialise arrays
    for(uint32_t i = 0; i<angularResolution; i++) {
        for(uint32_t j = 0; j<rangeResolution; j++) {
            scanArray[i][j] = 0.0;
            scanArrayAmplified[i][j] = 0.0;
            scanArrayToPlot[i][j] = 0.0;
            scanArrayToPlotPrevious[i][j] = -1.0;
        }
    }

    scanAngleStep = 360.0f / (float) angularResolution;
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

float RadarCalculation::getRangeNm() const
{
    return radarRangeNm.at(radarRangeIndex); //Assume that radarRangeIndex is in bounds
}

void RadarCalculation::setGain(float value)
{
    radarGain = value;
}

void RadarCalculation::setClutter(float value)
{
    radarSeaClutterReduction = value;
}

void RadarCalculation::setRainClutter(float value)
{
    radarRainClutterReduction = value;
}

float RadarCalculation::getGain() const
{
    return radarGain;
}

float RadarCalculation::getClutter() const
{
    return radarSeaClutterReduction;
}

float RadarCalculation::getRainClutter() const
{
    return radarRainClutterReduction;
}

void RadarCalculation::increaseClutter(float value)
{
    radarSeaClutterReduction += value;
    if (radarSeaClutterReduction > 100) {
        radarSeaClutterReduction = 100;
    }
}

void RadarCalculation::decreaseClutter(float value)
{
    radarSeaClutterReduction -= value;
    if (radarSeaClutterReduction < 0) {
        radarSeaClutterReduction = 0;
    }
}

void RadarCalculation::increaseRainClutter(float value)
{
    radarRainClutterReduction += value;
    if (radarRainClutterReduction > 100) {
        radarRainClutterReduction = 100;
    }
}

void RadarCalculation::decreaseRainClutter(float value)
{
    radarRainClutterReduction -= value;
    if (radarRainClutterReduction < 0) {
        radarRainClutterReduction = 0;
    }
}

void RadarCalculation::increaseGain(float value)
{
    radarGain += value;
    if (radarGain > 100) {
        radarGain = 100;
    }
}

void RadarCalculation::decreaseGain(float value)
{
    radarGain -= value;
    if (radarGain < 0) {
        radarGain = 0;
    }
}

float RadarCalculation::getEBLRangeNm() const
{
    return EBLRangeNm;
}

float RadarCalculation::getCursorBrg() const
{
    return CursorBrg;
}

float RadarCalculation::getCursorRangeNm() const
{
    return CursorRangeNm;
}

float RadarCalculation::getEBLBrg() const
{
    return EBLBrg;
}

void RadarCalculation::setPIData(int32_t PIid, float PIbearing, float PIrange)
{
    if (PIid >= 0 && PIid < (int32_t)piBearings.size() && PIid < (int32_t)piRanges.size()) {
        piBearings.at(PIid) = PIbearing;
        piRanges.at(PIid) = PIrange;
    }
}

float RadarCalculation::getPIbearing(int32_t PIid) const
{
    if (PIid >= 0 && PIid < (int32_t)piBearings.size()) {
        return piBearings.at(PIid);
    } else {
        return 0;
    }
}

float RadarCalculation::getPIrange(int32_t PIid) const
{
    if (PIid >= 0 && PIid < (int32_t)piRanges.size()) {
        return piRanges.at(PIid);
    } else {
        return 0;
    }
}

void RadarCalculation::increaseCursorRangeXNm()
{
    //Only trigger this if there's been enough time since the last update.
    clock_t clockNow = clock();
    float elapsed = (float)(clockNow - radarCursorsLastUpdated)/CLOCKS_PER_SEC;
    if (elapsed > 0.03) {
        radarCursorsLastUpdated = clockNow;
        float oldCursorRangeXNm = cursorRangeXNm;
        cursorRangeXNm += getRangeNm()/100;

        // Limit: 
        float testCursorRangeNm = pow(pow(cursorRangeXNm,2)+pow(cursorRangeYNm,2),0.5);
        if (testCursorRangeNm > getRangeNm()) {
            float testCursorBrgRad = std::atan2(oldCursorRangeXNm,cursorRangeYNm);
            cursorRangeXNm = getRangeNm() * sin(testCursorBrgRad);
            cursorRangeYNm = getRangeNm() * cos(testCursorBrgRad);
        }
    }
}

void RadarCalculation::decreaseCursorRangeXNm()
{
    //Only trigger this if there's been enough time since the last update.
    clock_t clockNow = clock();
    float elapsed = (float)(clockNow - radarCursorsLastUpdated)/CLOCKS_PER_SEC;
    if (elapsed > 0.03) {
        radarCursorsLastUpdated = clockNow;
        float oldCursorRangeXNm = cursorRangeXNm;
        cursorRangeXNm -= getRangeNm()/100;
        // Limit: 
        float testCursorRangeNm = pow(pow(cursorRangeXNm,2)+pow(cursorRangeYNm,2),0.5);
        if (testCursorRangeNm > getRangeNm()) {
            float testCursorBrgRad = std::atan2(oldCursorRangeXNm,cursorRangeYNm);
            cursorRangeXNm = getRangeNm() * sin(testCursorBrgRad);
            cursorRangeYNm = getRangeNm() * cos(testCursorBrgRad);
        }
    }
}

void RadarCalculation::increaseCursorRangeYNm()
{
    //Only trigger this if there's been enough time since the last update.
    clock_t clockNow = clock();
    float elapsed = (float)(clockNow - radarCursorsLastUpdated)/CLOCKS_PER_SEC;
    if (elapsed > 0.03) {
        radarCursorsLastUpdated = clockNow;
        float oldCursorRangeYNm = cursorRangeYNm;
        cursorRangeYNm += getRangeNm()/100;
        // Limit: 
        float testCursorRangeNm = pow(pow(cursorRangeXNm,2)+pow(cursorRangeYNm,2),0.5);
        if (testCursorRangeNm > getRangeNm()) {
            float testCursorBrgRad = std::atan2(cursorRangeXNm,oldCursorRangeYNm);
            cursorRangeXNm = getRangeNm() * sin(testCursorBrgRad);
            cursorRangeYNm = getRangeNm() * cos(testCursorBrgRad);
        }
    }
}

void RadarCalculation::decreaseCursorRangeYNm()
{
    //Only trigger this if there's been enough time since the last update.
    clock_t clockNow = clock();
    float elapsed = (float)(clockNow - radarCursorsLastUpdated)/CLOCKS_PER_SEC;
    if (elapsed > 0.03) {
        radarCursorsLastUpdated = clockNow;
        float oldCursorRangeYNm = cursorRangeYNm;
        cursorRangeYNm -= getRangeNm()/100;
        // Limit: 
        float testCursorRangeNm = pow(pow(cursorRangeXNm,2)+pow(cursorRangeYNm,2),0.5);
        if (testCursorRangeNm > getRangeNm()) {
            float testCursorBrgRad = std::atan2(cursorRangeXNm,oldCursorRangeYNm);
            cursorRangeXNm = getRangeNm() * sin(testCursorBrgRad);
            cursorRangeYNm = getRangeNm() * cos(testCursorBrgRad);
        }
    }
}

void RadarCalculation::increaseEBLRange()
{
    //Only trigger this if there's been enough time since the last update.
    clock_t clockNow = clock();
    float elapsed = (float)(clockNow - radarCursorsLastUpdated)/CLOCKS_PER_SEC;
    if (elapsed > 0.03) {
        radarCursorsLastUpdated = clockNow;

        EBLRangeNm += getRangeNm()/100;

    }
}

void RadarCalculation::decreaseEBLRange()
{
    //Only trigger this if there's been enough time since the last update.
    clock_t clockNow = clock();
    float elapsed = (float)(clockNow - radarCursorsLastUpdated)/CLOCKS_PER_SEC;
    if (elapsed > 0.03) {
        radarCursorsLastUpdated = clockNow;

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
    float elapsed = (float)(clockNow - radarCursorsLastUpdated)/CLOCKS_PER_SEC;
    if (elapsed > 0.03) {
        radarCursorsLastUpdated = clockNow;

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
    float elapsed = (float)(clockNow - radarCursorsLastUpdated)/CLOCKS_PER_SEC;
    if (elapsed > 0.03) {
        radarCursorsLastUpdated = clockNow;

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
		for (uint32_t i = 0; i < angularResolution; i++) {
			for (uint32_t j = 0; j < rangeResolution; j++) {
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

int RadarCalculation::getArpaMode() const
{
    return arpaMode;
}

void RadarCalculation::setArpaMode(int mode)
{
    // 0: Off/Manual, 1: MARPA, 2: ARPA
    arpaMode = mode;
    if (arpaMode < 1) {
        // Clear arpa scans:
        // Remove all ARPA (not manual) contacts from arpaContacts. Clear arpaTracks, but reset estimate.displayID to 0 for manual contacts, so it is regenerated 
        for (int i = arpaContacts.size() - 1; i >= 0; i--) {
            // Iterate from end to start, as we may be removing items
            if (arpaContacts.at(i).contactType == CONTACT_MANUAL) {
                arpaContacts.at(i).estimate.displayID = 0;
            } else {
                // Remove it
                arpaContacts.erase(arpaContacts.begin() + i);
            }
        }
        arpaTracks.clear(); // This will be regenerated as we have set display ID to 0
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

void RadarCalculation::setArpaListSelection(int32_t selection) 
{
    arpaListSelection = selection;
}

int32_t RadarCalculation::getArpaListSelection() const
{
    return arpaListSelection;
}

void RadarCalculation::setRadarARPAVectors(float vectorMinutes)
{
    vectorLengthMinutes = vectorMinutes;
}

void RadarCalculation::setRadarDisplayRadius(uint32_t radiusPx)
{
    if (radarRadiusPx != radiusPx) { //If changed
        radarRadiusPx = radiusPx;
        radarScreenStale=true;
    }
}

uint32_t RadarCalculation::getARPATracksSize() const
{
    return arpaTracks.size();
}

int RadarCalculation::getARPAContactIDFromTrackIndex(uint32_t trackIndex) const
{
    if (trackIndex >= 0 && trackIndex < arpaTracks.size()) {
        return arpaTracks.at(trackIndex);
    } else {
        // Not found
        return -1;
    }
}

ARPAContact RadarCalculation::getARPAContactFromTrackIndex(uint32_t trackIndex) const
{
    
    int contactID = getARPAContactIDFromTrackIndex(trackIndex);

    if(contactID >=0 && contactID < arpaContacts.size()){
        return arpaContacts.at(contactID);
    } else {
        ARPAContact emptyContact;
        return emptyContact;
    }
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

void RadarCalculation::update(irr::video::IImage * radarImage, irr::video::IImage * radarImageOverlaid, irr::core::vector3d<int64_t> offsetPosition, const Terrain& terrain, const OwnShip& ownShip, const Buoys& buoys, const OtherShips& otherShips, float weather, float rain, float tideHeight, float deltaTime, uint64_t absoluteTime, irr::core::vector2di mouseRelPosition, bool isMouseDown)
{

    #ifdef WITH_PROFILING
    IPROF_FUNC;
    #endif

    { IPROF("Set up");

    //Reset screen if needed
    if(radarScreenStale) {
        radarImage->fill(getRadarSurroundColour());
        //Reset 'previous' array so it will all get re-drawn
        for(uint32_t i = 0; i<angularResolution; i++) {
            toReplot[i] = true;
            for(uint32_t j = 0; j<rangeResolution; j++) {
                scanArrayToPlotPrevious[i][j] = -1.0;
            }
        }
        radarScreenStale = false;
    }

    } { IPROF("Mouse cursor");

    //Find position of mouse cursor for radar cursor
    if (isMouseDown) {
        float mouseCursorRangeXNm = (float)mouseRelPosition.X/(float)radarRadiusPx*radarRangeNm.at(radarRangeIndex);//Nm
        float mouseCursorRangeYNm = -1.0*(float)mouseRelPosition.Y/(float)radarRadiusPx*radarRangeNm.at(radarRangeIndex);//Nm
        float mouseCursorRange = pow(pow(mouseCursorRangeXNm,2)+pow(mouseCursorRangeYNm,2),0.5);
        
        //Check if in range
        if (mouseCursorRange <= radarRangeNm.at(radarRangeIndex) ) {
            // Store
            cursorRangeXNm = mouseCursorRangeXNm;
            cursorRangeYNm = mouseCursorRangeYNm;
        }
    }

    // Always update the CursorRangeNm and CursorBrg from the current cursorRangeXNm and cursorRangeYNm 
    CursorBrg = irr::core::RADTODEG*std::atan2(cursorRangeXNm,cursorRangeYNm);
    if (headUp) {
        // Adjust angle if needed
        CursorBrg += ownShip.getHeading();
    }
    CursorBrg = Angles::normaliseAngle(CursorBrg);
    CursorRangeNm = pow(pow(cursorRangeXNm,2)+pow(cursorRangeYNm,2),0.5);

    } { IPROF("Scan");
    scan(offsetPosition, terrain, ownShip, buoys, otherShips, weather, rain, tideHeight, deltaTime, absoluteTime); // scan into scanArray[row (angle)][column (step)], and with filtering and amplification into scanArrayAmplified[][]
    } { IPROF("Update ARPA");
	updateARPA(offsetPosition, ownShip, absoluteTime); //From data in arpaContacts, updated in scan()
	} { IPROF("Render");
	render(radarImage, radarImageOverlaid, ownShip.getCOG(), ownShip.getSOG()); //From scanArrayAmplified[row (angle)][column (step)], render to radarImage
	}

}


void RadarCalculation::scan(irr::core::vector3d<int64_t> offsetPosition, const Terrain& terrain, const OwnShip& ownShip, const Buoys& buoys, const OtherShips& otherShips, float weather, float rain, float tideHeight, float deltaTime, uint64_t absoluteTime)
{

    //IPROF_FUNC;
    const uint32_t SECONDS_BETWEEN_SCANS = 2;

    bc::graphics::Vec3 position = ownShip.getPosition();
    //Get absolute position relative to SW corner of world model
    irr::core::vector3d<int64_t> absolutePosition = offsetPosition;
    absolutePosition.X += position.x;
    absolutePosition.Y += position.y;
    absolutePosition.Z += position.z;

    //Some tuning constants
    float radarFactorLand=2.0;
    float radarFactorVessel=0.0001;

    //Convert range to cell size
    float cellLength = M_IN_NM*radarRangeNm.at(radarRangeIndex)/rangeResolution; ; //Assume that radarRangeIndex is in bounds

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

    const float RADAR_RPM = 25; //Todo: Make a ship parameter
    const float RPMtoDEGPERSECOND = 6;
    uint32_t scansPerLoop = RADAR_RPM * RPMtoDEGPERSECOND * deltaTime / (float) scanAngleStep + (float) rand() / RAND_MAX ; //Add random value (0-1, mean 0.5), so with rounding, we get the correct radar speed, even though we can only do an integer number of scans

    if (scansPerLoop > 30) {scansPerLoop = 30;} //Limit to reasonable bounds
    for(uint32_t i = 0; i<scansPerLoop;i++) { //Start of repeatable scan section

        // the actual angle we want to work with has to be determined here
        currentScanAngle = ((float) currentScanLine / (float) angularResolution) * 360.0f;

        float scanSlope = -0.5; //Slope at start of scan (in metres/metre) - Make slightly negative so vessel contacts close in get detected
        for (uint32_t currentStep = 1; currentStep<rangeResolution; currentStep++) { //Note that currentStep starts as 1, not 0. This is used in anti-rain clutter filter, which checks element at currentStep-1
            //scan into array, accessed as  scanArray[row (angle)][column (step)]

            //Clear old value
            scanArray[currentScanLine][currentStep] = 0.0;

            //Get location of area being scanned
            float localRange = cellLength*currentStep;
            float relX = localRange*sin(currentScanAngle*irr::core::DEGTORAD); //Distance from ship
            float relZ = localRange*cos(currentScanAngle*irr::core::DEGTORAD);
            float localX = position.x + relX;
            float localZ = position.z + relZ;

            //get extents
            float minCellAngle = Angles::normaliseAngle(currentScanAngle - scanAngleStep/2.0);
            float maxCellAngle = Angles::normaliseAngle(currentScanAngle + scanAngleStep/2.0);
            float minCellRange = localRange - cellLength/2.0;
            float maxCellRange = localRange + cellLength/2.0;

            // Get extreme points
            float relXCorner1 = minCellRange*sin(minCellAngle*irr::core::DEGTORAD);
            float relXCorner2 = minCellRange*sin(maxCellAngle*irr::core::DEGTORAD);
            float relXCorner3 = maxCellRange*sin(minCellAngle*irr::core::DEGTORAD);
            float relXCorner4 = maxCellRange*sin(maxCellAngle*irr::core::DEGTORAD);
            float relZCorner1 = minCellRange*cos(minCellAngle*irr::core::DEGTORAD);
            float relZCorner2 = minCellRange*cos(maxCellAngle*irr::core::DEGTORAD);
            float relZCorner3 = maxCellRange*cos(minCellAngle*irr::core::DEGTORAD);
            float relZCorner4 = maxCellRange*cos(maxCellAngle*irr::core::DEGTORAD);

            //get adjustment of height for earth's curvature
            float dropWithCurvature = std::pow(localRange,2)/(2*EARTH_RAD_M*EARTH_RAD_CORRECTION);

            //Calculate noise
            float localNoise = radarNoise(radarNoiseLevel,radarSeaClutter,radarRainClutter,weather,localRange,currentScanAngle,0,scanSlope,rain); //FIXME: Needs wind direction

            //Scan other contacts here
            for(unsigned int thisContact = 0; thisContact<radarData.size(); thisContact++) {
                float contactHeightAboveLine = (radarData.at(thisContact).height - radarScannerHeight - dropWithCurvature) - scanSlope*localRange;
                if (contactHeightAboveLine > 0) {
                    //Contact would be visible if in this cell. Check if it is
                    
                    //Ellipse based check - check if any corner point of the cell is within the contact ellipse. If so, then it's definitely visible. If not, fall back to old checks
                    bool contactEllipseFound = false;
                    
                    if (isPointInEllipse(relX, relZ, radarData.at(thisContact).relX, radarData.at(thisContact).relZ, radarData.at(thisContact).width, radarData.at(thisContact).length, radarData.at(thisContact).heading)) {
                        contactEllipseFound = true;
                    } else if (isPointInEllipse(relXCorner1, relZCorner1, radarData.at(thisContact).relX, radarData.at(thisContact).relZ, radarData.at(thisContact).width, radarData.at(thisContact).length, radarData.at(thisContact).heading)) {
                        contactEllipseFound = true;
                    } else if (isPointInEllipse(relXCorner2, relZCorner2, radarData.at(thisContact).relX, radarData.at(thisContact).relZ, radarData.at(thisContact).width, radarData.at(thisContact).length, radarData.at(thisContact).heading)) {
                        contactEllipseFound = true;
                    } else if (isPointInEllipse(relXCorner3, relZCorner3, radarData.at(thisContact).relX, radarData.at(thisContact).relZ, radarData.at(thisContact).width, radarData.at(thisContact).length, radarData.at(thisContact).heading)) {
                        contactEllipseFound = true;
                    } else if (isPointInEllipse(relXCorner4, relZCorner4, radarData.at(thisContact).relX, radarData.at(thisContact).relZ, radarData.at(thisContact).width, radarData.at(thisContact).length, radarData.at(thisContact).heading)) {
                        contactEllipseFound = true;
                    }
                    

                    //Start of B3D code
                    //Check if centre of target within the cell. If not then check if Either min range or max range of contact is within the cell, or min and max span the cell
                    if (contactEllipseFound 
                            || (radarData.at(thisContact).range >= minCellRange && radarData.at(thisContact).range <= maxCellRange) 
                            || (radarData.at(thisContact).minRange >= minCellRange && radarData.at(thisContact).minRange <= maxCellRange)
                            || (radarData.at(thisContact).maxRange >= minCellRange && radarData.at(thisContact).maxRange <= maxCellRange) 
                            || (radarData.at(thisContact).minRange < minCellRange && radarData.at(thisContact).maxRange > maxCellRange)) {

                        //Check if centre of target within the cell. If not then check if either min angle or max angle of contact is within the cell, or min and max span the cell
                        if (contactEllipseFound 
                            || (Angles::isAngleBetween(radarData.at(thisContact).angle,minCellAngle,maxCellAngle)) 
                            || (Angles::isAngleBetween(radarData.at(thisContact).minAngle,minCellAngle,maxCellAngle))
                            || (Angles::isAngleBetween(radarData.at(thisContact).maxAngle,minCellAngle,maxCellAngle))
                            || (Angles::normaliseAngle(radarData.at(thisContact).minAngle-minCellAngle) > 270 && Angles::normaliseAngle(radarData.at(thisContact).maxAngle-maxCellAngle) < 90)) {

                            float rangeAtCellMin = rangeAtAngle(minCellAngle,radarData.at(thisContact).relX,radarData.at(thisContact).relZ,radarData.at(thisContact).heading);
                            float rangeAtCellMax = rangeAtAngle(maxCellAngle,radarData.at(thisContact).relX,radarData.at(thisContact).relZ,radarData.at(thisContact).heading);

                            //check if the contact intersects this exact cell, if its extremes overlap it
                            //Also check if the target centre is in the cell, or the extended target spans the cell (ie RangeAtCellMin less than minCellRange and rangeAtCellMax greater than maxCellRange and vice versa)
                            if (contactEllipseFound 
                                    || (((radarData.at(thisContact).range >= minCellRange && radarData.at(thisContact).range <= maxCellRange) 
                                            && (Angles::isAngleBetween(radarData.at(thisContact).angle,minCellAngle,maxCellAngle)))
                                        || (rangeAtCellMin >= minCellRange && rangeAtCellMin <= maxCellRange)
                                        || (rangeAtCellMax >= minCellRange && rangeAtCellMax <= maxCellRange)
                                        || (rangeAtCellMin < minCellRange && rangeAtCellMax > maxCellRange)
                                        || (rangeAtCellMax < minCellRange && rangeAtCellMin > maxCellRange))) {

                                float radarEchoStrength = radarFactorVessel * std::pow(M_IN_NM/localRange,4) * radarData.at(thisContact).rcs;
                                scanArray[currentScanLine][currentStep] += radarEchoStrength;

                                //Start ARPA section
                                // ARPA mode - 0: Off/Manual, 1: MARPA, 2: ARPA
                                if (arpaMode > 0 && radarEchoStrength*2 > localNoise) {
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
                                        float angleUncertainty = scanAngleStep/2.0 * (2.0*(float)rand()/RAND_MAX - 1);
                                        float rangeUncertainty = rangeSensitivity * (2.0*(float)rand()/RAND_MAX - 1)/M_IN_NM;

                                        newScan.bearingDeg = angleUncertainty + radarData.at(thisContact).angle;
                                        newScan.rangeNm = rangeUncertainty + radarData.at(thisContact).range / M_IN_NM;

                                        newScan.x = absolutePosition.X + newScan.rangeNm*M_IN_NM * sin(newScan.bearingDeg*RAD_IN_DEG);
                                        newScan.z = absolutePosition.Z + newScan.rangeNm*M_IN_NM * cos(newScan.bearingDeg*RAD_IN_DEG);;
                                        //newScan.estimatedRCS = 100;//Todo: Implement

                                        //Keep track of estimated total movement if in full ARPA
                                        // 0: Off/Manual, 1: MARPA, 2: ARPA
                                        if (scansSize > 0 && arpaMode == 2) {
                                            arpaContacts.at(existingArpaContact).totalXMovementEst += arpaContacts.at(existingArpaContact).scans.at(scansSize-1).x - newScan.x;
                                            arpaContacts.at(existingArpaContact).totalZMovementEst += arpaContacts.at(existingArpaContact).scans.at(scansSize-1).z - newScan.z;
                                        } else {
                                            arpaContacts.at(existingArpaContact).totalXMovementEst = 0;
                                            arpaContacts.at(existingArpaContact).totalZMovementEst = 0;
                                        }

                                        if (arpaContacts.at(existingArpaContact).estimate.stationary) {
                                            // If stationary, don't keep previous scans (we are about to add the most recent)
                                            arpaContacts.at(existingArpaContact).scans.clear();
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
            float terrainHeightAboveSea = terrain.getHeight(localX,localZ) - tideHeight;
            float radarHeight = terrainHeightAboveSea - dropWithCurvature - radarScannerHeight;
            float localSlope = radarHeight/localRange;
            float heightAboveLine = radarHeight - scanSlope*localRange; //Find height above previous maximum scan slope

            if (heightAboveLine>0 && terrainHeightAboveSea>0) {
                float radarLocalGradient = heightAboveLine/cellLength;
                scanSlope = localSlope; //Highest so far on scan
                scanArray[currentScanLine][currentStep] += radarFactorLand*std::atan(radarLocalGradient)*(2/PI)/std::pow(localRange/M_IN_NM,3); //make a reflection off a plane wall at 1nm have a magnitude of 1*radarFactorLand
            }

            //Add radar noise
            scanArray[currentScanLine][currentStep] += localNoise;

            //Do amplification: scanArrayAmplified between 0 and 1 will set displayed intensity, values above 1 will be limited at max intensity

            //Calculate from parameters
            //localRange is range in metres
            float rainFilter = pow(radarRainClutterReduction/100.0,0.1);
            float maxSTCdistance = 8*M_IN_NM*radarSeaClutterReduction/100.0; //This sets the distance at which the swept gain control becomes 1, and is 8Nm at full reduction
            float radarSTCGain;
            if(maxSTCdistance>0) {
                radarSTCGain = pow(localRange/maxSTCdistance,3);
                if (radarSTCGain > 1) {radarSTCGain=1;} //Gain should never be increased (above 1.0)
            } else {
                radarSTCGain = 1;
            }

            //calculate high pass filter
            float intensityGradient = scanArray[currentScanLine][currentStep] - scanArray[currentScanLine][currentStep-1];
            if (intensityGradient<0) {intensityGradient=0;}

            float filteredSignal = intensityGradient*rainFilter + scanArray[currentScanLine][currentStep]*(1-rainFilter);
            float radarLocalGain = 500000*(8*pow(radarGain/100.0,4)) * radarSTCGain ;

            //take log (natural) of signal
            float logSignal = log(filteredSignal*radarLocalGain);
            scanArrayAmplified[currentScanLine][currentStep] = std::max(0.0f,logSignal);

            //Generate a filtered version, based on the angles around. Lag behind by (for example) 3 steps, so we can filter on what's ahead, as well as what's behind
            int32_t filterAngle = (int32_t)currentScanLine - 3;
                while(filterAngle < 0) {filterAngle+=angularResolution;}
                while(filterAngle >= angularResolution) {filterAngle-=angularResolution;}
            int32_t filterAngle_1 = filterAngle - 3;
                while(filterAngle_1 < 0) {filterAngle_1+=angularResolution;}
                while(filterAngle_1 >= angularResolution) {filterAngle_1-=angularResolution;}
            int32_t filterAngle_2 = filterAngle - 2;
                while(filterAngle_2 < 0) {filterAngle_2+=angularResolution;}
                while(filterAngle_2 >= angularResolution) {filterAngle_2-=angularResolution;}
            int32_t filterAngle_3 = filterAngle - 1;
                while(filterAngle_3 < 0) {filterAngle_3+=angularResolution;}
                while(filterAngle_3 >= angularResolution) {filterAngle_3-=angularResolution;}
            int32_t filterAngle_4 = filterAngle;
                while(filterAngle_4 < 0) {filterAngle_4+=angularResolution;}
                while(filterAngle_4 >= angularResolution) {filterAngle_4-=angularResolution;}
            int32_t filterAngle_5 = filterAngle + 1;
                while(filterAngle_5 < 0) {filterAngle_5+=angularResolution;}
                while(filterAngle_5 >= angularResolution) {filterAngle_5-=angularResolution;}
            int32_t filterAngle_6 = filterAngle + 2;
                while(filterAngle_6 < 0) {filterAngle_6+=angularResolution;}
                while(filterAngle_6 >= angularResolution) {filterAngle_6-=angularResolution;}
            int32_t filterAngle_7 = filterAngle + 3;
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

void RadarCalculation::addManualPoint(bool newContact, irr::core::vector3d<int64_t> offsetPosition, const OwnShip& ownShip, uint64_t absoluteTime)
{
    // Assumes that CursorRangeNm and CursorBrg reflect the current cursor point
    
    int existingArpaContact=-1;
    
    if (newContact) {
        ARPAContact newContact;
        newContact.contact = 0; // This is a pointer used for ARPA types, not relevant for manual
        newContact.contactType=CONTACT_MANUAL;
        //newContact.displayID = 0; //Initially not displayed
        newContact.totalXMovementEst = 0; // These are also not used for manual
        newContact.totalZMovementEst = 0; // These are also not used for manual

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

        //std::cout << "Created new contact, existingArpaContact now = " << existingArpaContact << std::endl;
    } else {
        // Update existing selected manual Contact:

        // Find if a manual contact is selected
        if (getARPAContactFromTrackIndex(arpaListSelection).contactType == CONTACT_MANUAL) {
            existingArpaContact = getARPAContactIDFromTrackIndex(arpaListSelection); 
        }
        if (existingArpaContact < 0) {
            // No contact found, don't do anything
            return;
        }
    }

    // Set up
    bc::graphics::Vec3 position = ownShip.getPosition();
    // Get absolute position relative to SW corner of world model
    irr::core::vector3d<int64_t> absolutePosition = offsetPosition;
    absolutePosition.X += position.x;
    absolutePosition.Y += position.y;
    absolutePosition.Z += position.z;

    //Add this 'scan' (Actually a MARPA Update)
    ARPAScan newScan;
    newScan.timeStamp = absoluteTime;

    //Don't add noise/uncertainty
    newScan.bearingDeg = CursorBrg;
    newScan.rangeNm = CursorRangeNm;

    newScan.x = absolutePosition.X + newScan.rangeNm*M_IN_NM * sin(newScan.bearingDeg*RAD_IN_DEG);
    newScan.z = absolutePosition.Z + newScan.rangeNm*M_IN_NM * cos(newScan.bearingDeg*RAD_IN_DEG);;
    //newScan.estimatedRCS = 100;//Todo: Implement

    //Don't need to keep track of totalXMovementEst and totalZMovementEst for manual
    
    arpaContacts.at(existingArpaContact).scans.push_back(newScan);
    //Todo: should we limit the size of this, so it doesn't continue accumulating?
}

void RadarCalculation::clearManualPoints()
{
    int existingArpaContact=-1;
    if (getARPAContactFromTrackIndex(arpaListSelection).contactType == CONTACT_MANUAL) {
        existingArpaContact = getARPAContactIDFromTrackIndex(arpaListSelection); 
    }

    if (existingArpaContact >= 0) {
        // Found the contact, remove all scans. Estimate will be regenerated later.
        arpaContacts.at(existingArpaContact).scans.clear();
    }

}

void RadarCalculation::clearTargetFromCursor()
{
    // Assumes that CursorRangeNm and CursorBrg reflect the current cursor point
    
    float cursorRelX = CursorRangeNm*M_IN_NM * sin(CursorBrg*RAD_IN_DEG);
    float cursorRelZ = CursorRangeNm*M_IN_NM * cos(CursorBrg*RAD_IN_DEG);
    
    // Iterate through ARPA contacts and find closest. 
    // If none within 1/10th of radar rang, don't do anything
    float closestDistance = getRangeNm()*M_IN_NM / 10.0;
    int closeContact = -1;
    for (int i = 0; i < arpaContacts.size(); i++) {
        float targetRelX = arpaContacts.at(i).estimate.range*M_IN_NM * sin(arpaContacts.at(i).estimate.bearing*RAD_IN_DEG);
        float targetRelZ = arpaContacts.at(i).estimate.range*M_IN_NM * cos(arpaContacts.at(i).estimate.bearing*RAD_IN_DEG);

        float targetRelXDiff = targetRelX - cursorRelX;
        float targetRelZDiff = targetRelZ - cursorRelZ;
        
        // Only check if tracked (i.e. if not marked as 'stationary')
        if (arpaContacts.at(i).estimate.stationary == false) {
            float targetRelDistance = std::sqrt(pow(targetRelXDiff,2)+pow(targetRelZDiff,2));
            if (targetRelDistance < closestDistance) {
                closeContact = i;
                closestDistance = targetRelDistance;    
            }
        }
    }

    if (closeContact >= 0 && closeContact < arpaContacts.size()) {
        // Clear pointer to underlying contact, and mark as stationary
        arpaContacts.at(closeContact).estimate.stationary = true;
        arpaContacts.at(closeContact).contact = 0;
    }

}

void RadarCalculation::trackTargetFromCursor()
{
    // Assumes that CursorRangeNm and CursorBrg reflect the current cursor point
    
    float cursorRelX = CursorRangeNm*M_IN_NM * sin(CursorBrg*RAD_IN_DEG);
    float cursorRelZ = CursorRangeNm*M_IN_NM * cos(CursorBrg*RAD_IN_DEG);
    
    // Iterate through ARPA contacts and find closest. 
    // If none within 1/10th of radar rang, don't do anything
    float closestDistance = getRangeNm()*M_IN_NM / 10.0;
    int closeContact = -1;
    for (int i = 0; i < arpaContacts.size(); i++) {
        float targetRelX = arpaContacts.at(i).estimate.range*M_IN_NM * sin(arpaContacts.at(i).estimate.bearing*RAD_IN_DEG);
        float targetRelZ = arpaContacts.at(i).estimate.range*M_IN_NM * cos(arpaContacts.at(i).estimate.bearing*RAD_IN_DEG);

        float targetRelXDiff = targetRelX - cursorRelX;
        float targetRelZDiff = targetRelZ - cursorRelZ;
        
        // Only check if not already tracked (i.e. if marked as 'stationary')
        if (arpaContacts.at(i).estimate.stationary == true) {
            float targetRelDistance = std::sqrt(pow(targetRelXDiff,2)+pow(targetRelZDiff,2));
            if (targetRelDistance < closestDistance) {
                closeContact = i;
                closestDistance = targetRelDistance;    
            }
        }
    }

    if (closeContact >= 0 && closeContact < arpaContacts.size()) {
        arpaContacts.at(closeContact).estimate.stationary = false;
    }

}

void RadarCalculation::updateARPA(irr::core::vector3d<int64_t> offsetPosition, const OwnShip& ownShip, uint64_t absoluteTime)
{

    //IPROF_FUNC;
    //Own ship absolute position
    bc::graphics::Vec3 position = ownShip.getPosition();
    //Get absolute position relative to SW corner of world model
    irr::core::vector3d<int64_t> absolutePosition = offsetPosition;
    absolutePosition.X += position.x;
    absolutePosition.Y += position.y;
    absolutePosition.Z += position.z;

    //Based on scans data in arpaContacts, estimate current speed, heading and position
    for (unsigned int i = 0; i<arpaContacts.size(); i++) {
        updateArpaEstimate(arpaContacts.at(i), i, ownShip, absolutePosition, absoluteTime); //This will update the estimate etc.
    } //For loop through arpa contacts
}

void RadarCalculation::updateArpaEstimate(ARPAContact& thisArpaContact, int contactID, const OwnShip& ownShip, irr::core::vector3d<int64_t> absolutePosition, uint64_t absoluteTime) 
{
    if (arpaMode < 1 && thisArpaContact.contactType == CONTACT_NORMAL) {
        //Set all contacts to zero (untracked) if normal type if ARPA is off
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
    } else {
        if (thisArpaContact.scans.size() == 0) {
            // Reset estimate if there are no scans at all
            //thisArpaContact.estimate.displayID = 0; // Don't reset display ID, so contact can be re-used
            thisArpaContact.estimate.stationary = true;
            thisArpaContact.estimate.lost = false;
            thisArpaContact.estimate.absVectorX = 0;
            thisArpaContact.estimate.absVectorZ = 0;
            thisArpaContact.estimate.absHeading = 0;
            thisArpaContact.estimate.bearing = 0;
            thisArpaContact.estimate.range = 0;
            thisArpaContact.estimate.speed = 0;
            thisArpaContact.estimate.cpa = 0;
            thisArpaContact.estimate.tcpa = 0;
            thisArpaContact.estimate.contactType = thisArpaContact.contactType;
        } else {
            // At least one scan

            //Record the contact type in the estimate
            thisArpaContact.estimate.contactType = thisArpaContact.contactType;
            
            //Check stationary contacts to see if they've got detectable motion: TODO: Make this better: Should weight based on current range?
            //TODO: Test this weighting
            if (thisArpaContact.estimate.stationary) {
                float latestRangeNm =  thisArpaContact.scans.back().rangeNm;
                if (latestRangeNm < 1) {
                    latestRangeNm = 1;
                }
                float weightedMotionX = fabs(thisArpaContact.totalXMovementEst/latestRangeNm);
                float weightedMotionZ = fabs(thisArpaContact.totalZMovementEst/latestRangeNm);
                if (thisArpaContact.estimate.contactType == CONTACT_MANUAL || 
                    (weightedMotionX >= 100 || 
                    weightedMotionZ >= 100) ) {
                    // Always show for manually acquired targets, or if movement has been detected
                    thisArpaContact.estimate.stationary = false;
                }
            }

            // Check if contact lost, if last scanned more than 60 seconds ago 
            // (exception for manual, don't detect as lost)
            if ( absoluteTime - thisArpaContact.scans.back().timeStamp > 60 && 
                 thisArpaContact.estimate.contactType == CONTACT_NORMAL) {
                thisArpaContact.estimate.lost=true;
                //std::cout << "Contact " << i << " lost" << std::endl;
            } else {
                if (!thisArpaContact.estimate.stationary) {
                    //If ID is 0 (unassigned), set id and increment
                    if (thisArpaContact.estimate.displayID==0) {
                        arpaTracks.push_back(contactID);
                        thisArpaContact.estimate.displayID = getARPATracksSize(); // The display ID is the current size of the arpaTracks list.
                        // If a manual contact, make it the selected one
                        if (thisArpaContact.contactType == CONTACT_MANUAL) {
                            setArpaListSelection(thisArpaContact.estimate.displayID - 1); // Zero indexed list
                        } 
                    }
                }

                int32_t stepsBack = 60; //Default time for tracking (time = stepsBack * SECONDS_BETWEEN_SCANS)
                int32_t recentStepsBack = 10; //Shorter time for tracking (if motion has changed significantly)

                int32_t currentScanIndex = thisArpaContact.scans.size() - 1;
                int32_t referenceScanIndex = currentScanIndex - stepsBack;
                if (referenceScanIndex < 0) {
                    referenceScanIndex = 0;
                }

                ARPAScan currentScanData = thisArpaContact.scans.at(currentScanIndex);
                ARPAScan referenceScanData = thisArpaContact.scans.at(referenceScanIndex);

                //Check if heading/speed has changed dramatically, by taking last 10 scans. If so, reduce steps back to 10
                int32_t actualStepsBack = currentScanIndex - referenceScanIndex;
                if (actualStepsBack > recentStepsBack) {
                    ARPAScan recentScanData = thisArpaContact.scans.at(currentScanIndex-recentStepsBack);
                    float deltaTimeRecent = currentScanData.timeStamp - recentScanData.timeStamp;
                    float deltaTimeFull   = currentScanData.timeStamp - referenceScanData.timeStamp;
                    if (deltaTimeRecent>0 && deltaTimeFull > 0) {
                        float deltaXRecent = currentScanData.x - recentScanData.x;
                        float deltaZRecent = currentScanData.z - recentScanData.z;
                        float deltaXFull   = currentScanData.x - referenceScanData.x;
                        float deltaZFull   = currentScanData.z - referenceScanData.z;

                        //Absolute vector
                        float absVectorXRecent = deltaXRecent/deltaTimeRecent; //m/s
                        float absVectorZRecent = deltaZRecent/deltaTimeRecent; //m/s
                        float absVectorXFull = deltaXFull/deltaTimeFull; //m/s
                        float absVectorZFull = deltaZFull/deltaTimeFull; //m/s

                        //Difference in estimation
                        float changeX = absVectorXRecent - absVectorXFull;
                        float changeZ = absVectorZRecent - absVectorZFull;

                        //If speed estimates differ by more than 1m/s in either direction, prefer the more recent estimate. Otherwise, leave unchanged
                        if (std::abs(changeX) > 1.0 || std::abs(changeZ) > 1.0) {
                            referenceScanData = recentScanData; //It seems like the course or speed has changed significantly
                        }
                    }
                }

                //Find difference in time, position x, position z
                float deltaTime = currentScanData.timeStamp - referenceScanData.timeStamp;
                if (deltaTime<=0) {
                    // Special case to just show estimated position if nothing else can be calculated
                    float relXEst = currentScanData.x - absolutePosition.X;
                    float relZEst = currentScanData.z - absolutePosition.Z;
                    thisArpaContact.estimate.bearing = std::atan2(relXEst,relZEst)/RAD_IN_DEG;
                    while (thisArpaContact.estimate.bearing < 0 ) {
                        thisArpaContact.estimate.bearing += 360;
                    }
                    thisArpaContact.estimate.range =  std::sqrt(pow(relXEst,2)+pow(relZEst,2))/M_IN_NM; //Nm
                } else {
                    float deltaX = currentScanData.x - referenceScanData.x;
                    float deltaZ = currentScanData.z - referenceScanData.z;

                    //Absolute vector
                    thisArpaContact.estimate.absVectorX = deltaX/deltaTime; //m/s
                    thisArpaContact.estimate.absVectorZ = deltaZ/deltaTime; //m/s
                    thisArpaContact.estimate.absHeading = std::atan2(deltaX,deltaZ)/RAD_IN_DEG;
                    while (thisArpaContact.estimate.absHeading < 0 ) {
                        thisArpaContact.estimate.absHeading += 360;
                    }
                    //Relative vector:
                    thisArpaContact.estimate.relVectorX = thisArpaContact.estimate.absVectorX - ownShip.getSOG() * sin((ownShip.getCOG())*irr::core::DEGTORAD);
                    thisArpaContact.estimate.relVectorZ = thisArpaContact.estimate.absVectorZ - ownShip.getSOG() * cos((ownShip.getCOG())*irr::core::DEGTORAD); //ownShipSpeed in m/s
                    thisArpaContact.estimate.relHeading = std::atan2(thisArpaContact.estimate.relVectorX,thisArpaContact.estimate.relVectorZ)/RAD_IN_DEG;
                    while (thisArpaContact.estimate.relHeading < 0 ) {
                        thisArpaContact.estimate.relHeading += 360;
                    }

                    //Estimated current position:
                    float relXEst = currentScanData.x - absolutePosition.X + thisArpaContact.estimate.absVectorX * (absoluteTime - currentScanData.timeStamp);
                    float relZEst = currentScanData.z - absolutePosition.Z + thisArpaContact.estimate.absVectorZ * (absoluteTime - currentScanData.timeStamp);
                    thisArpaContact.estimate.bearing = std::atan2(relXEst,relZEst)/RAD_IN_DEG;
                    while (thisArpaContact.estimate.bearing < 0 ) {
                        thisArpaContact.estimate.bearing += 360;
                    }
                    thisArpaContact.estimate.range =  std::sqrt(pow(relXEst,2)+pow(relZEst,2))/M_IN_NM; //Nm
                    thisArpaContact.estimate.speed = std::sqrt(pow(thisArpaContact.estimate.absVectorX,2) + pow(thisArpaContact.estimate.absVectorZ,2))*MPS_TO_KTS;

                    //TODO: CPA AND TCPA here: Need checking/testing
                    float contactRelAngle = thisArpaContact.estimate.relHeading - (180+thisArpaContact.estimate.bearing);
                    float contactRange = thisArpaContact.estimate.range; //(Nm)
                    float relDistanceToCPA = contactRange * cos(contactRelAngle*RAD_IN_DEG); //Distance along the other ship's relative motion line
                    float relativeSpeed = std::sqrt(pow(thisArpaContact.estimate.relVectorX,2) + pow(thisArpaContact.estimate.relVectorZ,2))*MPS_TO_KTS;
                    if (fabs(relativeSpeed) < 0.001) {relativeSpeed = 0.001;} //Avoid division by zero

                    thisArpaContact.estimate.cpa = contactRange * sin(contactRelAngle*RAD_IN_DEG);
                    thisArpaContact.estimate.tcpa = 60*relDistanceToCPA/relativeSpeed; // (nm / (nm/hr)), so time in hours, converted to minutes
                    //std::cout << "Contact " << thisArpaContact.estimate.displayID << " CPA: " <<  thisArpaContact.estimate.cpa << " nm in " << thisArpaContact.estimate.tcpa << " minutes" << std::endl;


                } //If time between scans > 0 
            } //Contact not lost
        } //If at least 2 scans
    } //If ARPA is on
}

void RadarCalculation::render(irr::video::IImage * radarImage, irr::video::IImage * radarImageOverlaid, float ownShipHeading, float ownShipSpeed)
{

    //IPROF_FUNC;
    //*************************
    //generate image from array
    //*************************

    //Render background radar picture into radarImage, then copy to radarImageOverlaid and do any 2d drawing on top (so we don't have to redraw all pixels each time
    uint32_t bitmapWidth = radarRadiusPx*2; //Set width to use - to map GUI radar display diameter in screen pixels

    //If the image is smaller than ideal, render anyway
    if (radarImage->getDimension().Width < bitmapWidth) {
        bitmapWidth = radarImage->getDimension().Width;
    }
    //if (radarImage->getDimension().Width < bitmapWidth) //Check the image we're rendering into is big enough
    //    {return;}

    //draw from array to image
    float centrePixel = (bitmapWidth-1.0)/2.0; //The centre of the bitmap. Normally this will be a fractional number (##.5)

    //precalculate cell max/min range for speed outside nested loop
	std::vector<float> cellMinRange;
	std::vector<float> cellMaxRange;
	//float cellMinRange [rangeResolution];
    //float cellMaxRange [rangeResolution];
	cellMinRange.push_back(0);
	cellMaxRange.push_back(0);
    for (uint32_t currentStep = 1; currentStep<rangeResolution; currentStep++) { //Note that we start with the element at 1, so we've already pushed in a dummy entry at 0
        cellMinRange.push_back((currentStep-0.5)*(bitmapWidth*0.5/(float)rangeResolution));//Range in pixels from centre
        cellMaxRange.push_back((currentStep+0.5)*(bitmapWidth*0.5/(float)rangeResolution));
    }

    float scanAngle;

    for (int scanLine = 0; scanLine < angularResolution; scanLine++) {

        scanAngle = ((float) scanLine / (float) angularResolution) * 360.0f;

        float cellMinAngle = scanAngle - scanAngleStep / 2.0;
        float cellMaxAngle = scanAngle + scanAngleStep / 2.0;

        for (uint32_t currentStep = 1; currentStep<rangeResolution; currentStep++) {

            //If the sector has changed, draw it. If we're stabilising the picture, need to re-draw all in case the ship's head has changed
            if(toReplot[scanLine] || stabilised)
            {

                if (headUp || scanArrayToPlotPrevious[scanLine][currentStep] != scanArrayToPlot[scanLine][currentStep]) { //If north up, we only need to replot if the previous plot to this sector was different
                    float pixelColour=scanArrayToPlot[scanLine][currentStep];

                    if (pixelColour>1.0) {pixelColour = 1.0;}
                    if (pixelColour<0)   {pixelColour =   0;}

                    //Interpolate colour between foreground and background
                    irr::video::SColor thisColour = getRadarForegroundColour().getInterpolated(getRadarBackgroundColour(), pixelColour);
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
    float radarOffsetAngle = 0;
    if (headUp) {
        radarOffsetAngle = -1*ownShipHeading;
    }

    //Draw parallel indexes on here
    if (piRanges.size() == piBearings.size()) {
        for(unsigned int i = 0; i< piRanges.size(); i++) {
            float thisPIrange = piRanges.at(i);
            float thisPIbrg = piBearings.at(i);
            if(fabs(thisPIrange) > 0.0001 && fabs(thisPIrange) < getRangeNm()) {
                //Not zero range or off screen

                float piRangePX = (float)bitmapWidth/2.0 * thisPIrange / getRangeNm(); //Find range in Px

                //find sin and cos of PI angle (so we only need once)
                float sinPIbrg=sin(-1*(thisPIbrg + radarOffsetAngle)*RAD_IN_DEG);
				float cosPIbrg=cos(-1*(thisPIbrg + radarOffsetAngle)*RAD_IN_DEG);

				//find central point on line
                float x_a = -1*piRangePX * sin( (90-(-1*(thisPIbrg + radarOffsetAngle)))*RAD_IN_DEG ) + centrePixel;
                float z_a =    piRangePX * cos( (90-(-1*(thisPIbrg + radarOffsetAngle)))*RAD_IN_DEG ) + centrePixel;

                //find half chord length (length of PI line)
                float halfChord = pow(pow((float)bitmapWidth/2.0,2) - pow(piRangePX,2),0.5); //already checked that PIRange is smaller, so should be valid

                //calculate end points of line
                float x_1=x_a - halfChord * sinPIbrg;
                float z_1=z_a - halfChord * cosPIbrg;
                float x_2=x_a + halfChord * sinPIbrg;
				float z_2=z_a + halfChord * cosPIbrg;

				drawLine(radarImageOverlaid,x_1,z_1,x_2,z_2,255,255,255,255);

				//Show line number
				//Find point towards centre from the line
				float xDirection = centrePixel-x_a;
				float yDirection = centrePixel-z_a;
				if (x_a!=0 && z_a !=0) {

                    float mag = pow(pow(xDirection,2)+pow(yDirection,2),0.5);
                    xDirection/=mag;
                    yDirection/=mag;

                    int32_t xTextPos = x_a + 15*xDirection;
                    int32_t yTextPos = z_a + 15*yDirection;

                    irr::video::IImage* idNumberImage = NumberToImage::getImage(i+1,device);
                    if (idNumberImage) {
                        irr::core::rect<int32_t> sourceRect = irr::core::rect<int32_t>(0,0,idNumberImage->getDimension().Width,idNumberImage->getDimension().Height);
                        idNumberImage->copyToWithAlpha(radarImageOverlaid,irr::core::position2d<int32_t>(xTextPos,yTextPos),sourceRect,irr::video::SColor(255,255,255,255));
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
            float contactRangePx = (float)bitmapWidth/2.0 * thisEstimate.range/getRangeNm();

            //Find estimated screen location of contact
            int32_t deltaX = centrePixel + contactRangePx * sin((thisEstimate.bearing+radarOffsetAngle)*RAD_IN_DEG);
            int32_t deltaY = centrePixel - contactRangePx * cos((thisEstimate.bearing+radarOffsetAngle)*RAD_IN_DEG);

            //Show contact on screen
            drawCircle(radarImageOverlaid,deltaX,deltaY,radarRadiusPx/40,255,255,255,255); //Draw circle around contact

            //Draw contact's display ID :
            irr::video::IImage* idNumberImage = NumberToImage::getImage(thisEstimate.displayID,device);

            if (idNumberImage) {
                irr::core::rect<int32_t> sourceRect = irr::core::rect<int32_t>(0,0,idNumberImage->getDimension().Width,idNumberImage->getDimension().Height);
                idNumberImage->copyToWithAlpha(radarImageOverlaid,irr::core::position2d<int32_t>(deltaX-10,deltaY-10),sourceRect,irr::video::SColor(255,255,255,255));
                idNumberImage->drop();
            }

            //draw a vector
            float adjustedVectorX;
            float adjustedVectorZ;
            if (trueVectors) {
                adjustedVectorX = thisEstimate.absVectorX;
                adjustedVectorZ = thisEstimate.absVectorZ;
            } else {
                adjustedVectorX = thisEstimate.relVectorX;
                adjustedVectorZ = thisEstimate.relVectorZ;
            }

            //Rotate if in head/course up mode
            if (headUp) {
                float cosOffsetAngle = cos(-1*radarOffsetAngle*irr::core::DEGTORAD);
                float sinOffsetAngle = sin(-1*radarOffsetAngle*irr::core::DEGTORAD);

                //Implement rotation here
                float newX = adjustedVectorX*cosOffsetAngle - adjustedVectorZ*sinOffsetAngle;
                float newZ = adjustedVectorX*sinOffsetAngle + adjustedVectorZ*cosOffsetAngle;

                adjustedVectorX = newX;
                adjustedVectorZ = newZ;
            }

            int32_t headingVectorX = Utilities::round(((float)bitmapWidth/2.0)   * adjustedVectorX * 60 * vectorLengthMinutes / (M_IN_NM * getRangeNm())); //Vector length in pixels
            int32_t headingVectorY = Utilities::round(((float)bitmapWidth/2.0)*-1* adjustedVectorZ * 60 * vectorLengthMinutes / (M_IN_NM * getRangeNm()));

            //std::cout << headingVectorX << " " << headingVectorY << std::endl;

            drawLine(radarImageOverlaid,deltaX,deltaY,deltaX + headingVectorX,deltaY+headingVectorY,255,255,255,255);

        }
    }


}

void RadarCalculation::drawSector(irr::video::IImage * radarImage,float centreX, float centreY, float innerRadius, float outerRadius, float startAngle, float endAngle, uint32_t alpha, uint32_t red, uint32_t green, uint32_t blue, float ownShipHeading)
//draw a bounded sector
{

    //IPROF_FUNC;

    if (headUp) {
        startAngle -= ownShipHeading;
        endAngle -= ownShipHeading;
    }

    //find the corner points (Fixme: Not quite right when the extreme point is on the outer curve)
    float sinStartAngle = std::sin(irr::core::DEGTORAD*startAngle);
    float cosStartAngle = std::cos(irr::core::DEGTORAD*startAngle);
    float sinEndAngle = std::sin(irr::core::DEGTORAD*endAngle);
    float cosEndAngle = std::cos(irr::core::DEGTORAD*endAngle);

    float point1X = centreX + sinStartAngle*innerRadius;
    float point1Y = centreY - cosStartAngle*innerRadius;
    float point2X = centreX + sinStartAngle*outerRadius;
    float point2Y = centreY - cosStartAngle*outerRadius;
    float point3X = centreX + sinEndAngle*outerRadius;
    float point3Y = centreY - cosEndAngle*outerRadius;
    float point4X = centreX + sinEndAngle*innerRadius;
    float point4Y = centreY - cosEndAngle*innerRadius;

    //find the 'bounding box'
    int32_t minX = std::min(std::min(point1X,point2X),std::min(point3X,point4X));
    int32_t maxX = std::max(std::max(point1X,point2X),std::max(point3X,point4X));
    int32_t minY = std::min(std::min(point1Y,point2Y),std::min(point3Y,point4Y));
    int32_t maxY = std::max(std::max(point1Y,point2Y),std::max(point3Y,point4Y));

    float innerRadiusSqr = innerRadius*innerRadius;
    float outerRadiusSqr = outerRadius*outerRadius;

    //draw the points
    for (int i = minX;i<=maxX;i++) {
        float localX = i - centreX; //position referred to centre
        float localXSq = localX*localX;

        for (int j = minY;j<=maxY;j++) {

            float localY = j - centreY; //position referred to centre

            float localRadiusSqr = localXSq + localY*localY; //check radius of points
            //float localAngle = irr::core::RADTODEG*std::atan2(localX,-1*localY); //check angle of point
            //float localAngle = irr::core::RADTODEG*fast_atan2f(localX,-1*localY);

            //if the point is within the limits, plot it
            if (localRadiusSqr >= innerRadiusSqr && localRadiusSqr <= outerRadiusSqr) {
                //if (Angles::isAngleBetween(localAngle,startAngle,endAngle)) {
                if (Angles::isAngleBetween(bc::graphics::Vec2(localX,-1*localY),bc::graphics::Vec2(sinStartAngle,cosStartAngle),bc::graphics::Vec2(sinEndAngle,cosEndAngle))) {
                    //Plot i,j
                    if (i >= 0 && j >= 0) {radarImage->setPixel(i,j,irr::video::SColor(alpha,red,green,blue));}
                }
            }
        }
    }

}

void RadarCalculation::drawLine(irr::video::IImage * radarImage, float startX, float startY, float endX, float endY, uint32_t alpha, uint32_t red, uint32_t green, uint32_t blue)//Try with float as inputs so we can do interpolation based on the theoretical start and end
{

    float deltaX = endX - startX;
    float deltaY = endY - startY;

    float lengthSum = std::abs(deltaX) + std::abs(deltaY);

    uint32_t radiusSquared = pow(radarRadiusPx,2);

    if (lengthSum > 0) {
        for (float i = 0; i<=1; i += 1/lengthSum) {
            int32_t thisX = Utilities::round(startX + deltaX * i);
            int32_t thisY = Utilities::round(startY + deltaY * i);
            //Find distance from centre
            int32_t centreToX = thisX - radarRadiusPx;
            int32_t centreToY = thisY - radarRadiusPx;
            if (pow(centreToX,2) + pow(centreToY,2) <= radiusSquared) {
                if (thisX >= 0 && thisY >= 0) {
                    radarImage->setPixel(thisX,thisY,irr::video::SColor(alpha,red,green,blue));
                }
            }
        }
    } else {
        int32_t thisX = Utilities::round(startX);
        int32_t thisY = Utilities::round(startY);
        //Find distance from centre
        int32_t centreToX = thisX - radarRadiusPx;
        int32_t centreToY = thisY - radarRadiusPx;
        if (pow(centreToX,2) + pow(centreToY,2) <= radiusSquared) {
            if (thisX >= 0 && thisY >= 0) {radarImage->setPixel(thisX,thisY,irr::video::SColor(alpha,red,green,blue));}
        }
    }
}

void RadarCalculation::drawCircle(irr::video::IImage * radarImage, float centreX, float centreY, float radius, uint32_t alpha, uint32_t red, uint32_t green, uint32_t blue)//Try with float as inputs so we can do interpolation based on the theoretical start and end
{
    float circumference = 2.0 * PI * radius;

    uint32_t radiusSquared = pow(radarRadiusPx,2);

    if (circumference > 0) {
        for (float i = 0; i<=1; i += 1/circumference) {
            int32_t thisX = Utilities::round(centreX + radius * sin(i*2*PI));
            int32_t thisY = Utilities::round(centreY + radius * cos(i*2*PI));
            //Find distance from centre
            int32_t centreToX = thisX - radarRadiusPx;
            int32_t centreToY = thisY - radarRadiusPx;
            if (pow(centreToX,2) + pow(centreToY,2) <= radiusSquared) {
                if (thisX >= 0 && thisY >= 0) {
                    radarImage->setPixel(thisX,thisY,irr::video::SColor(alpha,red,green,blue));
                }
            }
        }
    } else {
        int32_t thisX = Utilities::round(centreX);
        int32_t thisY = Utilities::round(centreY);
        //Find distance from centre
        int32_t centreToX = thisX - radarRadiusPx;
        int32_t centreToY = thisY - radarRadiusPx;
        if (pow(centreToX,2) + pow(centreToY,2) <= radiusSquared) {
            if (thisX >= 0 && thisY >= 0) {
                radarImage->setPixel(thisX,thisY,irr::video::SColor(alpha,red,green,blue));
            }
        }
    }
}

float RadarCalculation::rangeAtAngle(float checkAngle,float centreX, float centreZ, float heading)
{
	//Special case is if heading and checkAngle are identical. In this case, return the centre point if it lies on the angle, and 0 if not
	if (std::abs(Angles::normaliseAngle(checkAngle-heading)) < 0.001) {
		if (Angles::normaliseAngle(irr::core::RADTODEG*std::atan2(centreX,centreZ)-checkAngle) < 0.1) {
			return std::sqrt(std::pow(centreX,2) + std::pow(centreZ,2));
		} else {
			return 0;
		}
	}

	float lambda; //This is the distance from the centre of the contact

	lambda = (centreX - centreZ*tan(irr::core::DEGTORAD*checkAngle))/(cos(irr::core::DEGTORAD*heading)*tan(irr::core::DEGTORAD*checkAngle) - sin(irr::core::DEGTORAD*heading));

	float distanceSqr = std::pow(lambda,2) + lambda*(2*centreX*sin(irr::core::DEGTORAD*heading) + 2*centreZ*cos(irr::core::DEGTORAD*heading)) + (std::pow(centreX,2) + std::pow(centreZ,2));

	float distance = 0;

	if (distanceSqr > 0) {
		distance = std::sqrt(distanceSqr);
	}

	return distance;

}

float RadarCalculation::radarNoise(float radarNoiseLevel, float radarSeaClutter, float radarRainClutter, float weather, float radarRange,float radarBrgDeg, float windDirectionDeg, float radarInclinationAngle, float rainIntensity)
//radarRange in metres
{
	float radarNoiseVal = 0;

	if (radarRange != 0) {

		float randomValue = (float)rand()/RAND_MAX; //store this so we can manipulate the random distribution;
		float randomValueSea = (float)rand()/RAND_MAX; //different value for sea clutter;

		//reshape the uniform random distribution into one with an infinite tail up to high values
		float randomValueWithTail=0;
		if (randomValue > 0) {
            //3rd power is to shape distribution so sufficient high energy returns are generated
			randomValueWithTail = randomValue * pow( (1/randomValue) - 1, 3);
		}

		//same for sea clutter noise
		float randomValueWithTailSea=0;
		if (randomValueSea > 0) {
            if (radarInclinationAngle > 0) {
                randomValueWithTailSea = 0; //if radar is scanning upwards, must be above sea surface, so don't add clutter
            } else {
                //3rd power is to shape distribution so sufficient high energy returns are generated
                randomValueWithTailSea = randomValueSea * pow((1/randomValueSea) - 1, 3);
            }
		}

		//less high power returns for rain clutter - roughly gaussian, so get an average of independent random numbers
		float randomValueWithTailRain = ((float)rand()/RAND_MAX + (float)rand()/RAND_MAX + (float)rand()/RAND_MAX + (float)rand()/RAND_MAX)/4.0;

		//Apply directional correction to the clutter, so most is upwind, some is downwind. Mean value = 1
		float relativeWindAngle = (windDirectionDeg - radarBrgDeg)*RAD_IN_DEG;
		float windCorrectionFactor = 2.5*(0.5*(cos(2*relativeWindAngle)+1))*(0.5+sin(relativeWindAngle/2.0)*0.5);
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

bool RadarCalculation::isPointInEllipse(float pointX, float pointZ, float centreX, float centreZ, float width, float length, float angle)
{
    
    // Quick first check
    if ( fmax(abs(pointX - centreX), abs(pointZ - centreZ)) > fmax(width, length)) {
        return false;
    }
    
    // Detailed check

    // See https://stackoverflow.com/a/16824748/12829372
    float cosAngle = cos(-1.0 * angle * irr::core::DEGTORAD);
    float sinAngle = sin(-1.0 * angle * irr::core::DEGTORAD);

    float halfWidth2 = width/2 * width/2;
    float halfLength2 = length/2 * length/2;
    
    if (halfLength2 == 0 || halfWidth2 == 0) {
        return false;
    }

    float paramA = pow(cosAngle*(pointX-centreX)+sinAngle*(pointZ-centreZ),2);
    float paramB = pow(sinAngle*(pointX-centreX)-cosAngle*(pointZ-centreZ),2);

    float ellipse=(paramA/halfWidth2)+(paramB/halfLength2);

    if (ellipse <= 1) {
        return true;
    } else {
        return false;
    }

}

irr::video::SColor RadarCalculation::getRadarForegroundColour() const
{
    if (currentRadarColourChoice < radarForegroundColours.size()) {
        return radarForegroundColours.at(currentRadarColourChoice);
    }
    else {
        return irr::video::SColor(255, 255, 220, 0);
    }
}

irr::video::SColor RadarCalculation::getRadarBackgroundColour() const
{
    if (currentRadarColourChoice < radarBackgroundColours.size()) {
        return radarBackgroundColours.at(currentRadarColourChoice);
    }
    else {
        return irr::video::SColor(255, 0, 0, 200);
    }
}

irr::video::SColor RadarCalculation::getRadarSurroundColour() const
{
    if (currentRadarColourChoice < radarSurroundColours.size()) {
        return radarSurroundColours.at(currentRadarColourChoice);
    }
    else {
        return irr::video::SColor(255, 128, 128, 128);
    }
}