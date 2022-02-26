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

#include "Buoy.hpp"
#include "NavLight.hpp"
#include "IniFile.hpp"
#include "Constants.hpp"
#include "RadarData.hpp"
#include "SimulationModel.hpp"

//using namespace irr;

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
    irr::u32 numberOfBuoys;
    numberOfBuoys = IniFile::iniFileTou32(scenarioBuoyFilename,"Number");
    for(irr::u32 currentBuoy=1;currentBuoy<=numberOfBuoys;currentBuoy++) {

        //Get buoy type and construct filename
        std::string buoyName = IniFile::iniFileToString(scenarioBuoyFilename,IniFile::enumerate1("Type",currentBuoy));
        //Get buoy position
        irr::f32 buoyX = model->longToX(IniFile::iniFileTof32(scenarioBuoyFilename,IniFile::enumerate1("Long",currentBuoy)));
        irr::f32 buoyZ = model->latToZ(IniFile::iniFileTof32(scenarioBuoyFilename,IniFile::enumerate1("Lat",currentBuoy)));

        //get buoy RCS if set
        irr::f32 rcs = IniFile::iniFileTof32(scenarioBuoyFilename,IniFile::enumerate1("RCS",currentBuoy));

        irr::f32 heightCorrection = IniFile::iniFileTof32(scenarioBuoyFilename,IniFile::enumerate1("HeightCorrection",currentBuoy));


        bool floating = true;
        if (IniFile::iniFileTou32(scenarioBuoyFilename,IniFile::enumerate1("Grounded",currentBuoy))==1 ) {
            floating = false;
        }

        //Create buoy and load into vector
        buoys.push_back(Buoy (buoyName.c_str(),worldName,irr::core::vector3df(buoyX,0.0f,buoyZ),rcs,floating,heightCorrection,smgr,dev));

        //Find scene node
        irr::scene::ISceneNode* buoyNode = buoys.back().getSceneNode();

        //Load buoy light information from light.ini file if available

        //Find number of lights
        irr::u32 numberOfLights;
        numberOfLights = IniFile::iniFileTou32(scenarioLightFilename,"Number");
        //Run through lights, and check if any are associated with this buoy
        for (irr::u32 currentLight=1;currentLight<=numberOfLights;currentLight++) {
            if (IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("Buoy",currentLight)) ==currentBuoy ) {
                //Light on this buoy, add a light to the buoysLights vector in this location if required (FIXME: Think about response to waves?)
                irr::f32 lightHeight = IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("Height",currentLight));
                irr::u32 lightR = IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("Red",currentLight));
                irr::u32 lightG = IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("Green",currentLight));
                irr::u32 lightB = IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("Blue",currentLight));
                irr::f32 lightRange = IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("Range",currentLight));
                std::string lightSequence = IniFile::iniFileToString(scenarioLightFilename,IniFile::enumerate1("Sequence",currentLight));
                irr::u32 phaseStart = IniFile::iniFileTou32(scenarioLightFilename,IniFile::enumerate1("PhaseStart",currentLight));
                irr::f32 lightStart = IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("StartAngle",currentLight));
                irr::f32 lightEnd = IniFile::iniFileTof32(scenarioLightFilename,IniFile::enumerate1("EndAngle",currentLight));
                lightRange = lightRange * M_IN_NM;

                //Scale height to adjust for buoy scaling (As buoy lights given absolute heights, so needs to be scaled to match parent)
                if (buoyNode->getScale().Y>0) {
                    lightHeight/=buoyNode->getScale().Y;
                }
                //Create buoy light as a child of the buoy
                buoysLights.push_back(new NavLight (buoyNode,smgr, irr::core::vector3df(0,lightHeight,0),irr::video::SColor(255,lightR,lightG,lightB),lightStart,lightEnd,lightRange, lightSequence, phaseStart));
            }
        }
    }
}

void Buoys::update(irr::f32 deltaTime, irr::f32 scenarioTime, irr::f32 tideHeight, irr::u32 lightLevel, irr::core::vector3df ownShipPosition, irr::f32 ownShipLength)
{
    for(std::vector<Buoy>::iterator it = buoys.begin(); it != buoys.end(); ++it) {
        irr::f32 xPos, yPos, zPos;
        irr::core::vector3df pos = it->getPosition();
        xPos = pos.X;
        if (it->getFloating()) {
            yPos = tideHeight + model->getWaveHeight(pos.X,pos.Z) + it->getHeightCorrection();
        } else {
            yPos = 0 + it->getHeightCorrection();
        }
        zPos = pos.Z;
        it->setPosition(irr::core::vector3df(xPos,yPos,zPos));

        if (it->getFloating()) {
            irr::f32 angleX, angleZ;
            irr::core::vector2df normals = model->getLocalNormals(pos.X,pos.Z);
            angleX = normals.X * irr::core::RADTODEG;//Assume small angle, so just convert rad to deg
            angleZ = normals.Y * irr::core::RADTODEG;//Assume small angle, so just convert rad to deg
            it->setRotation(irr::core::vector3df(angleX,0,angleZ));
        } else {
            it->setRotation(irr::core::vector3df(0,0,0));
        }

        //Set or clear triangle selector depending on distance from own ship
        if (it->getSceneNode()->getAbsolutePosition().getDistanceFrom(ownShipPosition) < 2*ownShipLength) {
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

RadarData Buoys::getRadarData(irr::u32 number, irr::core::vector3df scannerPosition) const
//Get data for Buoy (number) relative to scannerPosition
{
    RadarData radarData;

    if (number<=buoys.size()) {
        radarData = buoys[number-1].getRadarData(scannerPosition);
    }

    return radarData;
}

irr::u32 Buoys::getNumber() const
{
    return buoys.size();
}

irr::core::vector3df Buoys::getPosition(int number) const
{
    if (number < (int)buoys.size()) {
        return buoys.at(number).getPosition();
    } else {
        return irr::core::vector3df(0,0,0);
    }

}

void Buoys::moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ)
{
    for(std::vector<Buoy>::iterator it = buoys.begin(); it != buoys.end(); ++it) {
        it->moveNode(deltaX,deltaY,deltaZ);
    }

    //Note the light is a child of the buoy, so it moves with it
}
