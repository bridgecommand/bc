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

#include <iostream>
#include <cmath>
#include <cstdlib> //For rand()
#include <algorithm> //For sort()

using namespace irr;

RadarCalculation::RadarCalculation()
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

    //initialise scanArray size (360 x rangeResolution points per scan)
    rangeResolution = 64;
    scanArray.resize(360,std::vector<f32>(rangeResolution,0.0));
    scanArrayAmplified.resize(360,std::vector<f32>(rangeResolution,0.0));
    scanArrayAmplifiedPrevious.resize(360,std::vector<f32>(rangeResolution,0.0));

    //initialise arrays
    for(u32 i = 0; i<360; i++) {
        for(u32 j = 0; j<rangeResolution; j++) {
            scanArray[i][j] = 0.0;
            scanArrayAmplified[i][j] = 0.0;
            scanArrayAmplifiedPrevious[i][j] = -1.0;
        }
    }

    currentScanAngle=0;
}

RadarCalculation::~RadarCalculation()
{
    //dtor
}

void RadarCalculation::load(std::string radarConfigFile)
{
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
        for (int i = 1; i <= numberOfRadarRanges; i++) {
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
        radarRainClutter =IniFile::iniFileTof32(radarConfigFile,"radar_rain_clutter");
        if (radarNoiseLevel < 0) {radarNoiseLevel = 0.000000000005;}
        if (radarSeaClutter < 0) {radarSeaClutter = 0.000000001;}
        if (radarRainClutter< 0) {radarRainClutter= 0.00001;}

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

void RadarCalculation::update(irr::video::IImage * radarImage, irr::core::vector3d<irr::s64> offsetPosition, const Terrain& terrain, const OwnShip& ownShip, const Buoys& buoys, const OtherShips& otherShips, irr::f32 weather, irr::f32 rain, irr::f32 tideHeight, irr::f32 deltaTime, uint64_t absoluteTime)
{
    scan(offsetPosition, terrain, ownShip, buoys, otherShips, weather, rain, tideHeight, deltaTime, absoluteTime); // scan into scanArray[row (angle)][column (step)], and with filtering and amplification into scanArrayAmplified[][]
    updateARPA(offsetPosition, ownShip, absoluteTime); //From data in arpaContacts, updated in scan()
    render(radarImage, ownShip.getHeading()); //From scanArrayAmplified[row (angle)][column (step)], render to radarImage
}

void RadarCalculation::scan(irr::core::vector3d<irr::s64> offsetPosition, const Terrain& terrain, const OwnShip& ownShip, const Buoys& buoys, const OtherShips& otherShips, irr::f32 weather, irr::f32 rain, irr::f32 tideHeight, irr::f32 deltaTime, uint64_t absoluteTime)
{

    const irr::u32 SECONDS_BETWEEN_SCANS = 20;

    core::vector3df position = ownShip.getPosition();
    //Get absolute position relative to SW corner of world model
    core::vector3d<irr::s64> absolutePosition = offsetPosition;
    offsetPosition.X += position.X;
    offsetPosition.Y += position.Y;
    offsetPosition.Z += position.Z;

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
            f32 localNoise = radarNoise(radarNoiseLevel,radarSeaClutter,radarRainClutter,weather,localRange,currentScanAngle,0,scanSlope,rain); //FIXME: Needs rain intensity and wind direction

            //Scan other contacts here
            //Fixme: Implementation needs completing later for ARPA to check if contact is detectable against clutter
            //for(std::vector<RadarData>::iterator it = radarData.begin(); it != radarData.end(); ++it) {
            for(int thisContact = 0; thisContact<radarData.size(); thisContact++) {
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
                                if (radarEchoStrength*2 > localNoise) {
                                    //Contact is detectable in noise

                                    //Iterate through arpaContacts array, checking if this contact is in the list (by checking the if the 'contact' pointer is to the same underlying ship/buoy)
                                    int existingArpaContact=-1;
                                    for (int j = 0; j<arpaContacts.size(); j++) {
                                        if (arpaContacts.at(j).contact == radarData.at(thisContact).contact) {
                                            existingArpaContact = j;
                                        }
                                    }
                                    //If it doesn't exist, add it, and make existingArpaContact point to it
                                    if (existingArpaContact<0) {
                                        ARPAContact newContact;
                                        newContact.contact = radarData.at(thisContact).contact;
                                        newContact.contactType=CONTACT_NORMAL;
                                        newContact.displayID = 0; //Initially not displayed

                                        //Zeros for estimated state
                                        newContact.estimate.ignored = false;
                                        newContact.estimate.absVectorX = 0;
                                        newContact.estimate.absVectorZ = 0;
                                        newContact.estimate.absHeading = 0;

                                        arpaContacts.push_back(newContact);
                                        existingArpaContact = arpaContacts.size()-1;
                                        std::cout << "Adding contact " << existingArpaContact << std::endl;
                                    }
                                    //Add this scan (if not already scanned in the last X seconds
                                    size_t scansSize = arpaContacts.at(existingArpaContact).scans.size();
                                    if (scansSize==0 || absoluteTime > SECONDS_BETWEEN_SCANS + arpaContacts.at(existingArpaContact).scans.at(scansSize-1).timeStamp) {
                                        ARPAScan newScan;
                                        newScan.timeStamp = absoluteTime;
                                        //TODO: Add noise/uncertainty
                                        newScan.bearingDeg = radarData.at(thisContact).angle;
                                        newScan.rangeNm = radarData.at(thisContact).range / M_IN_NM;
                                        newScan.x = absolutePosition.X + radarData.at(thisContact).relX;
                                        newScan.z = absolutePosition.Z + radarData.at(thisContact).relZ;
                                        newScan.estimatedRCS = 100;//Todo: Implement

                                        arpaContacts.at(existingArpaContact).scans.push_back(newScan);
                                        std::cout << "ARPA update on " << existingArpaContact << std::endl;

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

void RadarCalculation::updateARPA(irr::core::vector3d<irr::s64> offsetPosition, const OwnShip& ownShip, uint64_t absoluteTime)
{
    //Based on scans data in arpaContacts, estimate current speed, heading and position



}

void RadarCalculation::render(irr::video::IImage * radarImage, irr::f32 ownShipHeading)
{
    //*************************
    //generate image from array
    //*************************

    //Get image size
    u32 bitmapWidth = radarImage->getDimension().Width;
    if (radarImage->getDimension().Height != bitmapWidth)
        {return;} //Check image is square, and return without action if not

    //draw from array to image
    f32 centrePixel = (bitmapWidth-1.0)/2.0; //The centre of the bitmap. Normally this will be a fractional number (##.5)

    //precalculate cell max/min range for speed outside nested loop
    irr::f32 cellMinRange [rangeResolution];
    irr::f32 cellMaxRange [rangeResolution];
    for (u32 currentStep = 1; currentStep<rangeResolution; currentStep++) {
        cellMinRange[currentStep] = ((currentStep-0.5)*(bitmapWidth*0.5/(float)rangeResolution));//Range in pixels from centre
        cellMaxRange[currentStep] = ((currentStep+0.5)*(bitmapWidth*0.5/(float)rangeResolution));//Fixme: Check rounding etc
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
                    radarImage->setPixel(i,j,video::SColor(alpha,red,green,blue));
                }
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
