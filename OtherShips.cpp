#include "irrlicht.h"

#include "OtherShips.hpp"

#include "Constants.hpp"
#include "OtherShip.hpp"
#include "IniFile.hpp"
#include "RadarData.hpp"
#include "SimulationModel.hpp"

#include <iostream> //debugging

using namespace irr;

OtherShips::OtherShips()
{

}

OtherShips::~OtherShips()
{
    //dtor
}

void OtherShips::load(const std::string& scenarioName, irr::scene::ISceneManager* smgr, SimulationModel* model)
{

    //construct path
    std::string scenarioOtherShipsFilename = scenarioName;
    scenarioOtherShipsFilename.append("/othership.ini");

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

            //Load leg information
            std::vector<Leg> legs;
            irr::u32 numberOfLegs = IniFile::iniFileTou32(scenarioOtherShipsFilename,IniFile::enumerate1("Legs",i));
            irr::f32 legStartTime = 0;
            for(irr::u32 currentLegNo=1; currentLegNo<=numberOfLegs; currentLegNo++){
                //go through each leg (if any), and load
                Leg currentLeg;
                currentLeg.bearing = IniFile::iniFileTof32(scenarioOtherShipsFilename,IniFile::enumerate2("Bearing",i,currentLegNo));
                currentLeg.speed = IniFile::iniFileTof32(scenarioOtherShipsFilename,IniFile::enumerate2("Speed",i,currentLegNo));
                currentLeg.distance = IniFile::iniFileTof32(scenarioOtherShipsFilename,IniFile::enumerate2("Distance",i,currentLegNo));
                currentLeg.startTime = legStartTime;
                legs.push_back(currentLeg);

                //find the start time for the next leg
                legStartTime = legStartTime + SECONDS_IN_HOUR*(currentLeg.distance/currentLeg.speed); // nm/kts -> hours, so convert to seconds
            }
            //add a final 'stop' leg, which the ship will remain on after it has passed the other legs.
            Leg stopLeg;
            stopLeg.bearing=0;
            stopLeg.speed=0;
            stopLeg.distance=0;
            stopLeg.startTime = legStartTime;
            legs.push_back(stopLeg);

            //Create otherShip and load into vector
            otherShips.push_back(OtherShip (otherShipName,core::vector3df(shipX,0.0f,shipZ),legs,smgr));
        }
    }
}

void OtherShips::update(irr::f32 deltaTime, irr::f32 scenarioTime, irr::core::vector3df viewPosition)
{
    for(std::vector<OtherShip>::iterator it = otherShips.begin(); it != otherShips.end(); ++it) {
        it->update(deltaTime, scenarioTime, viewPosition);
    }
}

RadarData OtherShips::getRadarData(irr::u32 number, irr::core::vector3df scannerPosition) const
//Get data for OtherShip (number) relative to scannerPosition
{
    //Fixme: Implementation should probably just call otherShips[number-1].getRadarData

    RadarData radarData;

    if (number<=otherShips.size()) {
    //Get information about this otherShip, and return a RadarData struct containing info
        irr::core::vector3df contactPosition = otherShips[number-1].getPosition();
        irr::core::vector3df relativePosition = contactPosition-scannerPosition;

        radarData.relX = relativePosition.X;
        radarData.relZ = relativePosition.Z;
        radarData.angle = relativePosition.getHorizontalAngle().Y;
        radarData.range = relativePosition.getLength();
        radarData.heading = otherShips[number-1].getHeading();

        //Fixme: Complete implementation to include all radarData fields
        radarData.height;
        radarData.solidHeight;
        radarData.radarHorizon; //Only used for tracking contacts outside current radar visibility range
        radarData.length=otherShips[number-1].getLength();
        radarData.rcs=otherShips[number-1].getRCS();

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

irr::u32 OtherShips::getNumber() const
{
    return otherShips.size();
}
