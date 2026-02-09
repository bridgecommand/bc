/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2014 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY Or FITNESS For A PARTICULAR PURPOSE.  See the
     GNU General Public License For more details.

     You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#include "Buoys.hpp"

#include "irrlicht.h"

#include "Buoy.hpp"
#include "NavLight.hpp"
#include "IniFile.hpp"
#include "Constants.hpp"
#include "RadarData.hpp"
#include "SimulationModel.hpp"

//using namespace irr;

// Local helpers for converting between bc::graphics::Vec3 and irr::core::vector3df
namespace {
    inline irr::core::vector3df toIrrVec(const bc::graphics::Vec3& v) { return {v.x, v.y, v.z}; }
    inline bc::graphics::Vec3 fromIrrVec(const irr::core::vector3df& v) { return {v.X, v.Y, v.Z}; }
}

Buoys::Buoys()
{

}

Buoys::~Buoys()
{
    for(std::vector<NavLight*>::iterator it = buoysLights.begin(); it != buoysLights.end(); ++it) {
        delete (*it);
    }
    buoysLights.clear();
}

void Buoys::load(const std::string& worldName, irr::scene::ISceneManager* smgr, SimulationModel* model, irr::IrrlichtDevice* dev)
{
    this->model = model;

    //get buoy.ini filename
    std::string scenarioBuoyFilename = worldName;
    scenarioBuoyFilename.append("/buoy.ini");

    //get light.ini filename
    std::string scenarioLightFilename = worldName;
    scenarioLightFilename.append("/light.ini");

    //Find number of buoys
    uint32_t numberOfBuoys;
    numberOfBuoys = IniFile::iniFileTou32(scenarioBuoyFilename,"Number");
    for(uint32_t currentBuoy=1;currentBuoy<=numberOfBuoys;currentBuoy++) {

        //Get buoy type and construct filename
        std::string buoyName = IniFile::iniFileToString(scenarioBuoyFilename,IniFile::enumerate1("Type",currentBuoy));
        //Get buoy position
        float buoyX = model->longToX(IniFile::iniFileTof32(scenarioBuoyFilename,IniFile::enumerate1("Long",currentBuoy)));
        float buoyZ = model->latToZ(IniFile::iniFileTof32(scenarioBuoyFilename,IniFile::enumerate1("Lat",currentBuoy)));

        //get buoy RCS if set
        float rcs = IniFile::iniFileTof32(scenarioBuoyFilename,IniFile::enumerate1("RCS",currentBuoy));

        float heightCorrection = IniFile::iniFileTof32(scenarioBuoyFilename,IniFile::enumerate1("HeightCorrection",currentBuoy));


        bool floating = true;
        if (IniFile::iniFileTou32(scenarioBuoyFilename,IniFile::enumerate1("Grounded",currentBuoy))==1 ) {
            floating = false;
        }

        //Create buoy and load into vector
        std::string internalName = "Buoy_";
        internalName.append(std::to_string(currentBuoy-1)); // -1 as we want index from 0
        buoys.push_back(Buoy (buoyName.c_str(),internalName,worldName,bc::graphics::Vec3(buoyX,0.0f,buoyZ),rcs,floating,heightCorrection,smgr,dev));

        //Find scene node
        irr::scene::ISceneNode* buoyNode = buoys.back().getSceneNode();

        //Load buoy light information from light.ini file if available

        //Find number of lights
        uint32_t numberOfLights;
        numberOfLights = IniFile::iniFileTou32(scenarioLightFilename,"Number");
        //Run through lights, and check if any are associated with this buoy
        for (uint32_t currentLight=1;currentLight<=numberOfLights;currentLight++) {
            if (IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("Buoy",currentLight)) ==currentBuoy ) {
                //Light on this buoy, add a light to the buoysLights vector in this location if required (FIXME: Think about response to waves?)
                float lightHeight = IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("Height",currentLight));
                uint32_t lightR = IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("Red",currentLight));
                uint32_t lightG = IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("Green",currentLight));
                uint32_t lightB = IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("Blue",currentLight));
                float lightRange = IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("Range",currentLight));
                std::string lightSequence = IniFile::iniFileToString(scenarioLightFilename,IniFile::enumerate1("Sequence",currentLight));
                uint32_t phaseStart = IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("PhaseStart",currentLight));
                float lightStart = IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("StartAngle",currentLight));
                float lightEnd = IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("EndAngle",currentLight));
                lightRange = lightRange * M_IN_NM;

                //Scale height to adjust for buoy scaling (As buoy lights given absolute heights, so needs to be scaled to match parent)
                if (buoyNode->getScale().Y>0) {
                    lightHeight/=buoyNode->getScale().Y;
                }
                //Create buoy light as a child of the buoy
                buoysLights.push_back(new NavLight (buoyNode,smgr, bc::graphics::Vec3(0,lightHeight,0),bc::graphics::Color(255,lightR,lightG,lightB),lightStart,lightEnd,lightRange, lightSequence, phaseStart));
            }
        }
    }
}

void Buoys::update(float deltaTime, float scenarioTime, float tideHeight, uint32_t lightLevel, bc::graphics::Vec3 ownShipPosition, float ownShipLength)
{
    irr::core::vector3df irrOwnShipPos = toIrrVec(ownShipPosition);

    for(std::vector<Buoy>::iterator it = buoys.begin(); it != buoys.end(); ++it) {
        float xPos, yPos, zPos;
        bc::graphics::Vec3 pos = it->getPosition();
        xPos = pos.x;
        if (it->getFloating()) {
            yPos = tideHeight + model->getWaveHeight(pos.x,pos.z) + it->getHeightCorrection();
        } else {
            yPos = 0 + it->getHeightCorrection();
        }
        zPos = pos.z;
        it->setPosition(bc::graphics::Vec3(xPos,yPos,zPos));

        if (it->getFloating()) {
            float angleX, angleZ;
            bc::graphics::Vec2 normals = model->getLocalNormals(pos.x,pos.z);
            angleX = normals.x * irr::core::RADTODEG;//Assume small angle, so just convert rad to deg
            angleZ = normals.y * irr::core::RADTODEG;//Assume small angle, so just convert rad to deg
            it->setRotation(bc::graphics::Vec3(angleX,0,angleZ));
        } else {
            it->setRotation(bc::graphics::Vec3(0,0,0));
        }

        //Set or clear triangle selector depending on distance from own ship
        if (it->getSceneNode()->getAbsolutePosition().getDistanceFrom(irrOwnShipPos) < 2*ownShipLength) {
            it->enableTriangleSelector(true);
        } else {
            it->enableTriangleSelector(false);
        }

    }

    for(std::vector<NavLight*>::iterator it = buoysLights.begin(); it != buoysLights.end(); ++it) {

        //Update light size/visibility etc
        (*it)->update(scenarioTime, lightLevel);

        //Note that the buoy light is a child of the buoy, so it moves with it
    }
}

void Buoys::enableAllTriangleSelectors()
{
    for(std::vector<Buoy>::iterator it = buoys.begin(); it != buoys.end(); ++it) {
        // This will return to normal the next time Buoys::update is called.
        it->enableTriangleSelector(true);
    }
}

RadarData Buoys::getRadarData(uint32_t number, bc::graphics::Vec3 scannerPosition) const
//Get data for Buoy (number) relative to scannerPosition
{
    RadarData radarData;

    if (number<=buoys.size()) {
        radarData = buoys[number-1].getRadarData(scannerPosition);
    }

    return radarData;
}

uint32_t Buoys::getNumber() const
{
    return buoys.size();
}

bc::graphics::Vec3 Buoys::getPosition(int number) const
{
    if (number < (int)buoys.size()) {
        return buoys.at(number).getPosition();
    } else {
        return bc::graphics::Vec3(0,0,0);
    }

}

irr::scene::ISceneNode* Buoys::getSceneNode(int number)
{
    if (number < (int)buoys.size() && number >= 0) {
        return buoys.at(number).getSceneNode();
    } else {
        return 0;
    }
}


void Buoys::moveNode(float deltaX, float deltaY, float deltaZ)
{
    for(std::vector<Buoy>::iterator it = buoys.begin(); it != buoys.end(); ++it) {
        it->moveNode(deltaX,deltaY,deltaZ);
    }

    //Note the light is a child of the buoy, so it moves with it
}
