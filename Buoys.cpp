#include "irrlicht.h"

#include "Buoys.hpp"

#include "Buoy.hpp"
#include "NavLight.hpp"
#include "IniFile.hpp"
#include "Constants.hpp"
#include "RadarData.hpp"
#include "SimulationModel.hpp"

using namespace irr;

Buoys::Buoys()
{

}

Buoys::~Buoys()
{
    //dtor
}

void Buoys::load(const std::string& worldName, irr::scene::ISceneManager* smgr, SimulationModel* model)
{
    //get buoy.ini filename
    std::string scenarioBuoyFilename = worldName;
    scenarioBuoyFilename.append("/buoy.ini");

    //get light.ini filename
    std::string scenarioLightFilename = worldName;
    scenarioLightFilename.append("/light.ini");

    //Find number of buoys
    u32 numberOfBuoys;
    numberOfBuoys = IniFile::iniFileTou32(scenarioBuoyFilename,"Number");
    for(u32 currentBuoy=1;currentBuoy<=numberOfBuoys;currentBuoy++) {

        //Get buoy type and construct filename
        std::string buoyName = IniFile::iniFileToString(scenarioBuoyFilename,IniFile::enumerate1("Type",currentBuoy));
        //Get buoy position
        f32 buoyX = model->longToX(IniFile::iniFileTof32(scenarioBuoyFilename,IniFile::enumerate1("Long",currentBuoy)));
        f32 buoyZ = model->latToZ(IniFile::iniFileTof32(scenarioBuoyFilename,IniFile::enumerate1("Lat",currentBuoy)));

        //get buoy RCS if set
        f32 rcs = IniFile::iniFileTof32(scenarioBuoyFilename,IniFile::enumerate1("RCS",currentBuoy));

        //Create buoy and load into vector
        buoys.push_back(Buoy (buoyName.c_str(),core::vector3df(buoyX,0.0f,buoyZ),rcs,smgr));

        //Load buoy light information from light.ini file if available

        //Find number of lights
        u32 numberOfLights;
        numberOfLights = IniFile::iniFileTou32(scenarioLightFilename,"Number");
        //Run through lights, and check if any are associated with this buoy
        for (u32 currentLight=1;currentLight<=numberOfLights;currentLight++) {
            if (IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("Buoy",currentLight)) ==currentBuoy ) {
                //Light on this buoy, add a light to the buoysLights vector in this location if required (FIXME: Make a child of the sea)
                f32 lightHeight = IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("Height",currentLight));
                f32 lightR = IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("Red",currentLight));
                f32 lightG = IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("Green",currentLight));
                f32 lightB = IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("Blue",currentLight));
                f32 lightRange = IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("Range",currentLight));
                std::string lightSequence = IniFile::iniFileToString(scenarioLightFilename,IniFile::enumerate1("Sequence",currentLight));
                f32 lightStart = IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("StartAngle",currentLight));
                f32 lightEnd = IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("EndAngle",currentLight));
                lightRange = lightRange * M_IN_NM;


                buoysLights.push_back(NavLight (0,smgr,core::dimension2d<f32>(5, 5), core::vector3df(buoyX,lightHeight,buoyZ),video::SColor(255,lightR,lightG,lightB),lightStart,lightEnd,lightRange, lightSequence));
            }
        }
    }
}

void Buoys::update(irr::f32 deltaTime, irr::f32 scenarioTime, irr::core::vector3df viewPosition)
{
    for(std::vector<NavLight>::iterator it = buoysLights.begin(); it != buoysLights.end(); ++it) {
        it->update(scenarioTime, viewPosition);
    }
}

RadarData Buoys::getRadarData(irr::u32 number, irr::core::vector3df scannerPosition) const
//Get data for OtherShip (number) relative to scannerPosition
//Fixme: Shares a lot of code with OtherShips::getRadarData. Can these be inherited from a common parent?
{
    RadarData radarData;

    if (number<=buoys.size()) {
        //Get information about this buoy, and return a RadarData struct containing info
        irr::core::vector3df contactPosition = buoys[number-1].getPosition();
        irr::core::vector3df relativePosition = contactPosition-scannerPosition;

        radarData.relX = relativePosition.X;
        radarData.relZ = relativePosition.Z;

        radarData.angle = relativePosition.getHorizontalAngle().Y;
        radarData.range = relativePosition.getLength();

        radarData.heading = 0.0;

        //Fixme: Complete implementation to include all radarData fields
        radarData.height=999; //Fixme: Set these properly!
        radarData.solidHeight=9; //Fixme: Set these properly!
        radarData.radarHorizon=99999; //Fixme: Set these properly! //Only used for tracking contacts outside current radar visibility range
        radarData.length=buoys[number-1].getLength();
        radarData.rcs=buoys[number-1].getRCS();

        radarData.minRange;
        radarData.maxRange;
        radarData.minAngle;
        radarData.maxAngle;

        //Initial defaults: Will need changing with full implementation
        radarData.hidden=false;
        radarData.racon=""; //Racon code if set
        radarData.raconOffsetTime=0.0;
        radarData.SART=false;

    }

    return radarData;
}

irr::u32 Buoys::getNumber() const
{
    return buoys.size();
}
