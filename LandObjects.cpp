#include "LandObjects.hpp"

#include "LandObject.hpp"
#include "IniFile.hpp"
#include "Terrain.hpp"
//#include "Constants.hpp"
#include "SimulationModel.hpp"

#include <iostream>

using namespace irr;

LandObjects::LandObjects()
{

}

LandObjects::~LandObjects()
{
    //dtor
}

void LandObjects::load(const std::string& worldName, irr::scene::ISceneManager* smgr, SimulationModel* model, const Terrain& terrain)
{
    //get landObject.ini filename
    std::string scenarioLandObjectFilename = worldName;
    scenarioLandObjectFilename.append("/landobject.ini");

    //Find number of objects
    u32 numberOfObjects;
    numberOfObjects = IniFile::iniFileTou32(scenarioLandObjectFilename,"Number");
    for(u32 currentObject=1;currentObject<=numberOfObjects;currentObject++) {

        //std::cout << "Loading land object " << currentObject << std::endl;

        //Get Object type and construct filename
        std::string objectName = IniFile::iniFileToString(scenarioLandObjectFilename,IniFile::enumerate1("Type",currentObject));
        //Get object position
        f32 objectX = model->longToX(IniFile::iniFileTof32(scenarioLandObjectFilename,IniFile::enumerate1("Long",currentObject)));
        f32 objectZ = model->latToZ(IniFile::iniFileTof32(scenarioLandObjectFilename,IniFile::enumerate1("Lat",currentObject)));
        f32 objectY = IniFile::iniFileTof32(scenarioLandObjectFilename,IniFile::enumerate1("HeightCorrection",currentObject));;
        //Check if land object is given in absolute height, or relative to terrain.
        if (IniFile::iniFileTou32(scenarioLandObjectFilename,IniFile::enumerate1("Absolute",currentObject))!=1) {
            objectY += terrain.getHeight(objectX,objectZ);
        }

        //Get rotation (FIXME: Double check sign)
        f32 rotation = IniFile::iniFileTof32(scenarioLandObjectFilename,IniFile::enumerate1("Rotation",currentObject));

        //Create land object and load into vector
        landObjects.push_back(LandObject (objectName.c_str(),core::vector3df(objectX,objectY,objectZ),rotation,smgr));
        //std::cout << "Irr Loaded land object " << objectName << " at x:" << objectX << " Terrain y:" << terrain.getHeight(objectX,objectZ) << " z:" << objectZ << std::endl;

    }
}

irr::u32 LandObjects::getNumber() const
{
    return landObjects.size();
}
