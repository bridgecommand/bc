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
        scene::IMesh* shipMesh = smgr->getMesh("Models/Ownship/Atlantic85/Hull.3ds");
        ownShipNode = smgr->addMeshSceneNode(shipMesh);
        if (ownShipNode) {ownShipNode->setMaterialFlag(video::EMF_LIGHTING, false);}

        //make a camera
        camera = smgr->addCameraSceneNode(0, core::vector3df(0,0,0), core::vector3df(0,0,1));

        //Add terrain
        scene::ITerrainSceneNode* terrain = smgr->addTerrainSceneNode(
                       "World/SimpleEstuary/height.bmp",
                       0,					// parent node
                       -1,					// node id
		               core::vector3df(0.f, -44.07f, 0.f),		// position
		               core::vector3df(0.f, 180.f, 0.f),		// rotation (NOTE 180 deg rotation)
		               core::vector3df(6.97705f, 0.56498f, 8.6871f),	// scale
		               video::SColor ( 255, 255, 255, 255 ),	// vertexColor
		               5,					// maxLOD
		               scene::ETPS_17,		// patchSize
		               4					// smoothFactoespr
                       );
        terrain->setMaterialFlag(video::EMF_LIGHTING, false);
        terrain->setMaterialTexture(0, driver->getTexture("World/SimpleEstuary/texture.bmp"));

        //load buoys (In due course, move these into their own object, which can go into a vector, including scene node pointer and light characteristics etc)
        scene::IMesh* buoyMesh = smgr->getMesh("Models/Buoy/Safe/buoy.x");
        scene::IMeshSceneNode* buoy = smgr->addMeshSceneNode(buoyMesh,0,-1,core::vector3df(894.34f,0.0f,619.317f)); //Hardcoded position
        buoy->setMaterialFlag(video::EMF_LIGHTING, false);

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

        //sky box/dome
        // create skydome
	    driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
        scene::ISceneNode* skydome=smgr->addSkyDomeSceneNode(driver->getTexture("media/sky.bmp"),16,8,0.95f,2.0f);
	    driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);

        //make fog
        driver->setFog(video::SColor(128,128,128,128), video::EFT_FOG_LINEAR, 250, 1000, .003f, true, true);
        waterNode->setMaterialFlag(video::EMF_FOG_ENABLE, true);
        terrain->setMaterialFlag(video::EMF_FOG_ENABLE, true);
        skydome->setMaterialFlag(video::EMF_FOG_ENABLE, true);

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

        //Set position & speed by calling our private methods
        setPosition(xPos,yPos,zPos);
        setRotation(0, heading, 0); //Global vectors

         //link camera rotation to shipNode
        // get transformation matrix of node
        core::matrix4 m;
        m.setRotationDegrees(ownShipNode->getRotation());

        // transform forward vector of camera
        core::vector3df frv(0.0f, 0.0f, 1.0f);
        m.transformVect(frv);

        // transform upvector of camera
        core::vector3df upv(0.0f, 1.0f, 0.0f);
        m.transformVect(upv);

        // transform camera offset (thanks to Zeuss for finding it was missing)
        core::vector3df offset(0.0f,0.9f,0.6f);
        m.transformVect(offset);

        //move camera and angle
        camera->setPosition(ownShipNode->getPosition() + offset); //position camera behind the ship
        camera->setUpVector(upv); //set up vector of camera
        camera->setTarget(ownShipNode->getPosition() + offset + frv); //set target of camera (look at point)
        camera->updateAbsolutePosition();

        //send data to gui
        guiMain->updateGuiData(heading, speed); //Set GUI heading in degrees and speed (in m/s)
    }


    void SimulationModel::setPosition(f32 x, f32 y, f32 z)
    {
         ownShipNode->setPosition(core::vector3df(x,y,z));
    }

    void SimulationModel::setRotation(f32 rx, f32 ry, f32 rz)
    {
         ownShipNode->setRotation(core::vector3df(rx,ry,rz));
    }
