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
    //std::cout << "numberOfHarmonics " << numberOfHarmonics <<std::endl;

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

}

irr::f32 Tide::getTideHeight() {
    return tideHeight;
}

