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

#ifndef __BUOY_HPP_INCLUDED__
#define __BUOY_HPP_INCLUDED__

#include "graphics/Types.hpp"

#include <string>
#include <cmath>

// Forward declarations
namespace irr {
    class IrrlichtDevice;
    namespace scene {
        class ISceneManager;
        class ISceneNode;
        class IMeshSceneNode;
        class ITriangleSelector;
    }
}
struct RadarData;

class Buoy
{
    public:
        Buoy(const std::string& name, const std::string& internalName, const std::string& worldName, const bc::graphics::Vec3& location, float radarCrossSection, bool floating, float heightCorrection, irr::scene::ISceneManager* smgr, irr::IrrlichtDevice* dev);
        virtual ~Buoy();
        bc::graphics::Vec3 getPosition() const;
        void setPosition(bc::graphics::Vec3 position);
        void setRotation(bc::graphics::Vec3 rotation);
        float getLength() const;
        float getHeight() const;
        float getRCS() const;
        float getHeightCorrection() const;
        bool getFloating() const;
        RadarData getRadarData(bc::graphics::Vec3 scannerPosition) const;
        void moveNode(float deltaX, float deltaY, float deltaZ);
        irr::scene::ISceneNode* getSceneNode() const;
        void enableTriangleSelector(bool selectorEnabled);
    protected:
    private:
        irr::scene::IMeshSceneNode* buoy; //The scene node for the buoy.
        irr::scene::ITriangleSelector* selector; //The triangle selector for the buoy. We will set and unset this depending on the distance from the ownship for speed
        float length; //For radar calculation
        float height; //For radar calculation
        float heightCorrection;
        float rcs;
        bool floating; //Does the buoy move with water (normally true, false for a post etc)
        bool triangleSelectorEnabled; //Keep track of this so we don't keep re-setting the selector
};

#endif // __BUOY_HPP_INCLUDED__
