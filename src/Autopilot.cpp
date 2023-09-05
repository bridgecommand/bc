/*   BridgeCommand 5.7 Copyright (C) James Packer
     This file is Copyright (C) 2022 Fraunhofer FKIE

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

#include "Autopilot.hpp"
#include "Angles.hpp"
#include "Constants.hpp"
#include "IniFile.hpp"
#include "NMEASentences.hpp"
#include "SimulationModel.hpp"
#include "Utilities.hpp"
#include "irrTypes.h"
#include <algorithm>
#include <iostream>
#include <tuple>

Autopilot::Autopilot(SimulationModel* model)
{
    this->model = model;
    currentWaypointPos = {INVALID_LAT, INVALID_LONG};
    currentWaypointId = "";
    currentLegLen = 0;

    // Check if user set Autopilot to be enabled
    std::string userFolder = Utilities::getUserDir();
    std::string iniFilename = "bc5.ini";
    AUTOPILOT_ENABLED = false;
    if (Utilities::pathExists(userFolder + iniFilename)) {
        iniFilename = userFolder + iniFilename;
    }
    std::string enableAutopilot = IniFile::iniFileToString(iniFilename, "Autopilot_Enable", "false");
    if (!enableAutopilot.compare("true")) AUTOPILOT_ENABLED = true;
}

bool Autopilot::receiveAPB(APB sentence)
{
    // determine where to steer to depending on APB
    if (!AUTOPILOT_ENABLED) return false;

    // how far off-track are we?
    crossTrackError = sentence.cross_track_error;
    if (sentence.cross_track_units == 'N') {
        crossTrackError *= M_IN_NM;
    }
    char directionToTrack = sentence.direction;

    irr::f32 bearingToSteer = Angles::normaliseAngle(sentence.heading_to_dest);
    irr::f32 currentHeading = Angles::normaliseAngle(model->getHeading());
    irr::f32 relativeBearing = bearingToSteer - currentHeading;
    if (relativeBearing >= 180.0) {
        relativeBearing -= 360.0;
    }
    if (relativeBearing <= -180.0) {
        relativeBearing += 360.0;
    }

    irr::f32 rot = model->getRateOfTurn() * DEG_IN_RAD;
    irr::f32 dampening = 1.0;
    if (rot != 0.0) {
        irr::f32 timeUntilOvershoot = relativeBearing / rot;
        if (0 <= timeUntilOvershoot && timeUntilOvershoot < 15) {
            // linear scale from no dampening at 15s to steering into the
            // opposite direction at less than 2.0
            dampening = (1.0 / 13.0) * timeUntilOvershoot - (2.0 / 13.0);
        }
    }

    // set wheel to val between -30.0 (>=60 deg L) and 30.0 (>=60 deg R) (setWheel clamps vals)
    irr::f32 wheel = (relativeBearing / 60.0) * 30.0;

    if (model->isAzimuthDrive()) {
        // Special case for azimuth drive, as the azimuth angle to steer is opposite, and 
        // isn't clamped in setXXXAzimuthAngle()
        
        if (wheel > 30) {
            wheel = 30;
        }
        if (wheel <-30) {
            wheel = -30;
        }
        // Just set both azimuth angles to steer, no clever handling
        model->setPortAzimuthAngle(-1*wheel);
        model->setStbdAzimuthAngle(-1*wheel);
    } else {
        // Normal case, just set the wheel
        model->setWheel(wheel * dampening);
    }

    return false;
}

bool Autopilot::receiveRMB(RMB sentence)
{
    // determine how much to accelerate/decelerate based on RMB
    if (!AUTOPILOT_ENABLED) return false;
   
    irr::f32 destWaypointLat = parseNmeaLat(
            sentence.dest_waypoint_latitude,
            sentence.dest_waypoint_latitude_dir);
    irr::f32 destWaypointLong = parseNmeaLong(
            sentence.dest_waypoint_longitude,
            sentence.dest_waypoint_longitude_dir);

    if (destWaypointLat != INVALID_LAT && destWaypointLong != INVALID_LONG) {
        currentWaypointPos[0] = destWaypointLat;
        currentWaypointPos[1] = destWaypointLong;
    }

    if (sentence.dest_waypoint_id != currentWaypointId) {
        currentLegLen = sentence.range_to_dest * M_IN_NM;
        currentWaypointId = sentence.dest_waypoint_id;
    }

    // adjust motor throttle based on leg length if desired
    irr::f32 throttle = 1.0;
    if (currentLegLen < 150) {
        // very short leg, navigate cautiously
        throttle = 0.15;
    } else if (currentLegLen < 900) {
        throttle = currentLegLen / 900;
    } else {
        throttle = 1.0;
    }
    // start "breaking" when near the end of a leg (> 75% there)
    // breaking is linear from no breaking up to 0.1 of the leg throttle at 100%
    // with a minimum of 0.1 throttle
    if (currentLegLen == 0) {
        return true;
    }
    irr::f32 leg_progress = 1.0 - ((sentence.range_to_dest * M_IN_NM) / currentLegLen);
    if (leg_progress > 0.75) {
        throttle = std::max(0.1, throttle * (-3.6 * leg_progress + 3.7));
    }
    model->setPortEngine(throttle);
    model->setStbdEngine(throttle);
    return false;
}

Autopilot::~Autopilot()
{
}
