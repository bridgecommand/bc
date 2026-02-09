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

#ifndef __RADARSCREEN_HPP_INCLUDED__
#define __RADARSCREEN_HPP_INCLUDED__

#include <cstdint>
#include "graphics/Types.hpp"

namespace irr { namespace scene { class ISceneManager; class ISceneNode; class IMeshSceneNode; } }
namespace irr { namespace video { class IVideoDriver; class IImage; } }

class RadarScreen
{
    public:
        RadarScreen();
        virtual ~RadarScreen();

        void load(irr::scene::ISceneManager* smgr, irr::scene::ISceneNode* parent, bc::graphics::Vec3 offset, float size, float tilt);
        void setRadarDisplayRadius(uint32_t radiusPx);
        void update(irr::video::IImage* radarImage);
        irr::scene::ISceneNode* getSceneNode() const;


    private:
        irr::video::IVideoDriver* driver;
        irr::scene::IMeshSceneNode* radarScreen;
        irr::scene::ISceneNode* parent;
        bc::graphics::Vec3 offset;
        uint32_t radarRadiusPx;
        float tilt;
};

#endif
