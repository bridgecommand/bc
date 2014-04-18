#include "irrlicht.h"

#include "OtherShips.hpp"

#include "OtherShip.hpp"
#include "IniFile.hpp"
#include "SimulationModel.hpp"

using namespace irr;

OtherShips::OtherShips()
{

}

OtherShips::~OtherShips()
{
    //dtor
}

void OtherShips::loadOtherShips(const std::string& scenarioOtherShipsFilename, irr::scene::ISceneManager* smgr, SimulationModel* model)
{

    //Find number of other ships
        u32 numberOfOtherShips;
        numberOfOtherShips = IniFile::iniFileTou32(scenarioOtherShipsFilename,"Number");
        if (numberOfOtherShips > 0)
        {
            for(u32 i=1;i<=numberOfOtherShips;i++)
            {
                //Get ship type and construct filename (FIXME: In due course, the other ship constructor should be given the path to the otherShip folder, and use this to look in the local boat.ini file for the model filename and scaling.
                std::string otherShipName = IniFile::iniFileToString(scenarioOtherShipsFilename,IniFile::enumerate1("Type",i));
                //Get initial position
                f32 shipX = model->longToX(IniFile::iniFileTof32(scenarioOtherShipsFilename,IniFile::enumerate1("InitLong",i)));
                f32 shipZ = model->latToZ(IniFile::iniFileTof32(scenarioOtherShipsFilename,IniFile::enumerate1("InitLat",i)));

                //Fixme: also load leg information and decide how to handle this

                //Load from individual boat.ini file
                std::string shipIniFilename = "Models/Othership/";
                shipIniFilename.append(otherShipName);
                shipIniFilename.append("/boat.ini");

                //get filename from ini file
                std::string shipFileName = IniFile::iniFileToString(shipIniFilename,"FileName");

                //get scale factor from ini file (or zero if not set - assume 1)
                f32 shipScale = IniFile::iniFileTof32(shipIniFilename,"Scalefactor");
                if (shipScale==0.0) {
                    shipScale = 1.0; //Default if not set
                }

                f32 yCorrection = IniFile::iniFileTof32(shipIniFilename,"YCorrection");

                //Fixme: also load y offset etc information, and handle like own ships

                std::string shipFullPath = "Models/Othership/"; //FIXME: Use proper path handling
                shipFullPath.append(otherShipName);
                shipFullPath.append("/");
                shipFullPath.append(shipFileName);

                //Create otherShip and load into vector
                otherShips.push_back(OtherShip (shipFullPath.c_str(),core::vector3df(shipX,0.0f,shipZ),shipScale,yCorrection,smgr));
            }
        }
}
