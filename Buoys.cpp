#include "irrlicht.h"

#include "Buoys.hpp"
#include "IniFile.hpp"

using namespace irr;

Buoys::Buoys()
{

}

Buoys::~Buoys()
{
    //dtor
}

void Buoys::loadBuoys(const irr::io::path& scenarioBuoyFilename, irr::scene::ISceneManager* smgr)
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
                f32 buoyX = longToX(IniFile::iniFileTof32("Buoy.ini",IniFile::enumerate1("Long",i)));
                f32 buoyZ = latToZ(IniFile::iniFileTof32("Buoy.ini",IniFile::enumerate1("Lat",i)));
                //Create buoy and load into vector
                buoys.push_back(Buoy (buoyFullPath.c_str(),core::vector3df(buoyX,0.0f,buoyZ),buoyScale,smgr));
            }
        }
}

const irr::f32 Buoys::longToX(irr::f32 longitude) //Fixme - Should be in simulation model or terrain
    {
        f32 terrainLong = -10.0; //FIXME: Hardcoding - these should all be member variables, set on terrain load
        f32 terrainXWidth = 3572.25;
        f32 terrainLongExtent = 0.05;
        return ((longitude - terrainLong ) * (terrainXWidth)) / terrainLongExtent;
    }

    const irr::f32 Buoys::latToZ(irr::f32 latitude) //Fixme - Should be in simulation model or terrain
    {
        f32 terrainLat = 50.0; //FIXME: Hardcoding - these should all be member variables, set on terrain load
        f32 terrainZWidth = 4447.8;
        f32 terrainLatExtent = 0.04;
        return ((latitude - terrainLat ) * (terrainZWidth)) / terrainLatExtent;
    }

