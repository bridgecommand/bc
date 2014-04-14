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

void Buoys::loadBuoys(const irr::io::path& scenarioBuoyFilename, irr::scene::ISceneManager* smgr, SimulationModel* model)
{
    //Fixme: Doesn't use scenarioBuoyFilename, instead currently hardcoded to "buoy.ini"
    //Start: Will be moved into a Buoys object (?)
        //load info from buoy.ini file FIXME: The buoy.ini location is currently hard-coded in the program root directory, and paths aren't properly handled
        //The Buoy constructor should probably take the location of the individual buoy's folder, and itself look in the local buoy.ini file for the filename and scaling etc
        //Find number of buoys
        u32 numberOfBuoys;
        numberOfBuoys = IniFile::iniFileTou32("Buoy.ini","Number");
        if (numberOfBuoys > 0)
        {
            for(u32 i=1;i<=numberOfBuoys;i++)
            {
                //Get buoy type and construct filename (FIXME: In due course, the buoy constructor should be given the path to the buoy folder, and use this to look in the local buoy.ini file for the model filename and scaling.
                std::string buoyName = IniFile::iniFileToString("Buoy.ini",IniFile::enumerate1("Type",i));

                //Load from buoy.ini file if it exists
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

                //Get buoy position
                f32 buoyX = model->longToX(IniFile::iniFileTof32("Buoy.ini",IniFile::enumerate1("Long",i)));
                f32 buoyZ = model->latToZ(IniFile::iniFileTof32("Buoy.ini",IniFile::enumerate1("Lat",i)));
                //Create buoy and load into vector
                buoys.push_back(Buoy (buoyFullPath.c_str(),core::vector3df(buoyX,0.0f,buoyZ),buoyScale,smgr));
            }
        }
}
