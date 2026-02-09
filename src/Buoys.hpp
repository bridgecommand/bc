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

#ifndef __BUOYS_HPP_INCLUDED__
#define __BUOYS_HPP_INCLUDED__

#include "graphics/Types.hpp"

#include <vector>
#include <string>

//Forward declarations
class SimulationModel;
class Buoy;
class NavLight;
struct RadarData;
namespace irr { class IrrlichtDevice; namespace scene { class ISceneManager; class ISceneNode; } }

class Buoys
{
    public:
        Buoys();
        virtual ~Buoys();
        void load(const std::string& worldName, irr::scene::ISceneManager* smgr, SimulationModel* model, irr::IrrlichtDevice* dev);
        void update(float deltaTime, float scenarioTime, float tideHeight, uint32_t lightLevel, bc::graphics::Vec3 ownShipPosition, float ownShipLength);
        RadarData getRadarData(uint32_t number, bc::graphics::Vec3 scannerPosition) const;
        uint32_t getNumber() const;
        bc::graphics::Vec3 getPosition(int number) const;
        void moveNode(float deltaX, float deltaY, float deltaZ);
        void enableAllTriangleSelectors();
        irr::scene::ISceneNode* getSceneNode(int number);

    private:
        std::vector<Buoy> buoys;
        std::vector<NavLight*> buoysLights;
        SimulationModel* model; //Store reference to model
};

#endif
