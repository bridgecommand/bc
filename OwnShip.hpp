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

#ifndef __OWNSHIP_HPP_INCLUDED__
#define __OWNSHIP_HPP_INCLUDED__

#include "irrlicht.h"

#include <vector>

#include "Ship.hpp"

//Forward declarations
class SimulationModel;
class OwnShipData;
class Terrain;

class OwnShip : public Ship
{
    public:

        void load(OwnShipData ownShipData, irr::scene::ISceneManager* smgr, SimulationModel* model, Terrain* terrain, irr::IrrlichtDevice* dev);
        void update(irr::f32 deltaTime, irr::f32 scenarioTime, irr::f32 tideHeight, irr::f32 weather);
        std::vector<irr::core::vector3df> getCameraViews() const;
        std::string getRadarConfigFile() const;
        irr::f32 getDepth();
        irr::f32 getAngleCorrection() const;
        bool hasGPS() const;
        bool hasDepthSounder() const;
        irr::f32 getMaxSounderDepth() const;
        void setRudder(irr::f32); //Set the rudder (-ve is port, +ve is stbd)
        void setPortEngine(irr::f32); //Set the engine, (-ve astern, +ve ahead)
        void setStbdEngine(irr::f32); //Set the engine, (-ve astern, +ve ahead)
        void setRateOfTurn(irr::f32 rateOfTurn); //Sets the rate of turn (used when controlled as secondary)
        irr::f32 getRateOfTurn() const;
        irr::f32 getPortEngine() const; //-1 to 1
        irr::f32 getStbdEngine() const; //-1 to 1
        irr::f32 getPortEngineRPM() const;
        irr::f32 getStbdEngineRPM() const;
        irr::f32 getRudder() const; //-30 to 30
        irr::f32 getPitch() const;
        irr::f32 getRoll() const;
        bool isSingleEngine() const;

    protected:
    private:
        irr::IrrlichtDevice* device;
        std::vector<irr::core::vector3df> views; //The offset of the camera origin from the own ship origin
        std::string radarConfigFile;
        Terrain* terrain;
        SimulationModel* model;
        irr::f32 rollPeriod; //Roll period (s)
        irr::f32 rollAngle; //Roll Angle (deg)
        irr::f32 pitchPeriod; //Roll period (s)
        irr::f32 pitchAngle; //Roll Angle (deg)
        irr::f32 buffetPeriod; //Yaw period (s)
        irr::f32 buffet; //How much ship is buffeted by waves (undefined units)
        irr::f32 pitch; //(deg)
        irr::f32 roll; //(deg)
        irr::f32 portEngine; //-1 to + 1
        irr::f32 stbdEngine; //-1 to + 1
        bool singleEngine;
        irr::f32 rudder; //-30 to + 30
        //Dynamics parameters
        irr::f32 shipMass;
        irr::f32 inertia;
        irr::f32 maxEngineRevs;
        irr::f32 dynamicsSpeedA;
        irr::f32 dynamicsSpeedB;
        irr::f32 dynamicsTurnDragA;
        irr::f32 dynamicsTurnDragB;
        irr::f32 rudderA;
        irr::f32 rudderB;
        irr::f32 rudderBAstern;
        irr::f32 maxForce;
        irr::f32 propellorSpacing;
        irr::f32 asternEfficiency;
        irr::f32 propWalkAhead;
        irr::f32 propWalkAstern;
        //Dynamics variables
        irr::f32 portThrust;
        irr::f32 stbdThrust;
        irr::f32 drag;
        irr::f32 acceleration;
        irr::f32 maxSpeedAhead;
        irr::f32 maxSpeedAstern;
        irr::f32 requiredEngineProportion(irr::f32 speed);
        irr::f32 rateOfTurn;
        irr::f32 dragTorque;
        irr::f32 rudderTorque;
        irr::f32 engineTorque;
        irr::f32 propWalkTorque;

        irr::f32 waveHeightFiltered; //1st order transfer filtered response to waves
        //General settings
        bool gps;
        bool depthSounder;
        irr::f32 maxSounderDepth;

};

#endif // __OWNSHIP_HPP_INCLUDED__
