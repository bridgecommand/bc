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

#include "LegacyPhysicsModel.hpp"

#include <algorithm>
#include <cmath>

static constexpr double DEG_TO_RAD = 3.14159265358979323846 / 180.0;
static constexpr double RAD_TO_DEG = 180.0 / 3.14159265358979323846;

LegacyPhysicsModel::LegacyPhysicsModel() {
    // Defaults from LegacyShipParams
}

LegacyPhysicsModel::LegacyPhysicsModel(const LegacyShipParams& p) : params(p) {
    dims.displacement = p.shipMass;
    dims.maxEngineForce = p.maxForce;
    dims.singleEngine = p.singleEngine;
}

const ShipDimensions& LegacyPhysicsModel::getDimensions() const {
    return dims;
}

void LegacyPhysicsModel::step(double dt, const PhysicsInput& input, PhysicsState& state) {
    // Clamp time step for stability
    dt = std::min(dt, 0.1);

    double portEngine = input.portEngine;  // -1 to +1
    double stbdEngine = input.stbdEngine;  // -1 to +1
    double rudder = input.rudderAngle;     // degrees, +ve starboard

    // ── Compute thrust ──────────────────────────────────────────────────
    double portThrust = portEngine * params.maxForce;
    double stbdThrust = stbdEngine * params.maxForce;

    if (portThrust < 0) portThrust *= params.asternEfficiency;
    if (stbdThrust < 0) stbdThrust *= params.asternEfficiency;

    // For conventional engines, all thrust is axial
    double portAxialThrust = portThrust;
    double stbdAxialThrust = stbdThrust;

    // Single engine: mirror port to stbd (internally two half-power engines)
    if (params.singleEngine) {
        stbdThrust = portThrust;
        stbdAxialThrust = portAxialThrust;
    }

    // ── Axial dynamics ──────────────────────────────────────────────────
    // Speed through water (for drag calculations)
    // In the full model, speedThroughWater = surge - axialStream
    // Here we use surge directly; current effects are applied externally
    double speedThroughWater = state.surge;

    double axialDrag;
    if (speedThroughWater < 0) {
        // Compensate for loss of sign when squaring
        axialDrag = -params.dynamicsSpeedA * speedThroughWater * speedThroughWater
                    + params.dynamicsSpeedB * speedThroughWater;
    } else {
        axialDrag = params.dynamicsSpeedA * speedThroughWater * speedThroughWater
                    + params.dynamicsSpeedB * speedThroughWater;
    }

    double axialAcceleration = (portAxialThrust + stbdAxialThrust - axialDrag) / params.shipMass;
    // Plausibility clamp
    axialAcceleration = std::max(-9.81, std::min(9.81, axialAcceleration));

    state.surge += axialAcceleration * dt;
    state.surge = std::max(-50.0, std::min(50.0, state.surge));

    // ── Lateral dynamics ────────────────────────────────────────────────
    // For conventional (non-azimuth) ships, there is no lateral thrust from engines.
    // Lateral drag damps any existing lateral motion.
    double lateralDrag;
    if (state.sway < 0) {
        lateralDrag = -params.dynamicsLateralDragA * state.sway * state.sway
                      + params.dynamicsLateralDragB * state.sway;
    } else {
        lateralDrag = params.dynamicsLateralDragA * state.sway * state.sway
                      + params.dynamicsLateralDragB * state.sway;
    }

    double lateralAcceleration = -lateralDrag / params.shipMass;
    lateralAcceleration = std::max(-9.81, std::min(9.81, lateralAcceleration));

    state.sway += lateralAcceleration * dt;
    state.sway = std::max(-50.0, std::min(50.0, state.sway));

    // ── Turn dynamics ───────────────────────────────────────────────────
    double rateOfTurn_rad = state.yawRate * DEG_TO_RAD; // Convert to rad/s for legacy calcs

    // Rudder torque
    double rudderTorque;
    if ((portThrust + stbdThrust) > 0) {
        rudderTorque = rudder * speedThroughWater * params.rudderA
                     + rudder * (portThrust + stbdThrust) * params.rudderB;
    } else {
        rudderTorque = rudder * speedThroughWater * params.rudderA
                     + rudder * (portThrust + stbdThrust) * params.rudderBAstern;
    }

    // Engine torque (differential thrust)
    double engineTorque;
    if (params.singleEngine) {
        engineTorque = 0; // No differential with single engine
    } else {
        engineTorque = (portThrust * params.propellorSpacing
                      - stbdThrust * params.propellorSpacing) / 2.0;
    }

    // Prop walk torque
    double propWalkTorquePort, propWalkTorqueStbd;
    if (portThrust > 0) {
        propWalkTorquePort = params.propWalkAhead * (portThrust / params.maxForce);
    } else {
        propWalkTorquePort = params.propWalkAstern * (portThrust / params.maxForce);
    }
    if (stbdThrust > 0) {
        propWalkTorqueStbd = -params.propWalkAhead * (stbdThrust / params.maxForce);
    } else {
        propWalkTorqueStbd = -params.propWalkAstern * (stbdThrust / params.maxForce);
    }

    double propWalkTorque;
    if (params.singleEngine) {
        propWalkTorque = 2 * propWalkTorquePort;
    } else {
        propWalkTorque = propWalkTorquePort + propWalkTorqueStbd;
    }

    // Turn drag
    double dragTorque;
    if (rateOfTurn_rad < 0) {
        dragTorque = -(-params.dynamicsTurnDragA * rateOfTurn_rad * rateOfTurn_rad
                       + params.dynamicsTurnDragB * rateOfTurn_rad);
    } else {
        dragTorque = -(params.dynamicsTurnDragA * rateOfTurn_rad * rateOfTurn_rad
                       + params.dynamicsTurnDragB * rateOfTurn_rad);
    }

    // Angular acceleration
    double angularAcceleration = (rudderTorque + engineTorque + propWalkTorque + dragTorque)
                                 / params.inertia;

    rateOfTurn_rad += angularAcceleration * dt;
    // Plausibility clamp (~4*pi rad/s)
    rateOfTurn_rad = std::max(-12.0, std::min(12.0, rateOfTurn_rad));

    state.yawRate = rateOfTurn_rad * RAD_TO_DEG;

    // ── Integration ─────────────────────────────────────────────────────
    double heading_rad = state.heading * DEG_TO_RAD;

    // Update heading
    state.heading += state.yawRate * dt;
    while (state.heading < 0) state.heading += 360.0;
    while (state.heading >= 360.0) state.heading -= 360.0;

    // Update position (body frame to world frame)
    // heading 0 = North = +Z, heading 90 = East = +X
    state.posX += dt * (state.surge * sin(heading_rad) + state.sway * cos(heading_rad));
    state.posZ += dt * (state.surge * cos(heading_rad) - state.sway * sin(heading_rad));
}
