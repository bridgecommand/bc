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

#include "LandLights.hpp"

#include "NavLight.hpp"
#include "IniFile.hpp"
#include "Constants.hpp"
#include "Terrain.hpp"
#include "SimulationModel.hpp"

//using namespace irr;

LandLights::LandLights()
{

}

LandLights::~LandLights()
{
    //Drop navLights
    for(std::vector<NavLight*>::iterator it = landLights.begin(); it != landLights.end(); ++it) {
        delete (*it);
    }
    landLights.clear();
}

void LandLights::load(const std::string& worldName, irr::scene::ISceneManager* smgr, SimulationModel* model, const Terrain& terrain)
{
    //get light.ini filename
    std::string scenarioLightFilename = worldName;
    scenarioLightFilename.append("/light.ini");

    irr::u32 numberOfLights;
    numberOfLights = IniFile::iniFileTou32(scenarioLightFilename,"Number");
    //Run through lights, and check if any are not buoy lights
    for (irr::u32 currentLight=1;currentLight<=numberOfLights;currentLight++) {
        if (IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("Buoy",currentLight)) == 0 ) {
            //If not a buoy light
            irr::f32 lightX = model->longToX(IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("Long",currentLight)));
            irr::f32 lightZ = model->latToZ(IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("Lat",currentLight)));
            irr::f32 lightY = IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("Height",currentLight));
            if (IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("Absolute",currentLight)) != 1) {
                lightY = lightY + terrain.getHeight(lightX,lightZ);
            }

            irr::f32 lightR = IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("Red",currentLight));
            irr::f32 lightG = IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("Green",currentLight));
            irr::f32 lightB = IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("Blue",currentLight));
            irr::f32 lightRange = IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("Range",currentLight));
            std::string lightSequence = IniFile::iniFileToString(scenarioLightFilename,IniFile::enumerate1("Sequence",currentLight));
            irr::u32 phaseStart = IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("PhaseStart",currentLight));
            irr::f32 lightStart = IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("StartAngle",currentLight));
            irr::f32 lightEnd = IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("EndAngle",currentLight));
            lightRange = lightRange * M_IN_NM;


            landLights.push_back(new NavLight (0,smgr, irr::core::vector3df(lightX,lightY,lightZ),irr::video::SColor(255,lightR,lightG,lightB),lightStart,lightEnd,lightRange, lightSequence, phaseStart));
        }
    }

}

void LandLights::update(irr::f32 deltaTime, irr::f32 scenarioTime, irr::u32 lightLevel)
{
    for(std::vector<NavLight*>::iterator it = landLights.begin(); it != landLights.end(); ++it) {
        (*it)->update(scenarioTime, lightLevel);
    }
}

irr::u32 LandLights::getNumber() const
{
    return landLights.size();
}

void LandLights::moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ)
{
    for(std::vector<NavLight*>::iterator it = landLights.begin(); it != landLights.end(); ++it) {
        (*it)->moveNode(deltaX,deltaY,deltaZ);
    }
}
