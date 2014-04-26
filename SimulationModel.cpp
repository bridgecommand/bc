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

        //store time
        previousTime = device->getTimer()->getTime();

        //Load a ship model
        heading = 0;
        xPos = 0.0f;//864.34f;
        yPos = 0.0f;
        zPos = 0.0f;//619.317f;
        speed = 0.0f;
        //Load own ship model. This also sets heading and position
        ownShip.loadModel("Scenarios/a) Buoyage/ownship.ini",xPos, yPos, zPos, heading, smgr, this); //Fixme: Hardcoding of scenario
        //initialise variables - Fixme: Start with a hardcoded initial position

        //make a camera, setting parent and offset
        core::vector3df camOffset = ownShip.getCameraOffset(); //Get the initial camera offset from the own ship model
        camera.loadCamera(smgr,ownShip.getSceneNode(),camOffset);

        //Add terrain
        terrain.loadTerrain(smgr, driver);

        //Load other ships
        otherShips.loadOtherShips("Scenarios/a) Buoyage/othership.ini",smgr,this); //Fixme: Hardcoding of scenario

        //Load buoys
        buoys.loadBuoys("World/SimpleEstuary/buoy.ini", smgr, this); //Fixme: Hardcoding of world model

        //add water
        Water water (smgr, driver);

        //make a radar screen, setting parent and offset
        core::vector3df radarOffset = core::vector3df(0+ 0.4,1.2 -0.25,0.6 +0.75); //FIXME: hardcoded offset - should be read from the own ship model
        radarScreen.loadRadarScreen(driver,smgr,ownShip.getSceneNode(),radarOffset);

        //sky box/dome
        Sky sky (smgr, driver);

        //make fog
        driver->setFog(video::SColor(128,128,128,128), video::EFT_FOG_LINEAR, 250, 5000, .003f, true, true);

    } //end of SimulationModel constructor

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


    void SimulationModel::updateModel()
    {

        //get delta time
        currentTime = device->getTimer()->getTime();
        deltaTime = 10.0*(currentTime - previousTime)/1000.f; //Time in seconds //FIXME: Bodged accelerator
        previousTime = currentTime;

        //move, according to heading and speed
        xPos = xPos + sin(heading*core::DEGTORAD)*speed*deltaTime;
        zPos = zPos + cos(heading*core::DEGTORAD)*speed*deltaTime;

        //Set position & speed by calling own ship methods
        ownShip.setPosition(core::vector3df(xPos,yPos,zPos));
        ownShip.setRotation(core::vector3df(0, heading, 0)); //Global vectors

        //set radar screen position, and update it with a (currently dummy) image
        //Start temporary bit to create a dummy image. We should have a RadarCalculation model which has an update method that returns an IImage that we can use here
        video::IImage * radarImage = driver->createImage (video::ECF_A1R5G5B5, core::dimension2d<u32>(64, 64));
        radarImage->fill(video::SColor(255, 0, 0, 255));
        std::cout << terrain.getHeight(xPos,zPos) << std::endl;
        for(int i = 0;i<64;i++) {
            for(int j=0;j<64;j++) {
                f32 localPosX = (i-32)*20.0 + xPos;
                f32 localPosZ = (32-j)*20.0 + zPos;
                u32 pixelColour = terrain.getHeight(localPosX,localPosZ);
                if (pixelColour > 255) {pixelColour = 255;}
                radarImage->setPixel(i,j,video::SColor(255,pixelColour,pixelColour,pixelColour));
            }
        }
        radarImage->setPixel(32,32,video::SColor(255,255,255,0));
        //end of dummy image code
        radarScreen.updateRadarScreen(radarImage);
        radarImage->drop(); //We created this with 'create', so drop it when we're finished

        //update the camera position
        camera.updateCamera();

        //send data to gui
        guiMain->updateGuiData(heading, speed); //Set GUI heading in degrees and speed (in m/s)
    }

