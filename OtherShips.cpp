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

void OtherShips::load(const std::string& scenarioOtherShipsFilename, irr::scene::ISceneManager* smgr, SimulationModel* model)
{

    //Find number of other ships
    u32 numberOfOtherShips;
    numberOfOtherShips = IniFile::iniFileTou32(scenarioOtherShipsFilename,"Number");
    if (numberOfOtherShips > 0)
    {
        for(u32 i=1;i<=numberOfOtherShips;i++)
        {
            //Get ship type and construct filename
            std::string otherShipName = IniFile::iniFileToString(scenarioOtherShipsFilename,IniFile::enumerate1("Type",i));
            //Get initial position
            f32 shipX = model->longToX(IniFile::iniFileTof32(scenarioOtherShipsFilename,IniFile::enumerate1("InitLong",i)));
            f32 shipZ = model->latToZ(IniFile::iniFileTof32(scenarioOtherShipsFilename,IniFile::enumerate1("InitLat",i)));

            //Fixme: also load leg information and decide how to handle this
            //in ini file, the format is (for each leg): Bearing (deg), Speed (kts), distance (nm)
            std::vector<Leg> legs;
            irr::u32 numberOfLegs = IniFile::iniFileTou32(scenarioOtherShipsFilename,IniFile::enumerate1("Legs",i));
            for(irr::u32 currentLegNo=1; currentLegNo<=numberOfLegs; currentLegNo++){
                //go through each leg (if any), and load
                Leg currentLeg;
                currentLeg.bearing = IniFile::iniFileTof32(scenarioOtherShipsFilename,IniFile::enumerate2("Bearing",i,currentLegNo));
                currentLeg.speed = IniFile::iniFileTof32(scenarioOtherShipsFilename,IniFile::enumerate2("Speed",i,currentLegNo));
                currentLeg.distance = IniFile::iniFileTof32(scenarioOtherShipsFilename,IniFile::enumerate2("Distance",i,currentLegNo));
                currentLeg.startTime = 0; //Fixme: Work out when this leg should start.
                legs.push_back(currentLeg);
            }

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

            std::string shipFullPath = "Models/Othership/"; //FIXME: Use proper path handling
            shipFullPath.append(otherShipName);
            shipFullPath.append("/");
            shipFullPath.append(shipFileName);

            //Create otherShip and load into vector
            otherShips.push_back(OtherShip (shipFullPath.c_str(),core::vector3df(shipX,0.0f,shipZ),shipScale,yCorrection,legs,smgr));
        }
    }
}

void OtherShips::update()
{
    for(std::vector<OtherShip>::iterator it = otherShips.begin(); it != otherShips.end(); ++it) {
        it->update();
    }
}

