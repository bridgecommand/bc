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
#include "Terrain.hpp"

//Forward declarations
class SimulationModel;

class OwnShip : public Ship
{
    public:

        void load(const std::string& scenarioName, irr::scene::ISceneManager* smgr, SimulationModel* model, Terrain* terrain);
        void update(irr::f32 deltaTime, irr::f32 scenarioTime, irr::f32 tideHeight, irr::f32 weather);
        std::vector<irr::core::vector3df> getCameraViews() const;
        irr::f32 getDepth();
        void setRudder(irr::f32); //Set the rudder (-ve is port, +ve is stbd)
        void setPortEngine(irr::f32); //Set the engine, (-ve astern, +ve ahead)
        void setStbdEngine(irr::f32); //Set the engine, (-ve astern, +ve ahead)
        irr::f32 getPortEngine() const; //-1 to 1 : Fixme: check consistent
        irr::f32 getStbdEngine() const; //-1 to 1
        irr::f32 getRudder() const; //-30 to 30

    protected:
    private:
        std::vector<irr::core::vector3df> views; //The offset of the camera origin from the own ship origin
        Terrain* terrain;
        irr::f32 rollPeriod; //Roll period (s)
        irr::f32 rollAngle; //Roll Angle (deg)
        irr::f32 pitchPeriod; //Roll period (s)
        irr::f32 pitchAngle; //Roll Angle (deg)
        irr::f32 pitch; //(deg)
        irr::f32 roll; //(deg)
        irr::f32 portEngine; //-1 to + 1
        irr::f32 stbdEngine; //-1 to + 1
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

};

#endif // __OWNSHIP_HPP_INCLUDED__
