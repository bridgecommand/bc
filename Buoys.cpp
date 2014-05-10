#include "irrlicht.h"

#include "Buoys.hpp"

#include "Buoy.hpp"
#include "IniFile.hpp"
#include "SimulationModel.hpp"

using namespace irr;

Buoys::Buoys()
{

}

Buoys::~Buoys()
{
    //dtor
}

void Buoys::load(const std::string& scenarioBuoyFilename, irr::scene::ISceneManager* smgr, SimulationModel* model)
{
        //Find number of buoys
        u32 numberOfBuoys;
        numberOfBuoys = IniFile::iniFileTou32(scenarioBuoyFilename,"Number");
        if (numberOfBuoys > 0)
        {
            for(u32 i=1;i<=numberOfBuoys;i++)
            {
                //Get buoy type and construct filename
                std::string buoyName = IniFile::iniFileToString(scenarioBuoyFilename,IniFile::enumerate1("Type",i));
                //Get buoy position
                f32 buoyX = model->longToX(IniFile::iniFileTof32(scenarioBuoyFilename,IniFile::enumerate1("Long",i)));
                f32 buoyZ = model->latToZ(IniFile::iniFileTof32(scenarioBuoyFilename,IniFile::enumerate1("Lat",i)));

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

                //Create buoy and load into vector
                buoys.push_back(Buoy (buoyFullPath.c_str(),core::vector3df(buoyX,0.0f,buoyZ),buoyScale,smgr));
            }
        }
}
