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

//Extends from the general 'Ship' class
#include "OwnShip.hpp"

#include "Constants.hpp"
#include "SimulationModel.hpp"
#include "ScenarioDataStructure.hpp"
#include "Terrain.hpp"
#include "IniFile.hpp"
#include "Angles.hpp"
#include "Utilities.hpp"

#include <cstdlib> //For rand()

//using namespace irr;

void OwnShip::load(OwnShipData ownShipData, irr::scene::ISceneManager* smgr, SimulationModel* model, Terrain* terrain, irr::IrrlichtDevice* dev)
{
    //Store reference to terrain
    this->terrain = terrain;

    //Store reference to model
    this->model = model;

    //reference to device (for logging etc)
    device=dev;

    //Load from ownShip.ini file
    std::string ownShipName = ownShipData.ownShipName;
    //Get initial position and heading, and set these
    spd = ownShipData.initialSpeed*KTS_TO_MPS;
    lateralSpd = 0;
    xPos = model->longToX(ownShipData.initialLong);
    yPos = 0;
    zPos = model->latToZ(ownShipData.initialLat);
    hdg = ownShipData.initialBearing;


    basePath = "Models/Ownship/" + ownShipName + "/";
    std::string userFolder = Utilities::getUserDir();
    //Read model from user dir if it exists there.
    if (Utilities::pathExists(userFolder + basePath)) {
        basePath = userFolder + basePath;
    }

    //Load from boat.ini file if it exists
    std::string shipIniFilename = basePath + "boat.ini";

    //Construct the radar config file name, to be used later by the radar
    radarConfigFile = basePath + "radar.ini";

    //get the model file
    std::string ownShipFileName = IniFile::iniFileToString(shipIniFilename,"FileName");
    std::string ownShipFullPath = basePath + ownShipFileName;

    //Load dynamics settings
    shipMass = IniFile::iniFileTof32(shipIniFilename,"Mass");
    inertia = IniFile::iniFileTof32(shipIniFilename,"Inertia");
    maxEngineRevs = IniFile::iniFileTof32(shipIniFilename,"MaxRevs");
    dynamicsSpeedA = IniFile::iniFileTof32(shipIniFilename,"DynamicsSpeedA");
    dynamicsSpeedB = IniFile::iniFileTof32(shipIniFilename,"DynamicsSpeedB");
    dynamicsTurnDragA = IniFile::iniFileTof32(shipIniFilename,"DynamicsTurnDragA");
    dynamicsTurnDragB = IniFile::iniFileTof32(shipIniFilename,"DynamicsTurnDragB");
    rudderA = IniFile::iniFileTof32(shipIniFilename,"RudderA");
    rudderB = IniFile::iniFileTof32(shipIniFilename,"RudderB");
    rudderBAstern = IniFile::iniFileTof32(shipIniFilename,"RudderBAstern");
    maxForce = IniFile::iniFileTof32(shipIniFilename,"Max_propulsion_force");
    propellorSpacing = IniFile::iniFileTof32(shipIniFilename,"PropSpace");
    asternEfficiency = IniFile::iniFileTof32(shipIniFilename,"AsternEfficiency");// (Optional, default 1)
    propWalkAhead = IniFile::iniFileTof32(shipIniFilename,"PropWalkAhead");// (Optional, default 0)
    propWalkAstern = IniFile::iniFileTof32(shipIniFilename,"PropWalkAstern");// (Optional, default 0)
    //Pitch and roll parameters: FIXME for hardcoding, and in future should be linked to the water's movements

    // DEE todo parametarise this to a function of the GM and hence GZ and second moment of Inertia about a longitudinal axis passing through the metacentre, or at least a good approximation of it.  Future modelling could also try to model parametric rolling.
    rollPeriod = IniFile::iniFileTof32(shipIniFilename,"RollPeriod"); // Softcoded roll period Tr a function of the ships condition indpendant of Te, the wave encounter period
    if (rollPeriod == 0) {
      rollPeriod=8; // default to a roll periof of 8 seconds if unspecified
    }
// DEE ^^^^^


// DEE vvvvv
    RudderAngularVelocity = IniFile::iniFileTof32(shipIniFilename,"RudderAngularVelocity"); // Softcoded angular speed of the steering gear

    if (RudderAngularVelocity == 0) {
       RudderAngularVelocity=30; // default to an almost instentaeous rudder
    }

// DEE ^^^^^



    rollAngle = 2*IniFile::iniFileTof32(shipIniFilename,"Swell"); //Roll Angle (deg @weather=1)

// DEE vvvvv ammeneded to reflect larger ships, this should be parametarised in a similar manner to rollPeriod and taken into account in the future when calculating parametric rolling conditions
//    pitchPeriod = 6; //Pitch period (s)
//    pitchPeriod = 12; //Pitch period (s)
// DEE ^^^^^

    pitchPeriod = IniFile::iniFileTof32(shipIniFilename,"PitchPeriod"); // Softcoded roll period Tr a function of the ships condition indpendant of Te, the wave encounter period
    if (pitchPeriod == 0) {
      pitchPeriod=12; // default to a roll periof of 12 seconds if unspecified
    }

    pitchAngle = 0.5*IniFile::iniFileTof32(shipIniFilename,"Swell"); //Max pitch Angle (deg @weather=1)
    buffetPeriod = 8; //Yaw period (s)
    buffet = IniFile::iniFileTof32(shipIniFilename,"Buffet");

    depthSounder = (IniFile::iniFileTou32(shipIniFilename,"HasDepthSounder")==1);
    maxSounderDepth = IniFile::iniFileTof32(shipIniFilename,"MaxDepth");
    gps = (IniFile::iniFileTou32(shipIniFilename,"HasGPS")==1);
    if (maxSounderDepth < 1) {maxSounderDepth=100;} //Default

    bowThrusterPresent = (IniFile::iniFileTof32(shipIniFilename,"BowThrusterForce")>0);
    sternThrusterPresent = (IniFile::iniFileTof32(shipIniFilename,"SternThrusterForce")>0);
    turnIndicatorPresent = (IniFile::iniFileTou32(shipIniFilename,"HasRateOfTurnIndicator")==1);
    bowThrusterMaxForce = IniFile::iniFileTof32(shipIniFilename,"BowThrusterForce");
    sternThrusterMaxForce = IniFile::iniFileTof32(shipIniFilename,"SternThrusterForce");
    bowThrusterDistance = IniFile::iniFileTof32(shipIniFilename,"BowThrusterDistance");
    sternThrusterDistance = IniFile::iniFileTof32(shipIniFilename,"SternThrusterDistance");
    dynamicsLateralDragA = IniFile::iniFileTof32(shipIniFilename,"DynamicsLateralDragA");
    dynamicsLateralDragB = IniFile::iniFileTof32(shipIniFilename,"DynamicsLateralDragB");

    //Set defaults for values that shouldn't be zero
    if (asternEfficiency == 0)
        {asternEfficiency = 1;}
    if (shipMass == 0)
        {shipMass = 10000;}
    if (inertia == 0)
        {inertia = 2000;}

    //If lateral drag isn't set, use 50x main drag values
    if (dynamicsLateralDragA==0 && dynamicsLateralDragB==0) {
        dynamicsLateralDragA = dynamicsSpeedA*10;
        dynamicsLateralDragB = dynamicsSpeedB*10;
    }


    if (propellorSpacing==0) {
        singleEngine=true;
        maxForce *= 0.5; //Internally simulated with two equal engines, so halve the value
        device->getLogger()->log("Single engine");
    } else {
        singleEngine=false;
    }

    //Todo: Missing:
    //CentrifugalDriftEffect
    //PropWalkDriftEffect
    //Windage
    //WindageTurnEffect
    //Also:
    //DeviationMaximum
    //DeviationMaximumHeading
    //?Depth
    //?AngleCorrection

    //Start in engine control mode
    controlMode = MODE_ENGINE;

    //calculate max speed from dynamics parameters
    // DEE this looks like it is in knots and not metres per second
    maxSpeedAhead  = ((-1 * dynamicsSpeedB) + sqrt((dynamicsSpeedB*dynamicsSpeedB)-4*dynamicsSpeedA*-2*maxForce))/(2*dynamicsSpeedA);
	maxSpeedAstern = ((-1 * dynamicsSpeedB) + sqrt((dynamicsSpeedB*dynamicsSpeedB)-4*dynamicsSpeedA*-2*maxForce*asternEfficiency))/(2*dynamicsSpeedA);

    //Calculate engine speed required - the port and stbd engine speeds get send back to the GUI with updateGuiData.
	model->setPortEngine(requiredEngineProportion(spd)); //Set via model to ensure sound volume is set too
	model->setStbdEngine(requiredEngineProportion(spd)); //Set via model to ensure sound volume is set too
    rudder=0;
    rateOfTurn=0;

    //Scale
    irr::f32 scaleFactor = IniFile::iniFileTof32(shipIniFilename,"ScaleFactor");
    irr::f32 yCorrection = IniFile::iniFileTof32(shipIniFilename,"YCorrection");
    angleCorrection = IniFile::iniFileTof32(shipIniFilename,"AngleCorrection");

    //camera offset (in unscaled and uncorrected ship coords)
    irr::u32 numberOfViews = IniFile::iniFileTof32(shipIniFilename,"Views");
    if (numberOfViews==0) {
        std::cerr << "Own ship: View positions can't be loaded. Please check ini file " << shipIniFilename << std::endl;
        exit(EXIT_FAILURE);
    }
    for(irr::u32 i=1;i<=numberOfViews;i++) {
        irr::f32 camOffsetX = IniFile::iniFileTof32(shipIniFilename,IniFile::enumerate1("ViewX",i));
        irr::f32 camOffsetY = IniFile::iniFileTof32(shipIniFilename,IniFile::enumerate1("ViewY",i));
        irr::f32 camOffsetZ = IniFile::iniFileTof32(shipIniFilename,IniFile::enumerate1("ViewZ",i));
        views.push_back(irr::core::vector3df(scaleFactor*camOffsetX,scaleFactor*camOffsetY,scaleFactor*camOffsetZ));
    }

	screenDisplayPosition.X = IniFile::iniFileTof32(shipIniFilename, "RadarScreenX");
	screenDisplayPosition.Y = IniFile::iniFileTof32(shipIniFilename, "RadarScreenY");
	screenDisplayPosition.Z = IniFile::iniFileTof32(shipIniFilename, "RadarScreenZ");
	screenDisplaySize = IniFile::iniFileTof32(shipIniFilename, "RadarScreenSize");
	screenDisplayTilt = IniFile::iniFileTof32(shipIniFilename, "RadarScreenTilt");
	//Default position out of view if not set
	if (screenDisplayPosition.X == 0 && screenDisplayPosition.Y == 0 && screenDisplayPosition.Z == 0) {
		screenDisplayPosition.Y = 500;
	}

	if (screenDisplaySize <= 0) {
		screenDisplaySize = 1;
	}
	screenDisplayPosition.X *= scaleFactor;
	screenDisplayPosition.Y *= scaleFactor;
	screenDisplayPosition.Z *= scaleFactor;
	screenDisplaySize *= scaleFactor;

    //Load the model

    irr::scene::IAnimatedMesh* shipMesh;

    //Check if the 'model' is actualy the string "360". If so, treat it as a 360 equirectangular panoramic image with transparency.
    is360textureShip = false;
    if (Utilities::hasEnding(ownShipFullPath,"360")) {
        is360textureShip = true;
    }

    //Set mesh vertical correction (world units)
    heightCorrection = yCorrection*scaleFactor;


    if (is360textureShip) {

        //make a dummy node, to which the views will be added as children
        shipMesh = smgr->addSphereMesh("Sphere",1);
        ship = smgr->addAnimatedMeshSceneNode(shipMesh,0,IDFlag_IsPickable,irr::core::vector3df(0,0,0));

        //Add child meshes for each
        for(int i = 0; i<views.size(); i++) {

            irr::scene::IAnimatedMesh* viewMesh = smgr->addSphereMesh(irr::io::path("Sphere")+irr::io::path(i),5.0,32,32);
            smgr->getMeshManipulator()->flipSurfaces(viewMesh);

            //Angle correction
            irr::f32 panoRotationYaw = IniFile::iniFileTof32(shipIniFilename,IniFile::enumerate1("PanoRotationYaw",i+1));
            irr::f32 panoRotationPitch = IniFile::iniFileTof32(shipIniFilename,IniFile::enumerate1("PanoRotationPitch",i+1));
            irr::f32 panoRotationRoll = IniFile::iniFileTof32(shipIniFilename,IniFile::enumerate1("PanoRotationRoll",i+1));

            irr::scene::IAnimatedMeshSceneNode* viewNode = smgr->addAnimatedMeshSceneNode(viewMesh,ship,-1,views.at(i)/scaleFactor,irr::core::vector3df(panoRotationPitch,panoRotationYaw,panoRotationRoll));

            std::string panoPath = basePath + IniFile::iniFileToString(shipIniFilename,IniFile::enumerate1("Pano",i+1));
            irr::video::ITexture* texture360 = device->getVideoDriver()->getTexture(panoPath.c_str());


            if (texture360!=0) {
                viewNode->setMaterialTexture(0,texture360);
                viewNode->getMaterial(0).getTextureMatrix(0).setTextureScale(-1.0, 1.0);
            }

            viewNode->setMaterialFlag(irr::video::EMF_FOG_ENABLE, true);
            viewNode->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, true); //Normalise normals on scaled meshes, for correct lighting
            //Set lighting to use diffuse and ambient, so lighting of untextured models works
            if(viewNode->getMaterialCount()>0) {
                for(irr::u32 mat=0;mat<viewNode->getMaterialCount();mat++) {
                    viewNode->getMaterial(mat).MaterialType = irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL;
                    viewNode->getMaterial(mat).ColorMaterial = irr::video::ECM_DIFFUSE_AND_AMBIENT;
                }
            }
            if (i>0) {
                viewNode->setVisible(false); //Hide all except the first one
            }
        }
    } else {
        shipMesh = smgr->getMesh(ownShipFullPath.c_str());
        //Make mesh scene node
        if (shipMesh==0) {
            //Failed to load mesh - load with dummy and continue
            device->getLogger()->log("Failed to load own ship model:");
            device->getLogger()->log(ownShipFullPath.c_str());
            shipMesh = smgr->addSphereMesh("Dummy name");
        }

        //If any part is partially transparent, make it fully transparent (for bridge windows etc!)
        if (IniFile::iniFileTou32(shipIniFilename,"MakeTransparent")==1) {
            for(irr::u32 mb = 0; mb<shipMesh->getMeshBufferCount(); mb++) {
                if (shipMesh->getMeshBuffer(mb)->getMaterial().DiffuseColor.getAlpha() < 255) {
                    //Hide this mesh buffer by scaling to zero size
                    smgr->getMeshManipulator()->scale(shipMesh->getMeshBuffer(mb),irr::core::vector3df(0,0,0));
                }
            }
        }
        ship = smgr->addAnimatedMeshSceneNode(shipMesh,0,IDFlag_IsPickable,irr::core::vector3df(0,0,0));

        ship->setMaterialFlag(irr::video::EMF_FOG_ENABLE, true);
        ship->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, true); //Normalise normals on scaled meshes, for correct lighting
        //Set lighting to use diffuse and ambient, so lighting of untextured models works
        if(ship->getMaterialCount()>0) {
            for(irr::u32 mat=0;mat<ship->getMaterialCount();mat++) {
                ship->getMaterial(mat).MaterialType = irr::video::EMT_TRANSPARENT_VERTEX_ALPHA;
                ship->getMaterial(mat).ColorMaterial = irr::video::ECM_DIFFUSE_AND_AMBIENT;
            }
        }
    }

    ship->setScale(irr::core::vector3df(scaleFactor,scaleFactor,scaleFactor));
    ship->setPosition(irr::core::vector3df(0,heightCorrection,0));

    length = ship->getBoundingBox().getExtent().Z*scaleFactor; //Store length for basic collision calculation
    width = ship->getBoundingBox().getExtent().X*scaleFactor; //Store length for basic collision calculation

    //set initial pitch and roll
    pitch = 0;
    roll = 0;
    waveHeightFiltered = 0;

    //Initialise
    bowThruster = 0;
    sternThruster = 0;

    cog = 0;
    sog = 0;

    sternThrusterRate = 0;
    bowThrusterRate = 0;

    rudder = 0;
    wheel = 0;

    buoyCollision = false;
    otherShipCollision = false;

    //Detect sample points for terrain interaction here (think separately about how to do this for 360 models, probably with a separate collision model)
    //Add a triangle selector
    irr::scene::ITriangleSelector* selector=smgr->createTriangleSelector(ship);
    if(selector) {
        device->getLogger()->log("Created triangle selector");
        ship->setTriangleSelector(selector);
    }

    ship->updateAbsolutePosition();

    irr::core::aabbox3df boundingBox = ship->getTransformedBoundingBox();
    irr::f32 minX = boundingBox.MinEdge.X;
    irr::f32 maxX = boundingBox.MaxEdge.X;
    irr::f32 minY = boundingBox.MinEdge.Y;
    irr::f32 maxY = boundingBox.MaxEdge.Y;
    irr::f32 minZ = boundingBox.MinEdge.Z;
    irr::f32 maxZ = boundingBox.MaxEdge.Z;


    int xPoints = 10;
    int yPoints = 50;
    int zPoints = 50;

    //Grid from below looking up
    for (int i = 0; i<xPoints; i++) {
        for (int j = 0; j<zPoints; j++) {

            irr::f32 xTestPos = minX + (maxX-minX)*(irr::f32)i/(irr::f32)(xPoints-1);
            irr::f32 zTestPos = minZ + (maxZ-minZ)*(irr::f32)j/(irr::f32)(zPoints-1);

            irr::core::line3df ray; //Make a ray. This will start outside the mesh, looking in
            ray.start.X = xTestPos; ray.start.Y = minY; ray.start.Z = zTestPos;
            ray.end = ray.start;
            ray.end.Y = maxY;

            //Check the ray and add the contact point if it exists
            addContactPointFromRay(ray);
        }
    }

    //Grid from ahead/astern
    for (int i = 0; i<xPoints; i++) {
        for (int j = 0; j<yPoints; j++) {

            irr::f32 xTestPos = minX + (maxX-minX)*(irr::f32)i/(irr::f32)(xPoints-1);
            irr::f32 yTestPos = minY + (maxY-minY)*(irr::f32)j/(irr::f32)(yPoints-1);

            irr::core::line3df ray; //Make a ray. This will start outside the mesh, looking in
            ray.start.X = xTestPos; ray.start.Y = yTestPos; ray.start.Z = maxZ;
            ray.end = ray.start;
            ray.end.Z = minZ;

            //Check the ray and add the contact point if it exists
            addContactPointFromRay(ray);
            //swap ray direction and check again
            ray.start.Z = minZ;
            ray.end.Z = maxZ;
            addContactPointFromRay(ray);
        }
    }

    //Grid from side to side
    for (int i = 0; i<zPoints; i++) {
        for (int j = 0; j<yPoints; j++) {

            irr::f32 zTestPos = minZ + (maxZ-minZ)*(irr::f32)i/(irr::f32)(zPoints-1);
            irr::f32 yTestPos = minY + (maxY-minY)*(irr::f32)j/(irr::f32)(yPoints-1);

            irr::core::line3df ray; //Make a ray. This will start outside the mesh, looking in
            ray.start.X = maxX; ray.start.Y = yTestPos; ray.start.Z = zTestPos;
            ray.end = ray.start;
            ray.end.X = minX;

            //Check the ray and add the contact point if it exists
            addContactPointFromRay(ray);
            //swap ray direction and check again
            ray.start.X = minX;
            ray.end.X = maxX;
            addContactPointFromRay(ray);
        }
    }

    //We don't want to do further triangle selection with the ship, so set the selector to null
    ship->setTriangleSelector(0);
    device->getLogger()->log("Own ship points found: ");
    device->getLogger()->log(irr::core::stringw(contactPoints.size()).c_str());
}

void OwnShip::addContactPointFromRay(irr::core::line3d<irr::f32> ray)
{
    irr::core::vector3df intersection;
    irr::core::triangle3df hitTriangle;

    irr::scene::ISceneNode * selectedSceneNode =
        device->getSceneManager()->getSceneCollisionManager()->getSceneNodeAndCollisionPointFromRay(
        ray,
        intersection, // This will be the position of the collision
        hitTriangle, // This will be the triangle hit in the collision
        IDFlag_IsPickable, // (bitmask)
        0); // Check all nodes

    if(selectedSceneNode) {
        ContactPoint contactPoint;
        contactPoint.position = intersection;
        contactPoint.normal = hitTriangle.getNormal().normalize();
        contactPoint.position.Y -= heightCorrection; //Adjust for height correction

        //Find an internal node position, i.e. a point at which a ray check for internal intersection can start
        ray.start = contactPoint.position;
        ray.end = ray.start - 100*contactPoint.normal;
        //Check for the internal node
        selectedSceneNode =
            device->getSceneManager()->getSceneCollisionManager()->getSceneNodeAndCollisionPointFromRay(
            ray,
            intersection, // This will be the position of the collision
            hitTriangle, // This will be the triangle hit in the collision
            IDFlag_IsPickable, // (bitmask)
            0); // Check all nodes

        if (selectedSceneNode) {
            contactPoint.internalPosition = intersection;
            contactPoint.internalPosition.Y -= heightCorrection; //Adjust for height correction

            //Find cross product, for torque component
            irr::core::vector3df crossProduct = contactPoint.position.crossProduct(contactPoint.normal);
            contactPoint.torqueEffect = crossProduct.Y;

            //Store the contact point that we have found
            contactPoints.push_back(contactPoint); //Store

            //Debugging
            //contactDebugPoints.push_back(device->getSceneManager()->addSphereSceneNode(0.1));
            //contactDebugPoints.push_back(device->getSceneManager()->addCubeSceneNode(0.1));
        }
    }
}

void OwnShip::setRateOfTurn(irr::f32 rateOfTurn) //Sets the rate of turn (used when controlled as secondary)
{
    controlMode = MODE_AUTO; //Switch to controlled mode
    this->rateOfTurn = rateOfTurn;
}

irr::f32 OwnShip::getRateOfTurn() const
{
    return rateOfTurn;
}

void OwnShip::setRudder(irr::f32 rudder)
{
    controlMode = MODE_ENGINE; //Switch to engine and rudder mode
    //Set the rudder (-ve is port, +ve is stbd)
    this->rudder = rudder;
    if (this->rudder<-30) {
        this->rudder = -30;
    }
    if (this->rudder>30) {
        this->rudder = 30;
    }
}


// DEE vvvvvvv
void OwnShip::setWheel(irr::f32 wheel)
{
    controlMode = MODE_ENGINE; //Switch to engine and rudder mode
    //Set the wheel (-ve is port, +ve is stbd)
    this->wheel = wheel;
    if (this->wheel<-30) {
        this->wheel = -30;
    }
    if (this->wheel>30) {
        this->wheel = 30;
    }
}
// DEE ^^^^^^^



void OwnShip::setPortEngine(irr::f32 port)
{
    controlMode = MODE_ENGINE; //Switch to engine and rudder mode
    portEngine = port; //+-1
    if (portEngine>1) {
        portEngine = 1;
    }
    if (portEngine<-1) {
        portEngine = -1;
    }
}

void OwnShip::setStbdEngine(irr::f32 stbd)
{
    controlMode = MODE_ENGINE; //Switch to engine and rudder mode
    stbdEngine = stbd; //+-1
    if (stbdEngine>1) {
        stbdEngine = 1;
    }
    if (stbdEngine<-1) {
        stbdEngine = -1;
    }
}

void OwnShip::setBowThruster(irr::f32 proportion) {
    //Proportion is -1 to +1
    bowThruster = proportion;
    if (bowThruster>1) {
        bowThruster = 1;
    }
    if (bowThruster<-1) {
        bowThruster = -1;
    }
}

void OwnShip::setSternThruster(irr::f32 proportion) {
    //Proportion is -1 to +1
    sternThruster = proportion;
    if (sternThruster>1) {
        sternThruster = 1;
    }
    if (sternThruster<-1) {
        sternThruster = -1;
    }
}

void OwnShip::setBowThrusterRate(irr::f32 bowThrusterRate) {
     //Sets the rate of increase of bow thruster, used for joystick button control
    this->bowThrusterRate = bowThrusterRate;
}

void OwnShip::setSternThrusterRate(irr::f32 sternThrusterRate) {
     //Sets the rate of increase of stern thruster, used for joystick button control
    this->sternThrusterRate = sternThrusterRate;
}

irr::f32 OwnShip::getPortEngine() const
{
    return portEngine;
}

irr::f32 OwnShip::getStbdEngine() const
{
    return stbdEngine;
}

irr::f32 OwnShip::getBowThruster() const
{
    return bowThruster;
}

irr::f32 OwnShip::getSternThruster() const
{
    return sternThruster;
}

irr::f32 OwnShip::getPortEngineRPM() const
{
    return portEngine*maxEngineRevs;
}

irr::f32 OwnShip::getStbdEngineRPM() const
{
    return stbdEngine*maxEngineRevs;
}

irr::f32 OwnShip::getRudder() const
{
    return rudder;
}


// DEE vvvvvvvvvvvvvv
irr::f32 OwnShip::getWheel() const
{
    return wheel;
}
// DEE ^^^^^^^^^^^^^



irr::f32 OwnShip::getPitch() const
{
    return pitch;
}

irr::f32 OwnShip::getRoll() const
{
    return roll;
}

std::string OwnShip::getBasePath() const
{
	return basePath;
}

irr::core::vector3df OwnShip::getScreenDisplayPosition() const
{
	return screenDisplayPosition;
}

irr::f32 OwnShip::getScreenDisplaySize() const
{
	return screenDisplaySize;
}

irr::f32 OwnShip::getScreenDisplayTilt() const
{
	return screenDisplayTilt;
}

bool OwnShip::isSingleEngine() const
{
    return singleEngine;
}

bool OwnShip::isBuoyCollision() const
{
    return buoyCollision;
}

bool OwnShip::isOtherShipCollision() const
{
    return otherShipCollision;
}

irr::f32 OwnShip::requiredEngineProportion(irr::f32 speed)
{
    irr::f32 proportion = 0;
    if (speed >= 0) {
        proportion=(dynamicsSpeedA*speed*speed + dynamicsSpeedB*speed)/(2*maxForce);
    } else {
        proportion=(-1*dynamicsSpeedA*speed*speed + dynamicsSpeedB*speed)/(2*maxForce*asternEfficiency);
    }
    return proportion;
}

void OwnShip::update(irr::f32 deltaTime, irr::f32 scenarioTime, irr::f32 tideHeight, irr::f32 weather)
{

    //dynamics: hdg in degrees, spd in m/s. Internal units all SI
    if (controlMode == MODE_ENGINE) {

        //Check depth and update collision response forces and torque
        irr::f32 groundingDrag = 0;
        irr::f32 groundingLateralDrag = 0;
        irr::f32 groundingTurnDrag = 0;
        collisionDetectAndRespond(groundingDrag,groundingLateralDrag,groundingTurnDrag); //The drag values will get modified by this call

        //Update bow and stern thrusters, if being controlled by joystick buttons
        bowThruster += deltaTime * bowThrusterRate;
        if (bowThruster>1) {
            bowThruster = 1;
        }
        if (bowThruster<-1) {
            bowThruster = -1;
        }
        sternThruster += deltaTime * sternThrusterRate;
        if (sternThruster>1) {
            sternThruster = 1;
        }
        if (sternThruster<-1) {
            sternThruster = -1;
        }

        //Update spd and hdg with rudder and engine controls - assume two engines, should also work with single engine
        irr::f32 portThrust = portEngine * maxForce;
        irr::f32 stbdThrust = stbdEngine * maxForce;
        if (singleEngine) {
            stbdThrust = portThrust; //Ignore stbd slider if single engine (internally modelled as 2 engines, each with half the max force)
        }
        if (portThrust<0) {portThrust*=asternEfficiency;}
        if (stbdThrust<0) {stbdThrust*=asternEfficiency;}
        irr::f32 drag;
        if (spd<0) { //Compensate for loss of sign when squaring
            drag = -1*dynamicsSpeedA*spd*spd + dynamicsSpeedB*spd;
		} else {
			drag =    dynamicsSpeedA*spd*spd + dynamicsSpeedB*spd;
		}
		irr::f32 acceleration = (portThrust+stbdThrust-drag-groundingDrag)/shipMass;
        //Check acceleration plausibility (not more than 1g = 9.81ms/2)
        if (acceleration > 9.81) {
            acceleration = 9.81;
        } else if (acceleration < -9.81) {
            acceleration = -9.81;
        }
        spd += acceleration*deltaTime;
        //Also check speed for plausibility, limit to 50m/s
        if (spd > 50) {
            spd = 50;
        } else if (spd < -50) {
            spd = -50;
        }

        //Lateral dynamics
        irr::f32 lateralThrust = bowThruster*bowThrusterMaxForce + sternThruster*sternThrusterMaxForce;
// comment perhaps dynamicsLateralDragA and B should be proportional to the lateral submerged area so roughly dynamicsDragA * (L / B)
        irr::f32 lateralDrag;
        if (lateralSpd<0) { //Compensate for loss of sign when squaring
            lateralDrag = -1*dynamicsLateralDragA*lateralSpd*lateralSpd + dynamicsLateralDragB*lateralSpd;
		} else {
			lateralDrag =    dynamicsLateralDragA*lateralSpd*lateralSpd + dynamicsLateralDragB*lateralSpd;
		}
		irr::f32 lateralAcceleration = (lateralThrust-lateralDrag-groundingLateralDrag)/shipMass;
		//std::cout << "Lateral acceleration (m/s2): " << lateralAcceleration << std::endl;
		//Check acceleration plausibility (not more than 1g = 9.81ms/2)
        if (lateralAcceleration > 9.81) {
            lateralAcceleration = 9.81;
        } else if (lateralAcceleration < -9.81) {
            lateralAcceleration = -9.81;
        }
        lateralSpd += lateralAcceleration*deltaTime;
        //Also check speed for plausibility, limit to 50m/s
        if (lateralSpd > 50) {
            lateralSpd = 50;
        } else if (lateralSpd < -50) {
            lateralSpd = -50;
        }

        //Turn dynamics
        //Rudder
        if ((portThrust+stbdThrust) > 0) {
            rudderTorque = rudder*spd*rudderA + rudder*(portThrust+stbdThrust)*rudderB;
        } else {
            rudderTorque = rudder*spd*rudderA + rudder*(portThrust+stbdThrust)*rudderBAstern; //Reduced effect of rudder when engines engaged astern
        }
        //Engine
        engineTorque = (portThrust*propellorSpacing - stbdThrust*propellorSpacing)/2.0; //propspace is spacing between propellors, so halve to get moment arm
        //Prop walk
        irr::f32 propWalkTorquePort,propWalkTorqueStbd;
        if (portThrust > 0) {
            propWalkTorquePort=1*propWalkAhead*(portThrust/maxForce);//Had modification for 'invertspeed'
        } else {
			propWalkTorquePort=1*propWalkAstern*(portThrust/maxForce);
        }
		if (stbdThrust > 0) {
            propWalkTorqueStbd=-1*propWalkAhead*(stbdThrust/maxForce);//Had modification for 'invertspeed'
        } else {
			propWalkTorqueStbd=-1*propWalkAstern*(stbdThrust/maxForce);
        }
		propWalkTorque = propWalkTorquePort + propWalkTorqueStbd;
		//Thrusters
		irr::f32 thrusterTorque;
		thrusterTorque = bowThruster*bowThrusterMaxForce*bowThrusterDistance - sternThruster*sternThrusterMaxForce*sternThrusterDistance;
        //Turn drag
        if (rateOfTurn<0) {
            dragTorque=-1*dynamicsTurnDragA*rateOfTurn*rateOfTurn + dynamicsTurnDragB*rateOfTurn;
        } else {
            dragTorque=   dynamicsTurnDragA*rateOfTurn*rateOfTurn + dynamicsTurnDragB*rateOfTurn;
        }
        //Turn dynamics
        irr::f32 angularAcceleration = (rudderTorque + engineTorque + propWalkTorque + thrusterTorque - dragTorque - groundingTurnDrag)/inertia;
        rateOfTurn += angularAcceleration*deltaTime; //Rad/s
        //check plausibility for rate of turn, limit to ~4Pi rad/s
        if (rateOfTurn > 12) {
            rateOfTurn = 2;
        } else if (rateOfTurn < -12) {
            rateOfTurn = -12;
        }

        //apply buffeting to rate of turn - TODO: Check the integrals from this to work out if the end magnitude is right
        rateOfTurn += irr::core::DEGTORAD*buffet*weather*cos(scenarioTime*2*PI/buffetPeriod)*((irr::f32)std::rand()/RAND_MAX)*deltaTime; //Rad/s

        //Apply turn
        hdg += rateOfTurn*deltaTime*irr::core::RADTODEG; //Deg

        /*
        //Apply buffeting from waves
        irr::f32 buffetAngle= buffet*weather*sin(scenarioTime*2*PI/buffetPeriod)*deltaTime;//Deg
        buffetAngle = buffetAngle * (irr::f32)std::rand()/RAND_MAX;

        hdg += buffetAngle;
        */

        // DEE vvvvvvvvvvvvvvvvvv  Rudder Follow up code
	irr::f32 RudderPerSec=RudderAngularVelocity;
        irr::f32 MaxRudderInDtime=rudder+RudderPerSec*deltaTime;
        irr::f32 MinRudderInDtime=rudder-RudderPerSec*deltaTime;

        if (wheel>MaxRudderInDtime) {
		rudder = MaxRudderInDtime; // rudder as far to starboard as time will allow
        	} else { // wheel < MaxRudderInDtime
                if (wheel>MinRudderInDtime) {
		rudder = wheel; // rudder can turn to the wheel setting
			} else {
				rudder = MinRudderInDtime; // rudder as far to port as time will allow
				}
		}


        // DEE ^^^^^^^^^^^^^^^^^^




    } else //End of engine mode
    {
        //MODE_AUTO
        if (!positionManuallyUpdated) {
            //Apply rate of turn
            hdg += rateOfTurn*deltaTime*irr::core::RADTODEG; //Deg
        }
    }

    //Normalise heading
    while(hdg>=360) {hdg-=360;}
    while(hdg<0) {hdg+=360;}

    irr::f32 xChange = 0;
    irr::f32 zChange = 0;

    //move, according to heading and speed
    if (!positionManuallyUpdated) { //If the position has already been updated, skip (for this loop only)
        xChange = sin(hdg*irr::core::DEGTORAD)*spd*deltaTime + cos(hdg*irr::core::DEGTORAD)*lateralSpd*deltaTime;
        zChange = cos(hdg*irr::core::DEGTORAD)*spd*deltaTime - sin(hdg*irr::core::DEGTORAD)*lateralSpd*deltaTime;
        //Apply tidal stream, based on our current absolute position
        irr::core::vector2df stream = model->getTidalStream(model->getLong(),model->getLat(),model->getTimestamp());
        if (getDepth() > 0) {
            irr::f32 streamScaling = fmin(1,getDepth()); //Reduce effect as water gets shallower
            xChange += stream.X*deltaTime*streamScaling;
            zChange += stream.Y*deltaTime*streamScaling;
        }
    } else {
        positionManuallyUpdated = false;
    }

    xPos += xChange;
    zPos += zChange;


    if (deltaTime > 0) {

        //Speed over ground
        sog = pow((pow(xChange,2) + pow(zChange,2)),0.5)/deltaTime; //speed over ground in m/s

        //Course over ground
        if (xChange!=0 || zChange!=0 ) {
            cog = atan2(xChange,zChange)*irr::core::RADTODEG;
            while (cog >= 360) {cog -=360;}
            while (cog < 0) {cog +=360;}
        } else {
            cog = 0;
        }
    } //if paused, leave cog & sog unchanged.

    //std::cout << "CoG: " << cog << " SoG: " << sog << std::endl;

    //Apply up/down motion from waves, with some filtering
    irr::f32 timeConstant = 0.5;//Time constant in s; TODO: Make dependent on vessel size
    irr::f32 factor = deltaTime/(timeConstant+deltaTime);
    waveHeightFiltered = (1-factor) * waveHeightFiltered + factor*model->getWaveHeight(xPos,zPos); //TODO: Check implementation of simple filter!
    yPos = tideHeight+heightCorrection + waveHeightFiltered;

    //calculate pitch and roll - not linked to water/wave motion
    if (pitchPeriod>0)
        {pitch = weather*pitchAngle*sin(scenarioTime*2*PI/pitchPeriod);}
    if (rollPeriod>0)
        {roll = weather*rollAngle*sin(scenarioTime*2*PI/rollPeriod);}

    //Set position & angles
    ship->setPosition(irr::core::vector3df(xPos,yPos,zPos));
    ship->setRotation(Angles::irrAnglesFromYawPitchRoll(hdg+angleCorrection,pitch,roll));

}

irr::f32 OwnShip::getCOG() const
{
    return cog;
}

irr::f32 OwnShip::getSOG() const
{
    return sog; //m/s
}

irr::f32 OwnShip::getDepth() const
{
    return -1*terrain->getHeight(xPos,zPos)+getPosition().Y;
}

void OwnShip::collisionDetectAndRespond(irr::f32& reaction, irr::f32& lateralReaction, irr::f32& turnReaction)
{

    reaction = 0;
    lateralReaction = 0;
    turnReaction = 0;

    buoyCollision = false;
    otherShipCollision = false;

    if (is360textureShip) {
        //Simple method, check contact at the depth point only, to be updated to match updates in the main section

        irr::f32 localIntersection = 0; //Ready to use
        irr::f32 localDepth = getDepth(); //Simple one point method
        //Contact model (proof of principle!)
        if (localDepth < 0) {
            localIntersection = -1*localDepth; //TODO: We should actually project based on the gradient?
        }
        //Contact model (proof of principle!)
        if (localIntersection > 1) {
            localIntersection = 1; //Limit
        }

        if (localIntersection > 0) {
            //Simple 'proof of principle' values initially
            reaction += localIntersection*100*maxForce * sign(spd,0.1);
            lateralReaction += localIntersection*100*maxForce * sign(lateralSpd,0.1);
            turnReaction += localIntersection*100*maxForce * sign(rateOfTurn,0.1);
        }
        return;

    } else {
        //Normal ship model
        ship->updateAbsolutePosition();

        for (int i = 0; i<contactPoints.size(); i++) {
            irr::core::vector3df pointPosition = contactPoints.at(i).position;
            irr::core::vector3df internalPointPosition = contactPoints.at(i).internalPosition;

            //Rotate with own ship
            irr::core::matrix4 rot;
            rot.setRotationDegrees(ship->getRotation());
            rot.transformVect(pointPosition);
            rot.transformVect(internalPointPosition);

            pointPosition += ship->getAbsolutePosition();
            internalPointPosition += ship->getAbsolutePosition();

            irr::f32 localIntersection = 0; //Ready to use

            //Find depth below the contact point
            irr::f32 localDepth = -1*terrain->getHeight(pointPosition.X,pointPosition.Z)+pointPosition.Y;

            //Contact model (proof of principle!)
            if (localDepth < 0) {
                localIntersection = -1*localDepth; //TODO: We should actually project based on the gradient?
            }

            //Also check contact with pickable scenery elements here (or other ships?)
            irr::core::line3d<irr::f32> ray(internalPointPosition,pointPosition);
            irr::core::vector3df intersection;
            irr::core::triangle3df hitTriangle;
            irr::scene::ISceneNode * selectedSceneNode =
                device->getSceneManager()->getSceneCollisionManager()->getSceneNodeAndCollisionPointFromRay(
                ray,
                intersection, // This will be the position of the collision
                hitTriangle, // This will be the triangle hit in the collision
                IDFlag_IsPickable, // (bitmask)
                0); // Check all nodes

            //If this returns something, we must be in contact, so find distance between intersection and pointPosition
            if(selectedSceneNode && strcmp(selectedSceneNode->getName(),"LandObject")==0) {

                irr::f32 collisionDistance = pointPosition.getDistanceFrom(intersection);

                //If we're more collided with an object than the terrain, use this
                if (collisionDistance > localIntersection) {
                    localIntersection = collisionDistance;
                }

            }

            //Also check for buoy collision
            if (selectedSceneNode && strcmp(selectedSceneNode->getName(),"Buoy")==0) {
                buoyCollision = true;
            }

            //And for other ship collision
            if (selectedSceneNode && strcmp(selectedSceneNode->getName(),"OtherShip")==0) {
                otherShipCollision = true;
            }


            //Contact model (proof of principle!)
            if (localIntersection > 1) {
                localIntersection = 1; //Limit
            }

            if (localIntersection > 0) {
                //Simple 'proof of principle' values initially
                //reaction += localIntersection*100*maxForce * sign(spd,0.1);
                //lateralReaction += localIntersection*100*maxForce * sign(lateralSpd,0.1);
                //turnReaction += localIntersection*100*maxForce * sign(rateOfTurn,0.1);

                //Sumple 'stiffness' based response
                turnReaction += contactPoints.at(i).torqueEffect * localIntersection*100*maxForce;
                reaction += contactPoints.at(i).normal.Z*localIntersection*100*maxForce;
                lateralReaction += contactPoints.at(i).normal.X*localIntersection*100*maxForce;

                //Drag response:
                //turnReaction += 0.01*rateOfTurn*100*maxForce;
                //reaction += 0.01*spd*100*maxForce;
                //lateralReaction += 0.01*lateralSpd*100*maxForce;

            }

            //Debugging, show points:
            //contactDebugPoints.at(i*2)->setPosition(pointPosition);
            //contactDebugPoints.at(i*2 + 1)->setPosition(internalPointPosition);

        }
    }
}

irr::f32 OwnShip::getAngleCorrection() const
{
    return angleCorrection;
}

bool OwnShip::hasGPS() const
{
    return gps;
}

bool OwnShip::hasDepthSounder() const
{
    return depthSounder;
}

bool OwnShip::hasBowThruster() const
{
    return bowThrusterPresent;
}

bool OwnShip::hasSternThruster() const
{
    return sternThrusterPresent;
}

bool OwnShip::hasTurnIndicator() const
{
    return turnIndicatorPresent;
}

irr::f32 OwnShip::getMaxSounderDepth() const
{
    return maxSounderDepth;
}


std::vector<irr::core::vector3df> OwnShip::getCameraViews() const
{
    return views;
}

void OwnShip::setViewVisibility(irr::u32 view)
{
    if(is360textureShip) {
        irr::scene::ISceneNodeList childList = ship->getChildren();
        irr::scene::ISceneNodeList::ConstIterator it = childList.begin();
        int i = 0;
        while (it != childList.end()) {

            if(i==view) {
                (*it)->setVisible(true);
            } else {
                (*it)->setVisible(false);
            }

            i++;
            it++;
        }
    }
}

std::string OwnShip::getRadarConfigFile() const
{
    return radarConfigFile;
}

irr::f32 OwnShip::sign(irr::f32 inValue) const
{
    if (inValue > 0) {
        return 1.0;
    }
    if (inValue < 0) {
        return -1.0;
    }
    return 0.0;
}

irr::f32 OwnShip::sign(irr::f32 inValue, irr::f32 threshold) const
{
    if (threshold<=0) {
        return sign(inValue);
    }

    if (inValue > threshold) {
        return 1.0;
    }
    if (inValue < -1*threshold) {
        return -1.0;
    }
    return inValue/threshold;
}
