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

using namespace irr;

Tide::Tide()
{

}

Tide::~Tide()
{
    //dtor
}

void Tide::load(const std::string& worldName) {
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
    for(u32 i=1;i<=numberOfHarmonics; i++) {
        loadingHarmonic.amplitude = IniFile::iniFileTof32(tideFilename,IniFile::enumerate1("Amplitude",i));
        loadingHarmonic.offset = IniFile::iniFileTof32(tideFilename,IniFile::enumerate1("Offset",i));
        loadingHarmonic.speed = IniFile::iniFileTof32(tideFilename,IniFile::enumerate1("Speed",i));
        tidalHarmonics.push_back(loadingHarmonic);
    }

}

void Tide::update(uint64_t absoluteTime) {
    //update tideHeight for current time (unix epoch time in s)

    tideHeight = 0;
    irr::f64 timeHours = (irr::f64)absoluteTime/3600.0;

    int numberOfHarmonicElements = tidalHarmonics.size(); //including the 0th (constant) component
    if (numberOfHarmonicElements > 0)
    {
        tideHeight += tidalHarmonics[0].amplitude;

        //Add up harmonics
        for (int i=1;i<numberOfHarmonicElements; i++) {
            irr::f64 harmonicAngleDeg = tidalHarmonics[i].offset + timeHours * tidalHarmonics[i].speed;
            harmonicAngleDeg=harmonicAngleDeg-Utilities::round(harmonicAngleDeg/360)*360; //Normalise (DEGREES)
            tideHeight+= tidalHarmonics[i].amplitude*cos(harmonicAngleDeg*irr::core::DEGTORAD64);
        }

    }
    //std::cout << "Last high tide time:" << Utilities::timestampToString(highTideTime(absoluteTime,-1)) << std::endl;

}

irr::f32 Tide::getTideHeight() {
    return tideHeight;
}

irr::core::vector2df Tide::getTidalStream(irr::f32 posX, irr::f32 posZ, uint64_t absoluteTime) const {
    irr::core::vector2df tidalStream;
    tidalStream.X = 0; //Speed in metres per second in X direction (+ve is motion towards East)
    tidalStream.Y = 00; //Speed in metres per second in Z direction (+ve is motion towards North)



    return tidalStream;
}

irr::f32 Tide::getTideGradient(uint64_t absoluteTime) const {
    //return der(TideHeight) (in ?? units)
    //tim is absolute time tide is required for
    //TO BE TESTED!

    f32 timeHours = f32(absoluteTime)/SECONDS_IN_HOUR;
	f32 der=0;

	for (unsigned int i=1; i<tidalHarmonics.size(); i++) { //0th component has no gradient
        //;HarmonicHeight#=Tide(i,0)*Cos( Tide(i,1) + tim_hours*Tide(i,2)) ;tide(n,0) is amplitude, 1 is offset (rad), 2 is speed (rad/s)
		f32 harmonicAngle=tidalHarmonics.at(i).offset+timeHours*tidalHarmonics.at(i).speed;
		//reduce to range 0 to 360
		harmonicAngle=harmonicAngle-floor(harmonicAngle/360)*360;

		f32 harmonicDer=-1*tidalHarmonics.at(i).speed*tidalHarmonics.at(i).amplitude*sin(harmonicAngle*core::DEGTORAD);
		//
		//	;a cos (w t + c)
		//	;->
		//	;-wa sin(w t + c)

		der = der + harmonicDer;
	}
	return der;
}

uint64_t Tide::highTideTime(uint startSearchTime, int searchDirection) const {
//find the next high tide time in s before or after start_search_time. if search_direction is positive, search forward in time
//do this by finding when sum of derivatives of harmonics goes from +ve to -ve

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

uint64_t Tide::lowTideTime(uint startSearchTime, int searchDirection) const {
//find the next low tide time in s before or after start_search_time. if search_direction is positive, search forward in time
//do this by finding when sum of derivatives of harmonics goes from -ve to +ve

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
