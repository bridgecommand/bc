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
        void update(irr::f32 deltaTime, irr::f32 tideHeight);
        std::vector<irr::core::vector3df> getCameraViews() const;
        irr::f32 getDepth();
        void setRudder(irr::f32); //Set the rudder (-ve is port, +ve is stbd)
        void setPortEngine(irr::f32); //Set the engine, (-ve astern, +ve ahead)
        void setStbdEngine(irr::f32); //Set the engine, (-ve astern, +ve ahead)

    protected:
    private:
        std::vector<irr::core::vector3df> views; //The offset of the camera origin from the own ship origin
        Terrain* terrain;
        irr::f32 portEngine; //-1 to + 1
        irr::f32 stbdEngine; //-1 to + 1
        irr::f32 rudder; //-30 to + 30

};

#endif // __OWNSHIP_HPP_INCLUDED__
