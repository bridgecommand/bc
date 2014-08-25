#include "SimulationModel.hpp"
#include "Constants.hpp"
#include "Utilities.hpp"
#include <ctime>

using namespace irr;

SimulationModel::SimulationModel(IrrlichtDevice* dev, scene::ISceneManager* scene, GUIMain* gui, std::string scenarioName) //constructor, including own ship model
    {
        //get reference to scene manager
        device = dev;
        smgr = scene;
        driver = scene->getVideoDriver();
        guiMain = gui;

        //set scenario to load (will be read in from user) //fixme hardcoded
        //std::string scenarioName = "a) Buoyage";
        //std::string worldName = "SimpleEstuary";

        //construct path to scenario
        std::string scenarioPath = "Scenarios/"; //Fixme: Think about proper path handling?
        scenarioPath.append(scenarioName);

        //Read world file name from scenario:
        std::string environmentIniFilename = scenarioPath;
        environmentIniFilename.append("/environment.ini");
        std::string worldName = IniFile::iniFileToString(environmentIniFilename,"Setting");
        irr::f32 startTime = IniFile::iniFileTof32(environmentIniFilename,"StartTime");
        irr::u32 startDay=IniFile::iniFileTou32(environmentIniFilename,"StartDay");
        irr::u32 startMonth=IniFile::iniFileTou32(environmentIniFilename,"StartMonth");
        irr::u32 startYear=IniFile::iniFileTou32(environmentIniFilename,"StartYear");

        //Fixme: Think about time zone handling
        //Fixme: Note that if the time_t isn't long enough, 2038 problem exists
        scenarioOffsetTime = Utilities::dmyToTimestamp(startDay,startMonth,startYear);//Time in seconds to start of scenario day (unix timestamp for 0000h on day scenario starts)

        //set internal scenario time to start
        scenarioTime = startTime * SECONDS_IN_HOUR;

        //Start paused initially
        accelerator = 0.0;

        //Set initial tide height to zero
        tideHeight = 0;

        if (worldName == "") {
            //Could not load world name from scenario, so end here
            //ToDo: Tell user problem
            exit(EXIT_FAILURE);
        }

        //construct path to world model
        std::string worldPath = "World/";
        worldPath.append(worldName);

        //Add terrain: Needs to happen first, so the terrain parameters are available
        terrain.load(worldPath, smgr);

        //add water
        water.load(smgr);

        //sky box/dome
        Sky sky (smgr);

        //make ambient light
        light.load(smgr);

        //Load own ship model.
        ownShip.load(scenarioPath, smgr, this, &terrain);

        //make a camera, setting parent and offset
        std::vector<core::vector3df> views = ownShip.getCameraViews(); //Get the initial camera offset from the own ship model
        camera.load(smgr,ownShip.getSceneNode(),views);

        //Load other ships
        otherShips.load(scenarioPath,scenarioTime,smgr,this);

        //Load buoys
        buoys.load(worldPath, smgr, this);

        //Load land objects
        landObjects.load(worldPath, smgr, this, terrain);

        //Load land lights
        landLights.load(worldPath, smgr, this, terrain);

        //Load tidal information
        tide.load(worldPath);

        //make a radar screen, setting parent and offset from camera (could also be from own ship)
        core::vector3df radarOffset = core::vector3df(0.45,-0.28,0.75); //FIXME: hardcoded offset - should be read from the own ship model
        radarScreen.load(smgr,camera.getSceneNode(),radarOffset);

        //initialise offset
        offsetPosition = core::vector3d<u64>(0,0,0);

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

    void SimulationModel::setAccelerator(irr::f32 accelerator)
    {
        this->accelerator = accelerator;
    }

    void SimulationModel::lookLeft()
    {
        camera.lookLeft();
    }

    void SimulationModel::lookRight()
    {
        camera.lookRight();
    }

    void SimulationModel::lookAhead()
    {
        camera.lookAhead();
    }

    void SimulationModel::lookAstern()
    {
        camera.lookAstern();
    }

    void SimulationModel::lookPort()
    {
        camera.lookPort();
    }

    void SimulationModel::lookStbd()
    {
        camera.lookStbd();
    }

    void SimulationModel::changeView()
    {
        camera.changeView();
    }

    void SimulationModel::update()
    {

        //get delta time
        currentTime = device->getTimer()->getTime();
        deltaTime = accelerator*(currentTime - previousTime)/1000.f;
        //deltaTime = (currentTime - previousTime)/1000.f;
        previousTime = currentTime;

        //add this to the scenario time
        scenarioTime += deltaTime;
        absoluteTime = Utilities::round(scenarioTime) + scenarioOffsetTime;

        //Update tide height here.
        tide.update(scenarioTime);
        tideHeight = tide.getTideHeight();

        //update ambient lighting
        light.update(scenarioTime);
        driver->setFog(light.getLightSColor(), video::EFT_FOG_LINEAR, 250, 5000, .003f, true, true);
        irr::u32 lightLevel = light.getLightLevel();

        //update other ship positions etc
        otherShips.update(deltaTime,scenarioTime,tideHeight,camera.getPosition(),lightLevel); //Update other ship motion (based on leg information), and light visibility.

        //update buoys (for lights)
        buoys.update(deltaTime,scenarioTime,tideHeight,camera.getPosition(),lightLevel);

        //update own ship
        ownShip.update(deltaTime, tideHeight);

        //update water position
        water.update(tideHeight,camera.getPosition());

        //Normalise positions if required (More than 2000 metres from origin)
        if(ownShip.getPosition().getLength() > 2000) {
            core::vector3df ownShipPos = ownShip.getPosition();
            irr::s32 deltaX = -1*(s32)ownShipPos.X;
            irr::s32 deltaZ = -1*(s32)ownShipPos.Z;
            //Round to nearest 1000 metres - water tile width, to avoid jumps
            deltaX = 1000.0*Utilities::round(deltaX/1000.0);
            deltaZ = 1000.0*Utilities::round(deltaZ/1000.0);

            //Move all objects
            ownShip.moveNode(deltaX,0,deltaZ);
            terrain.moveNode(deltaX,0,deltaZ);
            otherShips.moveNode(deltaX,0,deltaZ);
            buoys.moveNode(deltaX,0,deltaZ);
            landObjects.moveNode(deltaX,0,deltaZ);
            landLights.moveNode(deltaX,0,deltaZ);

            //Change stored offset
            offsetPosition.X -= deltaX;
            offsetPosition.Z -= deltaZ;
            std::cout << "Normalised, offset X: " << offsetPosition.X << " Z: " << offsetPosition.Z <<std::endl;
        }

        //update the camera position
        camera.update();

        //set radar screen position, and update it with a radar image from the radar calculation
        video::IImage * radarImage = driver->createImage (video::ECF_R8G8B8, core::dimension2d<u32>(256, 256)); //Create image for radar calculation to work on
        radarCalculation.update(radarImage,terrain,ownShip,buoys,otherShips,tideHeight,deltaTime);
        radarScreen.update(radarImage);
        radarImage->drop(); //We created this with 'create', so drop it when we're finished

        //send data to gui
        guiMain->updateGuiData(ownShip.getHeading(), ownShip.getSpeed(), ownShip.getDepth(),Utilities::timestampToString(absoluteTime),accelerator==0); //Set GUI heading in degrees and speed (in m/s)
    }

