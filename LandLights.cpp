#include "irrlicht.h"

#include "LandLights.hpp"

#include "NavLight.hpp"
#include "IniFile.hpp"
#include "Constants.hpp"
#include "Terrain.hpp"
#include "SimulationModel.hpp"

using namespace irr;

LandLights::LandLights()
{

}

LandLights::~LandLights()
{
    //dtor
}

void LandLights::load(const std::string& worldName, irr::scene::ISceneManager* smgr, SimulationModel* model, const Terrain& terrain)
{
    //get light.ini filename
    std::string scenarioLightFilename = worldName;
    scenarioLightFilename.append("/light.ini");

    u32 numberOfLights;
    numberOfLights = IniFile::iniFileTou32(scenarioLightFilename,"Number");
    //Run through lights, and check if any are not buoy lights
    for (u32 currentLight=1;currentLight<=numberOfLights;currentLight++) {
        if (IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("Buoy",currentLight)) == 0 ) {
            //If not a buoy light
            f32 lightX = model->longToX(IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("Long",currentLight)));
            f32 lightZ = model->latToZ(IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("Lat",currentLight)));
            f32 lightY = IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("Height",currentLight));
            if (IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("Absolute",currentLight)) != 1) {
                f32 lightY = lightY + terrain.getHeight(lightX,lightZ);
            }

            f32 lightR = IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("Red",currentLight));
            f32 lightG = IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("Green",currentLight));
            f32 lightB = IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("Blue",currentLight));
            f32 lightRange = IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("Range",currentLight));
            std::string lightSequence = IniFile::iniFileToString(scenarioLightFilename,IniFile::enumerate1("Sequence",currentLight));
            f32 lightStart = IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("StartAngle",currentLight));
            f32 lightEnd = IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("EndAngle",currentLight));
            lightRange = lightRange * M_IN_NM;


            landLights.push_back(NavLight (0,smgr,core::dimension2d<f32>(5, 5), core::vector3df(lightX,lightY,lightZ),video::SColor(255,lightR,lightG,lightB),lightStart,lightEnd,lightRange, lightSequence));
        }
    }

}

void LandLights::update(irr::f32 deltaTime, irr::f32 scenarioTime, irr::core::vector3df viewPosition, irr::u32 lightLevel)
{
    for(std::vector<NavLight>::iterator it = landLights.begin(); it != landLights.end(); ++it) {
        it->update(scenarioTime, viewPosition, lightLevel);
    }
}

irr::u32 LandLights::getNumber() const
{
    return landLights.size();
}

