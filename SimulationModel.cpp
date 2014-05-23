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

        //set internal scenario time to start as zero
        scenarioTime = 0;

        //set scenario to load (will be read in from user) //fixme hardcoded
        std::string scenarioName = "b) Buoyage by night";
        std::string worldName = "SimpleEstuary";

        //construct path to scenario
        std::string scenarioPath = "Scenarios/"; //Fixme: Think about proper path handling?
        scenarioPath.append(scenarioName);

        //construct path to world model
        std::string worldPath = "World/";
        worldPath.append(worldName);

        //Load own ship model. (should also initialise speed)
        //speed = 0.0f;
        ownShip.load(scenarioPath, smgr, this);

        //make a camera, setting parent and offset
        core::vector3df camOffset = ownShip.getCameraOffset(); //Get the initial camera offset from the own ship model
        camera.load(smgr,ownShip.getSceneNode(),camOffset);

        //Add terrain
        terrain.load(smgr, driver);

        //Load other ships
        otherShips.load(scenarioPath,smgr,this);

        //Load buoys
        buoys.load(worldPath, smgr, this);

        //add water
        Water water (smgr, driver);

        //make ambient light
        smgr->setAmbientLight(video::SColor(255,64,64,64));
        //add a directional light
        //scene::ILightSceneNode* light = smgr->addLightSceneNode( ownShip.getSceneNode(), core::vector3df(0,400,-200), video::SColorf(0.3f,0.3f,0.3f), 100000.0f, 1 );
        //Probably set this as an ELT_DIRECTIONAL light, to set an 'infinitely' far light with constant direction.

        //make a radar screen, setting parent and offset from camera (could also be from own ship)
        core::vector3df radarOffset = core::vector3df(0.4,-0.25,0.75); //FIXME: hardcoded offset - should be read from the own ship model
        radarScreen.load(driver,smgr,camera.getSceneNode(),radarOffset);

        //sky box/dome
        Sky sky (smgr, driver);

        //make fog
        driver->setFog(smgr->getAmbientLight().toSColor(), video::EFT_FOG_LINEAR, 250, 5000, .003f, true, true);

        //store time
        previousTime = device->getTimer()->getTime();

    } //end of SimulationModel constructor

    irr::f32 SimulationModel::longToX(irr::f32 longitude) const
    {
        return terrain.longToX(longitude); //Cascade to terrain
    }

    irr::f32 SimulationModel::latToZ(irr::f32 latitude) const
    {
        return terrain.latToZ(latitude); //Cascade to terrain
    }

    void SimulationModel::setSpeed(f32 spd)
    {
         ownShip.setSpeed(spd);
    }

    irr::f32 SimulationModel::getSpeed() const
    {
        return(ownShip.getSpeed());
    }

    void SimulationModel::setHeading(f32 hdg)
    {
         ownShip.setHeading(hdg);
    }

    irr::f32 SimulationModel::getHeading() const
    {
        return(ownShip.getHeading());
    }


    void SimulationModel::update()
    {

        //get delta time
        currentTime = device->getTimer()->getTime();
        deltaTime = (currentTime - previousTime)/1000.f;
        previousTime = currentTime;

        //add this to the scenario time
        scenarioTime += deltaTime;

        //update other ship positions etc
        otherShips.update(deltaTime,scenarioTime,camera.getPosition()); //Update other ship motion (based on leg information), and light visibility.

        //update buoys (for lights)
        buoys.update(deltaTime,scenarioTime,camera.getPosition());

        //update own ship
        ownShip.update(deltaTime);

        //update the camera position
        camera.update();

        //set radar screen position, and update it with a radar image from the radar calculation
        video::IImage * radarImage = driver->createImage (video::ECF_A1R5G5B5, core::dimension2d<u32>(128, 128)); //Create image for radar calculation to work on
        radarCalculation.update(radarImage,terrain,ownShip);
        radarScreen.update(radarImage);
        radarImage->drop(); //We created this with 'create', so drop it when we're finished

        //send data to gui
        guiMain->updateGuiData(ownShip.getHeading(), ownShip.getSpeed()); //Set GUI heading in degrees and speed (in m/s)
    }

