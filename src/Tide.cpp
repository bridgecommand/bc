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
#include "irrlicht.h"
#include "IniFile.hpp"
#include "Utilities.hpp"
#include "Constants.hpp"
#include "ScenarioDataStructure.hpp"

#include <iostream>
#include <cmath>

Tide::Tide()
{

}

Tide::~Tide()
{
    //dtor
}

void Tide::load(const std::string& worldName, const ScenarioData& scenarioData) {

    //Initialise
    tideHeight = 0;

    //load tide.ini information
    std::string tideFilename = worldName;
    tideFilename.append("/tide.ini");

    uint32_t numberOfHarmonics = IniFile::iniFileTou32(tideFilename,"Harmonics");

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
    for(uint32_t i=1;i<=numberOfHarmonics; i++) {
        loadingHarmonic.amplitude = IniFile::iniFileTof32(tideFilename,IniFile::enumerate1("Amplitude",i));
        loadingHarmonic.offset = IniFile::iniFileTof32(tideFilename,IniFile::enumerate1("Offset",i));
        loadingHarmonic.speed = IniFile::iniFileTof32(tideFilename,IniFile::enumerate1("Speed",i));
        tidalHarmonics.push_back(loadingHarmonic);
    }

    std::string tidalStreamFilename = worldName;
    tidalStreamFilename.append("/tidalstream.ini");
    meanRangeSprings = IniFile::iniFileTof32(tidalStreamFilename,"MeanRangeSprings");
    meanRangeNeaps = IniFile::iniFileTof32(tidalStreamFilename,"MeanRangeNeaps");
    uint32_t numberOfDiamonds = IniFile::iniFileTou32(tidalStreamFilename,"Number");

    //Load other components
    tidalDiamond loadingDiamond;
    for(uint32_t i=1;i<=numberOfDiamonds; i++) {
        loadingDiamond.longitude =  IniFile::iniFileTof32(tidalStreamFilename,IniFile::enumerate1("Long",i));
        loadingDiamond.latitude =  IniFile::iniFileTof32(tidalStreamFilename,IniFile::enumerate1("Lat",i));

        //Load and convert SpeedN, SpeedS and Direction into speedXNeaps, speedXSprings etc in m/s for each hour between 6 before to 6 after high tide
        for(int j = 0; j<13; j++) {
            int hour = j-6;
            float speedNeaps = IniFile::iniFileTof32(tidalStreamFilename,IniFile::enumerate2("SpeedN",i,hour));
            float speedSprings = IniFile::iniFileTof32(tidalStreamFilename,IniFile::enumerate2("SpeedS",i,hour));
            float streamDirection = IniFile::iniFileTof32(tidalStreamFilename,IniFile::enumerate2("Direction",i,hour));
            loadingDiamond.speedXSprings[j] = sin(streamDirection*irr::core::DEGTORAD)*speedSprings*KTS_TO_MPS;
            loadingDiamond.speedZSprings[j] = cos(streamDirection*irr::core::DEGTORAD)*speedSprings*KTS_TO_MPS;
            loadingDiamond.speedXNeaps[j] = sin(streamDirection*irr::core::DEGTORAD)*speedNeaps*KTS_TO_MPS;
            loadingDiamond.speedZNeaps[j] = cos(streamDirection*irr::core::DEGTORAD)*speedNeaps*KTS_TO_MPS;
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

float Tide::getTideHeight() const {
    return tideHeight;
}

float Tide::calcTideHeight(uint64_t absoluteTime) const {
    float calculatedHeight = 0;

    calculatedHeight = 0;
    double timeHours = (double)absoluteTime/3600.0;

    int numberOfHarmonicElements = tidalHarmonics.size(); //including the 0th (constant) component
    if (numberOfHarmonicElements > 0)
    {
        calculatedHeight += tidalHarmonics[0].amplitude;

        //Add up harmonics
        for (int i=1;i<numberOfHarmonicElements; i++) {
            double harmonicAngleDeg = tidalHarmonics[i].offset + timeHours * tidalHarmonics[i].speed;
            harmonicAngleDeg=harmonicAngleDeg-Utilities::round(harmonicAngleDeg/360)*360; //Normalise (DEGREES)
            calculatedHeight+= tidalHarmonics[i].amplitude*cos(harmonicAngleDeg*irr::core::DEGTORAD64);
        }

    }

    return calculatedHeight;
}

bc::graphics::Vec2 Tide::getTidalStream(float longitude, float latitude, uint64_t requestTime) const {

	//Default return value
    bc::graphics::Vec2 tidalStream;
    tidalStream.x = 0;
    tidalStream.y = 0;

    //Find time to nearest high tide. TideHour is time since high water, -ve if before high water, +ve if after
    float tideHour = ((float)requestTime - (float)highTideTime(requestTime)) / SECONDS_IN_HOUR; //TODO: Check precision on this. Note we need to convert to signed number before subtraction!

    float totalWeight = 0;
    float weightedSumXSprings = 0;
    float weightedSumZSprings = 0;
    float weightedSumXNeaps = 0;
    float weightedSumZNeaps = 0;

    //Interpolate to find tidal stream information at each tidal diamond for now, then use weighted average to get local tidal stream
    //Set tidalStream.X and tidalStream.Y (in m/s)

    for (unsigned int i = 0; i<tidalDiamonds.size(); i++) {
        float distanceToDiamondLat = tidalDiamonds.at(i).latitude - latitude;
        float distanceToDiamondLong = tidalDiamonds.at(i).longitude - longitude;
        //Convert from lat/long distance into rough distance in nm
        //1 minute of latitude is 1nm, and longitude needs to be scaled down by cos(lat)

        float distanceToDiamond = std::pow(std::pow(distanceToDiamondLat,2) + std::pow(distanceToDiamondLong*cos(latitude*irr::core::DEGTORAD),2),0.5)/60;
        float thisWeight;
        if (fabs(distanceToDiamond) > 0.001) {
            thisWeight = 1/distanceToDiamond;
        } else {
            thisWeight = 1000;
        }
        totalWeight += thisWeight;

        //Interploate to get velocity component for current tide hour
        float tideHourOffset = tideHour + 6; //Scale to 0->12 range to align with arrays for

        float speedXNeaps;
        float speedZNeaps;
        float speedXSprings;
        float speedZSprings;

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
            float interpCoeff = (tideHourOffset-prevIndex);
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

		float localXNeaps = weightedSumXNeaps / totalWeight;
		float localZNeaps = weightedSumZNeaps / totalWeight;
		float localXSprings = weightedSumXSprings / totalWeight;
		float localZSprings = weightedSumZSprings / totalWeight;

		//Find how far we are between springs and neaps, based on meanRangeSprings, meanRangeNeaps, and calculated range
		float rangeOfDay = calcTideHeight(highTideTime(requestTime)) - calcTideHeight(lowTideTime(requestTime));
		if (rangeOfDay <= meanRangeNeaps) {
			tidalStream.x = localXNeaps;
			tidalStream.y = localZNeaps;
		}
		else if (rangeOfDay >= meanRangeSprings) {
			tidalStream.x = localXSprings;
			tidalStream.y = localZSprings;
		}
		else if ((meanRangeSprings - meanRangeNeaps) > 0) {
			float springsInterp = (rangeOfDay - meanRangeNeaps) / (meanRangeSprings - meanRangeNeaps);
			tidalStream.x = localXNeaps*(1 - springsInterp) + localXSprings*springsInterp;
			tidalStream.y = localZNeaps*(1 - springsInterp) + localZSprings*springsInterp;
		}
	} 
    return tidalStream;
}

float Tide::getTideGradient(uint64_t absoluteTime) const {
    //return der(TideHeight) (in ?? units)
    //tim is absolute time tide is required for

    float timeHours = float(absoluteTime)/SECONDS_IN_HOUR;
	float der=0;

	for (unsigned int i=1; i<tidalHarmonics.size(); i++) { //0th component has no gradient
        //;HarmonicHeight#=Tide(i,0)*Cos( Tide(i,1) + tim_hours*Tide(i,2)) ;tide(n,0) is amplitude, 1 is offset (rad), 2 is speed (rad/s)
		float harmonicAngle=tidalHarmonics.at(i).offset+timeHours*tidalHarmonics.at(i).speed;
		//reduce to range 0 to 360
		harmonicAngle=harmonicAngle-floor(harmonicAngle/360)*360;

		float harmonicDer=-1*tidalHarmonics.at(i).speed*tidalHarmonics.at(i).amplitude*sin(harmonicAngle*irr::core::DEGTORAD);
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
