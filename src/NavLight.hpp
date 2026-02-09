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

#ifndef __NAVLIGHT_HPP_INCLUDED__
#define __NAVLIGHT_HPP_INCLUDED__

#include "graphics/Types.hpp"

#include <string>
#include <cstdint>

// Forward declarations for Irrlicht pointer types
namespace irr {
    namespace scene {
        class ISceneNode;
        class ISceneManager;
        class IMeshSceneNode;
    }
    namespace video {
        class ITexture;
    }
}

class NavLight {

    public:
        NavLight(irr::scene::ISceneNode* parent, irr::scene::ISceneManager* smgr, bc::graphics::Vec3 position, bc::graphics::Color colour, float lightStartAngle, float lightEndAngle, float lightRange, std::string lightSequence="", uint32_t phaseStart=0);
        ~NavLight();
        void update(float scenarioTime, uint32_t lightLevel);
        bc::graphics::Vec3 getPosition() const;
        void setPosition(bc::graphics::Vec3 position);
        void moveNode(float deltaX, float deltaY, float deltaZ);

    private:
        irr::scene::ISceneManager* smgr;
        irr::scene::IMeshSceneNode* lightNode;
        irr::video::ITexture* lightTexture;
        float startAngle;
        float endAngle;
        float range;
        std::string sequence;
        float charTime; //Time in seconds per character in sequence
        float timeOffset;
        uint16_t currentAlpha; //Note that this is u16 not u8 so we can indicate an initial implausible value.
};

#endif
