#include "irrlicht.h"

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

        //load example buoy object and put in the buoys array
        buoys.push_back(Buoy ("Models/Buoy/Safe/buoy.x",core::vector3df(894.34f,0.0f,619.317f),smgr));

        //add some water (from demo 8)
        scene::IAnimatedMesh* waterMesh = smgr->addHillPlaneMesh( "myHill",
                       core::dimension2d<f32>(50,50),
                       core::dimension2d<u32>(100,100), 0, 0,
                       core::dimension2d<f32>(0,0),
                       core::dimension2d<f32>(10,10));

        scene::ISceneNode* waterNode = 0;
        waterNode = smgr->addWaterSurfaceSceneNode(waterMesh->getMesh(0), 0.5f, 300.0f, 10.0f);
        waterNode->setPosition(core::vector3df(0,-2*0.5f,0));

        waterNode->setMaterialTexture(0, driver->getTexture("media/water.bmp"));
        waterNode->setMaterialFlag(video::EMF_LIGHTING, false);
        waterNode->setMaterialFlag(video::EMF_FOG_ENABLE, true);

        //sky box/dome
        // create skydome
	    driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
        scene::ISceneNode* skydome=smgr->addSkyDomeSceneNode(driver->getTexture("media/sky.bmp"),16,8,0.95f,2.0f);
	    driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);
	    skydome->setMaterialFlag(video::EMF_FOG_ENABLE, true);

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

