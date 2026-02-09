/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2024 James Packer

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

#ifndef __LEGACYPHYSICSMODEL_HPP_INCLUDED__
#define __LEGACYPHYSICSMODEL_HPP_INCLUDED__

#include "PhysicsModel.hpp"

// Parameters that define a legacy physics ship
struct LegacyShipParams {
    double shipMass = 10000.0;          // Mass (kg)
    double inertia = 1e7;              // Izz - yaw moment of inertia (kg*m^2)
    double maxForce = 100000.0;        // Max engine force per engine (N)
    double asternEfficiency = 0.667;   // Astern thrust multiplier (0-1)
    double dynamicsSpeedA = 0.0;       // Axial drag quadratic coeff (N/(m/s)^2)
    double dynamicsSpeedB = 0.0;       // Axial drag linear coeff (N/(m/s))
    double dynamicsLateralDragA = 0.0; // Lateral drag quadratic coeff
    double dynamicsLateralDragB = 0.0; // Lateral drag linear coeff
    double dynamicsTurnDragA = 0.0;    // Turn drag quadratic coeff
    double dynamicsTurnDragB = 0.0;    // Turn drag linear coeff
    double rudderA = 0.0;              // Rudder speed coefficient
    double rudderB = 0.0;              // Rudder thrust coefficient (ahead)
    double rudderBAstern = 0.0;        // Rudder thrust coefficient (astern)
    double propellorSpacing = 0.0;     // Distance between propellers (m)
    double propWalkAhead = 0.0;        // Prop walk torque ahead (N*m at full power)
    double propWalkAstern = 0.0;       // Prop walk torque astern (N*m at full power)
    bool singleEngine = false;         // Single engine (modelled as 2x half-power)
};

// Extracted legacy physics model from OwnShip.cpp
// Handles: engine thrust, axial/lateral/turn drag, rudder torque,
//          engine torque (differential), prop walk, Forward Euler integration.
// Does NOT handle: wind, collision/grounding, thrusters, buffeting, azimuth drives.
// Those external forces are applied in OwnShip.cpp after step(), same as MMG.
class LegacyPhysicsModel : public IPhysicsModel {
public:
    LegacyPhysicsModel();
    explicit LegacyPhysicsModel(const LegacyShipParams& params);

    void step(double dt, const PhysicsInput& input, PhysicsState& state) override;
    const ShipDimensions& getDimensions() const override;

    const LegacyShipParams& getParams() const { return params; }

private:
    LegacyShipParams params;
    ShipDimensions dims; // Minimal, for interface compliance
};

#endif // __LEGACYPHYSICSMODEL_HPP_INCLUDED__
