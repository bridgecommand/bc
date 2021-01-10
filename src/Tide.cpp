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

#include "Tide.hpp"
#include "IniFile.hpp"
#include "Utilities.hpp"
#include "Constants.hpp"

#include <iostream>
#include <cmath>

//using namespace irr;

Tide::Tide()
{

}

Tide::~Tide()
{
    //dtor
}

void Tide::load(const std::string& worldName) {

    //Initialise
    tideHeight = 0;

    //load tide.ini information
    std::string tideFilename = worldName;
    tideFilename.append("/tide.ini");

    irr::u32 numberOfHarmonics = IniFile::iniFileTou32(tideFilename,"Harmonics");

    //If numberOfHarmonics = 0, we either have no tide file, or just a constant component. In either case, load
    //the constant component. Amplitude will be zero if no tide file
    //No tidal information, so load a dummy record
    tidalHarmonic constantHarmonic;
    constantHarmonic.amplitude = IniFile::iniFileTof32(tideFilename,"Amplitude(0)");
    constantHarmonic.offset = 0;
    constantHarmonic.speed = 0;
    tidalHarmonics.push_back(constantHarmonic);

    //Load other components
    tidalHarmonic loadingHarmonic;
    for(irr::u32 i=1;i<=numberOfHarmonics; i++) {
        loadingHarmonic.amplitude = IniFile::iniFileTof32(tideFilename,IniFile::enumerate1("Amplitude",i));
        loadingHarmonic.offset = IniFile::iniFileTof32(tideFilename,IniFile::enumerate1("Offset",i));
        loadingHarmonic.speed = IniFile::iniFileTof32(tideFilename,IniFile::enumerate1("Speed",i));
        tidalHarmonics.push_back(loadingHarmonic);
    }

    std::string tidalStreamFilename = worldName;
    tidalStreamFilename.append("/tidalstream.ini");
    meanRangeSprings = IniFile::iniFileTof32(tidalStreamFilename,"MeanRangeSprings");
    meanRangeNeaps = IniFile::iniFileTof32(tidalStreamFilename,"MeanRangeNeaps");
    irr::u32 numberOfDiamonds = IniFile::iniFileTou32(tidalStreamFilename,"Number");

    //Load other components
    tidalDiamond loadingDiamond;
    for(irr::u32 i=1;i<=numberOfDiamonds; i++) {
        loadingDiamond.longitude =  IniFile::iniFileTof32(tidalStreamFilename,IniFile::enumerate1("Long",i));
        loadingDiamond.latitude =  IniFile::iniFileTof32(tidalStreamFilename,IniFile::enumerate1("Lat",i));

        //Load and convert SpeedN, SpeedS and Direction into speedXNeaps, speedXSprings etc in m/s for each hour between 6 before to 6 after high tide
        for(int j = 0; j<13; j++) {
            int hour = j-6;
            irr::f32 speedNeaps = IniFile::iniFileTof32(tidalStreamFilename,IniFile::enumerate2("SpeedN",i,hour));
            irr::f32 speedSprings = IniFile::iniFileTof32(tidalStreamFilename,IniFile::enumerate2("SpeedS",i,hour));
            irr::f32 streamDirection = IniFile::iniFileTof32(tidalStreamFilename,IniFile::enumerate2("Direction",i,hour));
            loadingDiamond.speedXSprings[i] = sin(streamDirection*irr::core::DEGTORAD)*speedSprings*KTS_TO_MPS;
            loadingDiamond.speedZSprings[i] = cos(streamDirection*irr::core::DEGTORAD)*speedSprings*KTS_TO_MPS;
            loadingDiamond.speedXNeaps[i] = sin(streamDirection*irr::core::DEGTORAD)*speedNeaps*KTS_TO_MPS;
            loadingDiamond.speedZNeaps[i] = cos(streamDirection*irr::core::DEGTORAD)*speedNeaps*KTS_TO_MPS;
            //std::cout << "Loading hour " << hour << " where direction is " << streamDirection << std::endl;
            //std::cout << "Loading from: " << IniFile::enumerate2("Direction",i,hour) << std::endl;
        }

        tidalDiamonds.push_back(loadingDiamond);

    }

}

void Tide::update(uint64_t absoluteTime) {
    //update tideHeight for current time (unix epoch time in s)
    tideHeight=calcTideHeight(absoluteTime);

}

irr::f32 Tide::getTideHeight() const {
    return tideHeight;
}

irr::f32 Tide::calcTideHeight(uint64_t absoluteTime) const {
    irr::f32 calculatedHeight = 0;

    calculatedHeight = 0;
    irr::f64 timeHours = (irr::f64)absoluteTime/3600.0;

    int numberOfHarmonicElements = tidalHarmonics.size(); //including the 0th (constant) component
    if (numberOfHarmonicElements > 0)
    {
        calculatedHeight += tidalHarmonics[0].amplitude;

        //Add up harmonics
        for (int i=1;i<numberOfHarmonicElements; i++) {
            irr::f64 harmonicAngleDeg = tidalHarmonics[i].offset + timeHours * tidalHarmonics[i].speed;
            harmonicAngleDeg=harmonicAngleDeg-Utilities::round(harmonicAngleDeg/360)*360; //Normalise (DEGREES)
            calculatedHeight+= tidalHarmonics[i].amplitude*cos(harmonicAngleDeg*irr::core::DEGTORAD64);
        }

    }

    return calculatedHeight;
}

irr::core::vector2df Tide::getTidalStream(irr::f32 longitude, irr::f32 latitude, uint64_t absoluteTime) const {

	//Default return value
    irr::core::vector2df tidalStream;
    tidalStream.X = 0;
    tidalStream.Y = 0;

    //Find time to nearest high tide. TideHour is time since high water, -ve if before high water, +ve if after
    irr::f32 tideHour = ((irr::f32)absoluteTime - (irr::f32)highTideTime(absoluteTime)) / SECONDS_IN_HOUR; //TODO: Check precision on this. Note we need to convert to signed number before subtraction!

    irr::f32 totalWeight = 0;
    irr::f32 weightedSumXSprings = 0;
    irr::f32 weightedSumZSprings = 0;
    irr::f32 weightedSumXNeaps = 0;
    irr::f32 weightedSumZNeaps = 0;

    //Interpolate to find tidal stream information at each tidal diamond for now, then use weighted average to get local tidal stream
    //Set tidalStream.X and tidalStream.Y (in m/s)

    for (unsigned int i = 0; i<tidalDiamonds.size(); i++) {
        irr::f32 distanceToDiamondLat = tidalDiamonds.at(i).latitude - latitude;
        irr::f32 distanceToDiamondLong = tidalDiamonds.at(i).longitude - longitude;
        //Convert from lat/long distance into rough distance in nm
        //1 minute of latitude is 1nm, and longitude needs to be scaled down by cos(lat)

        irr::f32 distanceToDiamond = std::pow(std::pow(distanceToDiamondLat,2) + std::pow(distanceToDiamondLong*cos(latitude*irr::core::DEGTORAD),2),0.5)/60;
        irr::f32 thisWeight;
        if (fabs(distanceToDiamond) > 0.001) {
            thisWeight = 1/distanceToDiamond;
        } else {
            thisWeight = 1000;
        }
        totalWeight += thisWeight;

        //Interploate to get velocity component for current tide hour
        irr::f32 tideHourOffset = tideHour + 6; //Scale to 0->12 range to align with arrays for

        irr::f32 speedXNeaps;
        irr::f32 speedZNeaps;
        irr::f32 speedXSprings;
        irr::f32 speedZSprings;

        if (floor(tideHourOffset) < 0) {
            //Below lower limit
            speedXNeaps = tidalDiamonds.at(i).speedXNeaps[0];
            speedZNeaps = tidalDiamonds.at(i).speedZNeaps[0];
            speedXSprings = tidalDiamonds.at(i).speedXSprings[0];
            speedZSprings = tidalDiamonds.at(i).speedZSprings[0];
        } else if (ceil(tideHourOffset) > 12) {
            //Above upper limit
            speedXNeaps = tidalDiamonds.at(i).speedXNeaps[12];
            speedZNeaps = tidalDiamonds.at(i).speedZNeaps[12];
            speedXSprings = tidalDiamonds.at(i).speedXSprings[12];
            speedZSprings = tidalDiamonds.at(i).speedZSprings[12];
        } else {
            //Normal range
            unsigned int prevIndex = floor(tideHourOffset);
            unsigned int nextIndex = ceil(tideHourOffset);
            irr::f32 interpCoeff = (tideHourOffset-prevIndex);
            speedXNeaps = tidalDiamonds.at(i).speedXNeaps[prevIndex]*(1-interpCoeff) + tidalDiamonds.at(i).speedXNeaps[nextIndex]*interpCoeff;
            speedZNeaps = tidalDiamonds.at(i).speedZNeaps[prevIndex]*(1-interpCoeff) + tidalDiamonds.at(i).speedZNeaps[nextIndex]*interpCoeff;
            speedXSprings = tidalDiamonds.at(i).speedXSprings[prevIndex]*(1-interpCoeff) + tidalDiamonds.at(i).speedXSprings[nextIndex]*interpCoeff;
            speedZSprings = tidalDiamonds.at(i).speedZSprings[prevIndex]*(1-interpCoeff) + tidalDiamonds.at(i).speedZSprings[nextIndex]*interpCoeff;
        }

        weightedSumXNeaps += speedXNeaps*thisWeight;
        weightedSumZNeaps += speedZNeaps*thisWeight;
        weightedSumXSprings += speedXSprings*thisWeight;
        weightedSumZSprings += speedZSprings*thisWeight;

    }

	if (totalWeight > 0) {

		irr::f32 localXNeaps = weightedSumXNeaps / totalWeight;
		irr::f32 localZNeaps = weightedSumZNeaps / totalWeight;
		irr::f32 localXSprings = weightedSumXSprings / totalWeight;
		irr::f32 localZSprings = weightedSumZSprings / totalWeight;

		//Find how far we are between springs and neaps, based on meanRangeSprings, meanRangeNeaps, and calculated range
		irr::f32 rangeOfDay = calcTideHeight(highTideTime(absoluteTime)) - calcTideHeight(lowTideTime(absoluteTime));
		if (rangeOfDay <= meanRangeNeaps) {
			tidalStream.X = localXNeaps;
			tidalStream.Y = localZNeaps;
		}
		else if (rangeOfDay >= meanRangeSprings) {
			tidalStream.X = localXSprings;
			tidalStream.Y = localZSprings;
		}
		else if ((meanRangeSprings - meanRangeNeaps) > 0) {
			irr::f32 springsInterp = (rangeOfDay - meanRangeNeaps) / (meanRangeSprings - meanRangeNeaps);
			tidalStream.X = localXNeaps*(1 - springsInterp) + localXSprings*springsInterp;
			tidalStream.Y = localZNeaps*(1 - springsInterp) + localZSprings*springsInterp;
		}
	} 
    return tidalStream;
}

irr::f32 Tide::getTideGradient(uint64_t absoluteTime) const {
    //return der(TideHeight) (in ?? units)
    //tim is absolute time tide is required for

    irr::f32 timeHours = irr::f32(absoluteTime)/SECONDS_IN_HOUR;
	irr::f32 der=0;

	for (unsigned int i=1; i<tidalHarmonics.size(); i++) { //0th component has no gradient
        //;HarmonicHeight#=Tide(i,0)*Cos( Tide(i,1) + tim_hours*Tide(i,2)) ;tide(n,0) is amplitude, 1 is offset (rad), 2 is speed (rad/s)
		irr::f32 harmonicAngle=tidalHarmonics.at(i).offset+timeHours*tidalHarmonics.at(i).speed;
		//reduce to range 0 to 360
		harmonicAngle=harmonicAngle-floor(harmonicAngle/360)*360;

		irr::f32 harmonicDer=-1*tidalHarmonics.at(i).speed*tidalHarmonics.at(i).amplitude*sin(harmonicAngle*irr::core::DEGTORAD);
		//
		//	;a cos (w t + c)
		//	;->
		//	;-wa sin(w t + c)

		der = der + harmonicDer;
	}
	return der;
}

uint64_t Tide::highTideTime(uint64_t startSearchTime, int searchDirection) const {
//find the next high tide time in s before or after start_search_time. if search_direction is positive, search forward in time
//do this by finding when sum of derivatives of harmonics goes from +ve to -ve

//If searchDirection is 0, find the nearest high tide (by gradient climb)


    if (searchDirection==0) {
        if (getTideGradient(startSearchTime) > 0) { //Tide is rising
            searchDirection = 1;
        } else {
            searchDirection = -1;
        }
    }

    int timestep = 10*60; //Find to nearest 10 minutes

	if (searchDirection > 0) {
        //look forward in time
		for (uint64_t t = startSearchTime; t<=startSearchTime+SECONDS_IN_DAY; t+=timestep) {
			if (getTideGradient(t - timestep) > 0 && getTideGradient(t + timestep) < 0) {
				return t;
			}
		}
	} else {
        //look back in time
		for (uint64_t t = startSearchTime-timestep; t>=startSearchTime-SECONDS_IN_DAY; t-=timestep) {
			if (getTideGradient(t - timestep) > 0 && getTideGradient(t + timestep) < 0) {
				return t;
			}
		}
	}
	//no time found, return 0
	return 0;
}

uint64_t Tide::lowTideTime(uint64_t startSearchTime, int searchDirection) const {
//find the next low tide time in s before or after start_search_time. if search_direction is positive, search forward in time
//do this by finding when sum of derivatives of harmonics goes from -ve to +ve

//If searchDirection is 0, find the nearest high tide (by gradient climb)

    if (searchDirection==0) {
        if (getTideGradient(startSearchTime) < 0) { //Tide is falling
            searchDirection = 1;
        } else {
            searchDirection = -1;
        }
    }

    int timestep = 10*60; //Find to nearest 10 minutes

	if (searchDirection > 0) {
        //look forward in time
		for (uint64_t t = startSearchTime; t<=startSearchTime+SECONDS_IN_DAY; t+=timestep) {
			if (getTideGradient(t - timestep) < 0 && getTideGradient(t + timestep) > 0) {
				return t;
			}
		}
	} else {
        //look back in time
		for (uint64_t t = startSearchTime-timestep; t>=startSearchTime-SECONDS_IN_DAY; t-=timestep) {
			if (getTideGradient(t - timestep) < 0 && getTideGradient(t + timestep) > 0) {
				return t;
			}
		}
	}
	//no time found, return 0
	return 0;
}
