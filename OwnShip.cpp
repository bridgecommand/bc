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
        views.push_back(irr::core::vector3df(scaleFactor*camOffsetX,scaleFactor*(camOffsetY+0*yCorrection),scaleFactor*camOffsetZ));
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
	irr::scene::IAnimatedMesh* shipMesh = smgr->getMesh(ownShipFullPath.c_str());

    //Set mesh vertical correction (world units)
    heightCorrection = yCorrection*scaleFactor;

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


    ship = smgr->addAnimatedMeshSceneNode(shipMesh,0,-1,irr::core::vector3df(0,0,0));
    ship->setScale(irr::core::vector3df(scaleFactor,scaleFactor,scaleFactor));
    ship->setPosition(irr::core::vector3df(0,heightCorrection,0));

    ship->setMaterialFlag(irr::video::EMF_FOG_ENABLE, true);
    ship->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, true); //Normalise normals on scaled meshes, for correct lighting

    //Set lighting to use diffuse and ambient, so lighting of untextured models works
	if(ship->getMaterialCount()>0) {
        for(irr::u32 mat=0;mat<ship->getMaterialCount();mat++) {
            ship->getMaterial(mat).MaterialType = irr::video::EMT_TRANSPARENT_VERTEX_ALPHA;
            //ship->getMaterial(mat).setFlag(video::EMF_ZWRITE_ENABLE,true);
            ship->getMaterial(mat).ColorMaterial = irr::video::ECM_DIFFUSE_AND_AMBIENT;
        }
    }

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

irr::f32 OwnShip::getPortEngine() const
{
    return portEngine;
}

irr::f32 OwnShip::getStbdEngine() const
{
    return stbdEngine;
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
		irr::f32 acceleration = (portThrust+stbdThrust-drag)/shipMass;
        spd += acceleration*deltaTime;

        //Lateral dynamics
        irr::f32 lateralThrust = bowThruster*bowThrusterMaxForce + sternThruster*sternThrusterMaxForce;
// comment perhaps dynamicsLateralDragA and B should be proportional to the lateral submerged area so roughly dynamicsDragA * (L / B)
        irr::f32 lateralDrag;
        if (lateralSpd<0) { //Compensate for loss of sign when squaring
            lateralDrag = -1*dynamicsLateralDragA*lateralSpd*lateralSpd + dynamicsLateralDragB*lateralSpd;
		} else {
			lateralDrag =    dynamicsLateralDragA*lateralSpd*lateralSpd + dynamicsLateralDragB*lateralSpd;
		}
		irr::f32 lateralAcceleration = (lateralThrust-lateralDrag)/shipMass;
		//std::cout << "Lateral acceleration (m/s2): " << lateralAcceleration << std::endl;
		lateralSpd += lateralAcceleration*deltaTime;

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
        rateOfTurn += (rudderTorque + engineTorque + propWalkTorque + thrusterTorque - dragTorque)*deltaTime/inertia; //Rad/s

        //slow down if aground
        if (getDepth()<0) { //Todo: Have a separate groundingDepth(), that checks min depth at centre, and 3/4 ahead and astern of centre
            if (spd>0) {
                spd = fmin(0.1,spd); //currently hardcoded for 0.1 m/s, ~0.2kts
            }
            if (spd<0) {
                spd = fmax(-0.1,spd);
            }

            if (rateOfTurn>0) {
                rateOfTurn = fmin(0.01,rateOfTurn);//Rate of turn in rad/s, currently hardcoded for 0.01 rad/s
            }
            if (rateOfTurn<0) {
                rateOfTurn = fmax(-0.01,rateOfTurn);//Rate of turn in rad/s
            }

            if (lateralSpd>0) {
                lateralSpd = fmin(0.1,lateralSpd);
            }
            if (lateralSpd<0) {
                lateralSpd = fmax(-0.1,lateralSpd);
            }
        }

        //Apply turn
        hdg += rateOfTurn*deltaTime*irr::core::RADTODEG; //Deg

        //Apply buffeting from waves
        irr::f32 buffetAngle= buffet*weather*sin(scenarioTime*2*PI/buffetPeriod)*deltaTime;//Deg
        buffetAngle = buffetAngle * (irr::f32)std::rand()/RAND_MAX;

        hdg += buffetAngle;


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
    if(hdg>=360) {hdg-=360;}
    if(hdg<0) {hdg+=360;}

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
            if (cog >= 360) {cog -=360;}
            if (cog < 0) {cog +=360;}
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

irr::f32 OwnShip::getDepth()
{
    return -1*terrain->getHeight(xPos,zPos)+getPosition().Y;
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

std::string OwnShip::getRadarConfigFile() const
{
    return radarConfigFile;
}
