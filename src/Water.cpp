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

//NOTE: This uses a modified version of Irrlicht for the water surface scene node, which bases the waves
//on the absolute position, so you can tile multiple water nodes seamlessly.

#include <vector>
#include <iostream>

#include "Water.hpp"
#include "MovingWater.hpp"
#include "Utilities.hpp"

//using namespace irr;

Water::Water()
{

}

Water::~Water()
{
    //dtor
}

void Water::load(irr::scene::ISceneManager* smgr, irr::scene::ISceneNode* ownShip, float weather, uint32_t disableShaders, bool withReflection, uint32_t segments)
{

    irr::video::IVideoDriver* driver = smgr->getVideoDriver();

    //Set tile width
    //FIXME: Hardcoded or defined in multiple places
    tileWidth = 100; //Width in metres - Note this is used in Simulation model normalisation as 1000, so visible jumps in water are minimised


    waterNode = new irr::scene::MovingWaterSceneNode(smgr->getRootSceneNode(),smgr,ownShip,0,disableShaders,withReflection,segments);

    //waterNode->setPosition(irr::core::vector3df(0,-0.25f,0));

    waterNode->setMaterialTexture(0, driver->getTexture("media/water.bmp"));

}

void Water::update(float tideHeight, bc::graphics::Vec3 viewPosition, uint32_t lightLevel, float weather, float windSpeedKts, float windDirectionDeg)
{
    //Round these to nearest tileWidth
    float xPos = tileWidth * Utilities::round(viewPosition.x/tileWidth);
    float yPos = tideHeight;
    float zPos = tileWidth * Utilities::round(viewPosition.z/tileWidth);

    waterNode->setPosition(irr::core::vector3df(xPos,yPos,zPos));

    // Couple wind speed to wave parameters using Beaufort relationship:
    //   Hs = 0.0246 * U^2  (significant wave height from wind speed in m/s)
    //
    // Beaufort | Wind (kts) | U (m/s) | Hs (m) | Description
    //    0     |   0        |   0     |   0    | Calm
    //    2     |   5        |   2.6   |   0.2  | Smooth
    //    4     |   13       |   6.7   |   1.1  | Moderate
    //    6     |   25       |  12.9   |   4.1  | Very rough
    //    8     |   37       |  19.0   |   8.9  | Very high
    //   10     |   50       |  25.7   |  16.2  | Phenomenal
    //
    // The 'A' parameter in the FFT spectrum scales wave amplitude.
    // We derive A from significant wave height:
    //   Wave variance σ² ∝ A, and Hs = 4*σ, so A ∝ Hs²
    // Empirically tuned: A_base = 0.000025 gives ~0.5m Hs at Bf 4

    float windSpeed_mps = windSpeedKts * 0.5144f; // knots to m/s

    // Significant wave height from wind speed (Pierson-Moskowitz fully-developed sea)
    float Hs = 0.0246f * windSpeed_mps * windSpeed_mps;

    // Clamp Hs to reasonable range for the simulation tile size
    if (Hs < 0.01f) Hs = 0.01f;
    if (Hs > 15.0f) Hs = 15.0f;

    // Phillips/JONSWAP A parameter: scales as Hs² (wave variance)
    // Calibrated so A=0.000025 ≈ Hs=0.5m at the 100m tile with 64-point FFT
    float A = 0.0001f * Hs * Hs;

    // Wind vector for wave direction (wind blows FROM windDirection, waves travel WITH it)
    // Convert meteorological direction to wave propagation direction
    float windRad = (windDirectionDeg + 180.0f) * 3.14159265f / 180.0f;
    float windX = windSpeed_mps * sin(windRad);
    float windZ = windSpeed_mps * cos(windRad);
    // Minimum wind to avoid zero-vector
    if (windSpeed_mps < 0.5f) {
        windX = 0.5f;
        windZ = 0.5f;
    }

    waterNode->resetParameters(A, vector2(windX, windZ), weather + 0.25f);

}

float Water::getWaveHeight(float relPosX, float relPosZ) const
{
    return waterNode->getWaveHeight(relPosX,relPosZ);
}

bc::graphics::Vec2 Water::getLocalNormals(float relPosX, float relPosZ) const
{
    irr::core::vector2df n = waterNode->getLocalNormals(relPosX,relPosZ);
    return bc::graphics::Vec2(n.X, n.Y);
}


bc::graphics::Vec3 Water::getPosition() const
{
    irr::core::vector3df p = waterNode->getPosition();
    return bc::graphics::Vec3(p.X, p.Y, p.Z);
}

void Water::setVisible(bool visible)
{
    waterNode->setVisible(visible);
}
