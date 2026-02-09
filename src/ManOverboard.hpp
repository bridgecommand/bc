/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2017 James Packer

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

#ifndef __MANOVERBOARD_HPP_INCLUDED__
#define __MANOVERBOARD_HPP_INCLUDED__

#include "graphics/Types.hpp"

// Forward declarations
namespace irr {
    class IrrlichtDevice;
    namespace scene {
        class ISceneManager;
        class ISceneNode;
        class IMeshSceneNode;
    }
}
class SimulationModel;

class ManOverboard
{
    public:
        ManOverboard(const bc::graphics::Vec3& location, irr::scene::ISceneManager* smgr, irr::IrrlichtDevice* dev, SimulationModel* model);
        bc::graphics::Vec3 getPosition() const;
        void setVisible(bool isVisible);
        bool getVisible() const;
        void setPosition(bc::graphics::Vec3 position);
        void setRotation(bc::graphics::Vec3 rotation);
        void moveNode(float deltaX, float deltaY, float deltaZ);
        void update(float deltaTime, float tideHeight);
        irr::scene::ISceneNode* getSceneNode() const;
    protected:
    private:
        irr::scene::IMeshSceneNode* man; //The scene node for the man overboard model.
        SimulationModel* model;
};

#endif
