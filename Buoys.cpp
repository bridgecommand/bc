#include "irrlicht.h"

#include "Buoys.hpp"

#include "Buoy.hpp"
#include "NavLight.hpp"
#include "IniFile.hpp"
#include "Constants.hpp"
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
        if (numberOfBuoys > 0)
        {
            for(u32 currentBuoy=1;currentBuoy<=numberOfBuoys;currentBuoy++)
            {
                //Get buoy type and construct filename
                std::string buoyName = IniFile::iniFileToString(scenarioBuoyFilename,IniFile::enumerate1("Type",currentBuoy));
                //Get buoy position
                f32 buoyX = model->longToX(IniFile::iniFileTof32(scenarioBuoyFilename,IniFile::enumerate1("Long",currentBuoy)));
                f32 buoyZ = model->latToZ(IniFile::iniFileTof32(scenarioBuoyFilename,IniFile::enumerate1("Lat",currentBuoy)));

                //FIXME: This section should be re-factored into the Buoy object

                //Load from individual buoy.ini file if it exists
                std::string buoyIniFilename = "Models/Buoy/";
                buoyIniFilename.append(buoyName);
                buoyIniFilename.append("/buoy.ini");

                //get filename from ini file (or empty string if file doesn't exist)
                std::string buoyFileName = IniFile::iniFileToString(buoyIniFilename,"FileName");
                if (buoyFileName=="") {
                    buoyFileName = "buoy.x"; //Default if not set
                }
                //get scale factor from ini file (or zero if not set - assume 1)
                f32 buoyScale = IniFile::iniFileTof32(buoyIniFilename,"Scalefactor");
                if (buoyScale==0.0) {
                    buoyScale = 1.0; //Default if not set
                }

                std::string buoyFullPath = "Models/Buoy/"; //FIXME: Use proper path handling
                buoyFullPath.append(buoyName);
                buoyFullPath.append("/");
                buoyFullPath.append(buoyFileName);

                //End of refactor required

                //Create buoy and load into vector
                buoys.push_back(Buoy (buoyFullPath.c_str(),core::vector3df(buoyX,0.0f,buoyZ),buoyScale,smgr));

                //Load buoy light information from light.ini file if available

                //Find number of lights
                u32 numberOfLights;
                numberOfLights = IniFile::iniFileTou32(scenarioLightFilename,"Number");
                if (numberOfLights > 0){
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
        }
}

void Buoys::update(irr::f32 deltaTime, irr::f32 scenarioTime, irr::core::vector3df viewPosition)
{
    for(std::vector<NavLight>::iterator it = buoysLights.begin(); it != buoysLights.end(); ++it) {
        it->update(scenarioTime, viewPosition);
    }
}
