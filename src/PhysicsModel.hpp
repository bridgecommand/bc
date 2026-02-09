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

#ifndef __PHYSICSMODEL_HPP_INCLUDED__
#define __PHYSICSMODEL_HPP_INCLUDED__

#include <cmath>

// State vector for 3-DOF ship motion (surge, sway, yaw)
struct PhysicsState {
    double posX = 0.0;       // Position X (internal coords, metres)
    double posZ = 0.0;       // Position Z (internal coords, metres)
    double heading = 0.0;    // Heading (degrees, 0=North, CW positive)
    double surge = 0.0;      // Forward velocity (m/s, body frame)
    double sway = 0.0;       // Lateral velocity (m/s, body frame, +ve = starboard)
    double yawRate = 0.0;    // Yaw angular velocity (deg/s, CW positive)
    double roll = 0.0;       // Roll angle (degrees)
    double pitch = 0.0;      // Pitch angle (degrees)
};

// Control and environment inputs
struct PhysicsInput {
    double rudderAngle = 0.0;    // Rudder angle (degrees, +ve = starboard)
    double portEngine = 0.0;     // Port engine setting (-1 to +1)
    double stbdEngine = 0.0;     // Starboard engine setting (-1 to +1)
    double windSpeed = 0.0;      // True wind speed (m/s)
    double windDirection = 0.0;  // True wind direction (degrees, from)
    double currentSpeed = 0.0;   // Water current speed (m/s)
    double currentDirection = 0.0; // Water current direction (degrees, towards)
    double waveHeight = 0.0;     // Significant wave height (m)
};

// Ship hull dimensions and characteristics
struct ShipDimensions {
    double length = 100.0;       // Length between perpendiculars (m)
    double beam = 16.0;          // Beam / width (m)
    double draught = 6.0;        // Draught / depth (m)
    double displacement = 0.0;   // Displacement (kg)
    double blockCoefficient = 0.8; // Block coefficient Cb
    double maxSpeed = 10.0;      // Maximum speed (m/s)
    double maxEngineForce = 1e6; // Maximum engine thrust (N)
    double propellerDiameter = 4.0; // Propeller diameter (m)
    double maxRPM = 120.0;      // Maximum propeller RPM
    bool singleEngine = false;   // True if single engine (vs twin screw)
};

// Abstract physics model interface
class IPhysicsModel {
public:
    virtual ~IPhysicsModel() = default;

    // Advance state by dt seconds
    virtual void step(double dt, const PhysicsInput& input, PhysicsState& state) = 0;

    // Get ship dimensions (for queries)
    virtual const ShipDimensions& getDimensions() const = 0;
};

#endif // __PHYSICSMODEL_HPP_INCLUDED__
