#include "irrlicht.h"

#include <iostream> //For debugging

#include "IniFile.hpp" //For ini file handling (buoy)
#include "SimulationModel.hpp"

using namespace irr;

SimulationModel::SimulationModel(IrrlichtDevice* dev, video::IVideoDriver* drv, scene::ISceneManager* scene, GUIMain* gui) //constructor, including own ship model
    {
        //get reference to scene manager
        device = dev;
        driver = drv;
        smgr = scene;
        guiMain = gui;

        //initialise variables - Start with a hardcoded initial position
        heading = 0;
        xPos = 864.34f;
        yPos = 0.0f;
        zPos = 619.317f;
        speed = 0.0f;

        //store time
        previousTime = device->getTimer()->getTime();

        //Load a ship model
        ownShip.loadModel("Models/Ownship/Atlantic85/Hull.3ds",core::vector3df(0,0,0),smgr);

        //make a camera, setting parent and offset
        camera.loadCamera(smgr,ownShip.getSceneNode(),core::vector3df(0.0f,0.9f,0.6f));

        //Add terrain
        Terrain terrain (smgr, driver);

        //Load buoys
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
        //End: Will be moved into a Buoys object(?)

        //add water
        Water water (smgr, driver);

        //sky box/dome
        Sky sky (smgr, driver);

        //make fog
        driver->setFog(video::SColor(128,128,128,128), video::EFT_FOG_LINEAR, 250, 1000, .003f, true, true);


    } //end of SimulationModel constructor

    void SimulationModel::setSpeed(f32 spd)
    {
         speed = spd;
    }

    const irr::f32 SimulationModel::getSpeed()
    {
        return(speed);
    }

    void SimulationModel::setHeading(f32 hdg)
    {
         heading = hdg;
    }

    const irr::f32 SimulationModel::getHeading()
    {
        return(heading);
    }

    const irr::f32 SimulationModel::longToX(irr::f32 longitude)
    {
        f32 terrainLong = -10.0; //FIXME: Hardcoding - these should all be member variables, set on terrain load
        f32 terrainXWidth = 3572.25;
        f32 terrainLongExtent = 0.05;
        return ((longitude - terrainLong ) * (terrainXWidth)) / terrainLongExtent;
    }

    const irr::f32 SimulationModel::latToZ(irr::f32 latitude)
    {
        f32 terrainLat = 50.0; //FIXME: Hardcoding - these should all be member variables, set on terrain load
        f32 terrainZWidth = 4447.8;
        f32 terrainLatExtent = 0.04;
        return ((latitude - terrainLat ) * (terrainZWidth)) / terrainLatExtent;
    }

    void SimulationModel::updateModel()
    {

        //get delta time
        currentTime = device->getTimer()->getTime();
        deltaTime = (currentTime - previousTime)/1000.f; //Time in seconds
        previousTime = currentTime;

        //move, according to heading and speed
        xPos = xPos + sin(heading*core::DEGTORAD)*speed*deltaTime;
        zPos = zPos + cos(heading*core::DEGTORAD)*speed*deltaTime;

        //Set position & speed by calling own ship methods
        ownShip.setPosition(core::vector3df(xPos,yPos,zPos));
        ownShip.setRotation(core::vector3df(0, heading, 0)); //Global vectors

        camera.updateCamera();

        //send data to gui
        guiMain->updateGuiData(heading, speed); //Set GUI heading in degrees and speed (in m/s)
    }

