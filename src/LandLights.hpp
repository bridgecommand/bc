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

#ifndef __LANDLIGHTS_HPP_INCLUDED__
#define __LANDLIGHTS_HPP_INCLUDED__

#include <vector>
#include <string>
#include <cstdint>

// Forward declarations
namespace irr {
    namespace scene {
        class ISceneManager;
    }
}
class SimulationModel;
class NavLight;
class Terrain;

class LandLights
{
    public:
        LandLights();
        virtual ~LandLights();
        void load(const std::string& worldName, irr::scene::ISceneManager* smgr, SimulationModel* model, const Terrain& terrain);
        void update(float deltaTime, float scenarioTime, uint32_t lightLevel);
        uint32_t getNumber() const;
        void moveNode(float deltaX, float deltaY, float deltaZ);
    private:
        std::vector<NavLight*> landLights;
};

#endif
