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

#include "MMGPhysicsModel.hpp"

#include <algorithm>
#include <cmath>

// ── MMGCoefficients estimation from ship dimensions ────────────────────────
// References:
//   Kijima et al. (1990) - "On the Manoeuvring Performance of a Ship
//     with the Parameter of Loading Condition"
//   Yasukawa & Yoshimura (2015) - "Introduction of MMG standard method
//     for ship maneuvering predictions"

void MMGCoefficients::estimateFromDimensions(const ShipDimensions& dims) {
    double L = dims.length;
    double B = dims.beam;
    double T = dims.draught;
    double Cb = dims.blockCoefficient;

    // Added mass (Clarke 1983)
    m_x_prime = 0.05 * Cb;
    m_y_prime = 3.14159 * (T / L) * (1.0 + 0.16 * Cb * B / T - 5.1 * (B / L) * (B / L));
    J_z_prime = 3.14159 * (T / L) * (1.0 / 12.0 + 0.017 * Cb * B / T - 0.33 * (B / L));

    // Hull force derivatives (Kijima's regression formulae)
    Y_v_prime = -(0.5 * 3.14159 * (T / L) + 1.4 * Cb * (B / L));
    Y_r_prime = 0.5 * Cb * (B / L);
    N_v_prime = -(0.54 * (T / L) + (T / L) * (T / L));
    N_r_prime = -(0.54 * (T / L) - (T / L) * (T / L));

    // Higher-order derivatives (approximate from linear terms)
    Y_vvv_prime = Y_v_prime * 5.0;
    Y_rrr_prime = 0.008;
    Y_vvr_prime = 0.379;
    Y_vrr_prime = Y_v_prime * 1.2;
    N_vvv_prime = N_v_prime * 0.2;
    N_rrr_prime = N_r_prime * 0.25;
    N_vvr_prime = N_v_prime * 2.0;
    N_vrr_prime = 0.055;

    // Cross-coupling terms
    X_vv_prime = -0.04;
    X_vr_prime = 0.002;
    X_rr_prime = 0.011;

    // Resistance (estimated from block coefficient)
    X_0_prime = -(0.013 + 0.009 * Cb);

    // Propeller interaction (typical values)
    t_P = 0.10 + 0.15 * Cb;
    w_P0 = 0.50 * Cb;

    // Rudder (typical values scaled by ship size)
    A_R = 0.018 * L * T; // Rudder lateral area ~ 1.8% of L*T
    double rudderSpan = 0.7 * T; // Typical span ~ 70% of draught
    double aspectRatio = rudderSpan * rudderSpan / A_R; // Geometric aspect ratio
    f_alpha = 6.13 * aspectRatio / (aspectRatio + 2.25); // Lift curve slope
}

// ── Constructor ─────────────────────────────────────────────────────────────

MMGPhysicsModel::MMGPhysicsModel() {
    // Default KVLCC2 dimensions
    dims.length = 320.0;
    dims.beam = 58.0;
    dims.draught = 20.8;
    dims.displacement = 312600.0 * 1000.0; // tonnes to kg
    dims.blockCoefficient = 0.8098;
    dims.maxSpeed = 15.5 * 0.5144; // 15.5 knots to m/s
    dims.maxEngineForce = 2.5e6;
    dims.propellerDiameter = 9.86;
    dims.maxRPM = 100.0;
}

MMGPhysicsModel::MMGPhysicsModel(const ShipDimensions& d) : dims(d) {
    coeffs.estimateFromDimensions(dims);
}

MMGPhysicsModel::MMGPhysicsModel(const ShipDimensions& d, const MMGCoefficients& c)
    : dims(d), coeffs(c) {
}

// ── Mass with added mass ───────────────────────────────────────────────────

double MMGPhysicsModel::getMassX() const {
    double m = dims.displacement;
    double m_added = coeffs.m_x_prime * 0.5 * RHO * dims.length * dims.length * dims.draught;
    return m + m_added;
}

double MMGPhysicsModel::getMassY() const {
    double m = dims.displacement;
    double m_added = coeffs.m_y_prime * 0.5 * RHO * dims.length * dims.length * dims.draught;
    return m + m_added;
}

double MMGPhysicsModel::getInertiaZ() const {
    // Ship moment of inertia (approximate as uniform rod)
    double Izz = dims.displacement * dims.length * dims.length / 12.0;
    double J_added = coeffs.J_z_prime * 0.5 * RHO * pow(dims.length, 4) * dims.draught;
    return Izz + J_added;
}

// ── Shallow water correction ────────────────────────────────────────────────
// Reference: Gronarz (1997) power function model
// Factor is 1.0 in deep water (h/T > 4.0) and increases as depth decreases.
// At h/T = 1.2 (very shallow), factors are roughly 2-3x.

double MMGPhysicsModel::shallowWaterFactor(double hT_ratio) {
    if (hT_ratio >= 4.0) return 1.0;
    if (hT_ratio < 1.05) hT_ratio = 1.05; // Prevent singularity at h/T = 1.0
    // Empirical: factor = (T/h)^0.5 approximately matches Gronarz's results
    // for sway/yaw derivatives. More conservative than some published values.
    return pow(1.0 / (1.0 - 1.0 / hT_ratio), 0.5);
}

// ── Barras squat ────────────────────────────────────────────────────────────
// Reference: Barrass (1979), regression from 600+ measurements
// Returns bow squat in metres (positive = sinkage)

double MMGPhysicsModel::computeSquat(double speedKnots, double hT_ratio) const {
    if (hT_ratio > 4.0 || speedKnots < 0.5) return 0.0;
    // Open water squat formula: squat = Cb * V^2.08 / 100
    double squat = dims.blockCoefficient * pow(speedKnots, 2.08) / 100.0;
    // Increase in shallow water
    if (hT_ratio < 4.0 && hT_ratio > 1.05) {
        squat *= pow(4.0 / hT_ratio, 0.7);
    }
    return squat;
}

// ── Bank effects (Norrbin model) ────────────────────────────────────────────
// Reference: Norrbin (1974), Ch'ng et al. (1993)
// Ship is attracted toward nearest bank, bow is pushed away.

void MMGPhysicsModel::computeBankForces(double u, double L, double T,
                                          double distPort, double distStbd,
                                          double& Yb, double& Nb) {
    // Bank effect coefficients (typical values from Norrbin)
    static constexpr double C_bank_Y = 0.2;  // Lateral suction coefficient
    static constexpr double C_bank_N = 0.1;  // Yaw moment coefficient

    double U_sq = u * u;
    if (U_sq < 0.01 || (distPort > 5.0 * L && distStbd > 5.0 * L)) {
        Yb = 0; Nb = 0;
        return;
    }

    double qLT = 0.5 * RHO * L * T * U_sq;

    // Force towards nearest bank (positive Y = starboard)
    // 1/d^2 law: each bank attracts proportionally
    double yPort = 0.0, yStbd = 0.0;
    if (distPort > 0.1) {
        yPort = -C_bank_Y * qLT * (T / distPort) * (T / distPort); // Pull to port = -ve
    }
    if (distStbd > 0.1) {
        yStbd = C_bank_Y * qLT * (T / distStbd) * (T / distStbd); // Pull to stbd = +ve
    }
    Yb = yPort + yStbd;

    // Bow pushed away from nearest bank (yaw moment)
    double nPort = 0.0, nStbd = 0.0;
    if (distPort > 0.1) {
        nPort = C_bank_N * qLT * L * (T / distPort) * (T / distPort); // Bow to stbd = +ve
    }
    if (distStbd > 0.1) {
        nStbd = -C_bank_N * qLT * L * (T / distStbd) * (T / distStbd); // Bow to port = -ve
    }
    Nb = nPort + nStbd;
}

// ── Isherwood wind force model ──────────────────────────────────────────────
// Reference: Isherwood (1972) - "Wind Resistance of Merchant Ships"
// Uses apparent wind angle to compute longitudinal, lateral, and yaw coefficients.

void MMGPhysicsModel::computeWindForces(double windSpeed, double windDirection,
                                          double heading, double u, double v,
                                          double L, double B, double draught,
                                          double lateralArea, double frontalArea,
                                          double superstructureAft,
                                          double& Xw, double& Yw, double& Nw) {
    static constexpr double DEG2RAD = 3.14159265358979323846 / 180.0;

    if (windSpeed < 0.5) {
        Xw = 0; Yw = 0; Nw = 0;
        return;
    }

    // Default areas if not provided
    if (lateralArea <= 0.0) lateralArea = 0.4 * L * (0.5 * draught + 3.0); // Rough estimate
    if (frontalArea <= 0.0) frontalArea = 0.6 * B * (0.5 * draught + 3.0);
    if (superstructureAft <= 0.0) superstructureAft = 0.6 * L; // Centroid ~60% from bow

    // Compute apparent wind in body frame
    double windDir_rad = windDirection * DEG2RAD;
    double heading_rad = heading * DEG2RAD;

    // True wind components in world frame
    double wx_world = -windSpeed * sin(windDir_rad); // Wind FROM this direction
    double wz_world = -windSpeed * cos(windDir_rad);

    // Ship velocity in world frame
    double sin_h = sin(heading_rad);
    double cos_h = cos(heading_rad);
    double vx_world = u * sin_h + v * cos_h;
    double vz_world = u * cos_h - v * sin_h;

    // Apparent wind in world frame
    double awx = wx_world - vx_world;
    double awz = wz_world - vz_world;

    // Rotate to body frame (surge = forward along heading)
    double aw_surge = awx * sin_h + awz * cos_h;   // Head wind component
    double aw_sway = awx * cos_h - awz * sin_h;    // Cross wind component

    double Va = sqrt(aw_surge * aw_surge + aw_sway * aw_sway);
    if (Va < 0.5) {
        Xw = 0; Yw = 0; Nw = 0;
        return;
    }

    // Apparent wind angle (0 = head-on, 90 = beam, 180 = stern)
    double gamma = atan2(aw_sway, -aw_surge); // 0 = headwind

    // Isherwood coefficients (simplified Fourier series fit)
    // Cx = longitudinal force coefficient (negative = drag astern)
    // Cy = lateral force coefficient
    // Cn = yaw moment coefficient
    double cos_g = cos(gamma);
    double sin_g = sin(gamma);

    // Isherwood (1972) simplified: Cx ≈ -0.45 * cos(gamma) (head wind drag)
    // with corrections for beam wind
    double Cx = -0.45 * cos_g + 0.4 * cos_g * cos_g * cos_g;

    // Lateral: strong at beam wind, zero at head/stern
    double Cy = 0.9 * sin_g - 0.2 * sin_g * cos_g * cos_g;

    // Yaw: bow blown downwind, peak at ~30-40 deg off bow
    double Cn = -0.18 * sin_g + 0.1 * sin(2.0 * gamma);

    // Dynamic pressure * reference area
    double q = 0.5 * RHO_AIR * Va * Va;

    Xw = q * frontalArea * Cx;
    Yw = q * lateralArea * Cy;
    Nw = q * lateralArea * L * Cn;
}

// ── Hull forces ─────────────────────────────────────────────────────────────

void MMGPhysicsModel::computeHullForces(double u, double v, double r_rad,
                                         double shallowFactor,
                                         double& Xh, double& Yh, double& Nh) const {
    // U = total speed
    double U = sqrt(u * u + v * v);
    if (U < 0.01) {
        Xh = 0; Yh = 0; Nh = 0;
        return;
    }

    // Non-dimensionalize
    double v_prime = v / U;
    double r_prime = r_rad * dims.length / U;

    // Non-dimensional reference force = 0.5 * rho * L * T * U^2
    double qLT = 0.5 * RHO * dims.length * dims.draught * U * U;

    // Surge (X) - resistance increases in shallow water
    Xh = qLT * (coeffs.X_0_prime * shallowFactor
                 + coeffs.X_vv_prime * v_prime * v_prime
                 + coeffs.X_vr_prime * v_prime * r_prime
                 + coeffs.X_rr_prime * r_prime * r_prime);

    // Sway (Y) - lateral forces increase in shallow water
    Yh = qLT * shallowFactor * (coeffs.Y_v_prime * v_prime
                 + coeffs.Y_r_prime * r_prime
                 + coeffs.Y_vvv_prime * v_prime * v_prime * v_prime
                 + coeffs.Y_vvr_prime * v_prime * v_prime * r_prime
                 + coeffs.Y_vrr_prime * v_prime * r_prime * r_prime
                 + coeffs.Y_rrr_prime * r_prime * r_prime * r_prime);

    // Yaw (N) - yaw damping increases in shallow water
    Nh = qLT * dims.length * shallowFactor * (coeffs.N_v_prime * v_prime
                                + coeffs.N_r_prime * r_prime
                                + coeffs.N_vvv_prime * v_prime * v_prime * v_prime
                                + coeffs.N_vvr_prime * v_prime * v_prime * r_prime
                                + coeffs.N_vrr_prime * v_prime * r_prime * r_prime
                                + coeffs.N_rrr_prime * r_prime * r_prime * r_prime);
}

// ── Propeller force ─────────────────────────────────────────────────────────

void MMGPhysicsModel::computePropellerForce(double u, double engineSetting,
                                              double& Xp) const {
    // Engine setting maps to propeller RPM
    double n = fabs(engineSetting) * dims.maxRPM / 60.0; // rev/s
    if (n < 0.01) {
        Xp = 0;
        return;
    }

    // Wake fraction (simplified - constant for now)
    double w_P = coeffs.w_P0;

    // Advance ratio J = u_a / (n * D)
    double u_a = u * (1.0 - w_P); // advance speed
    double J = u_a / (n * dims.propellerDiameter);

    // Thrust coefficient K_T(J)
    double K_T = coeffs.k_0 + coeffs.k_1 * J + coeffs.k_2 * J * J;
    if (K_T < 0) K_T = 0; // Cannot have negative thrust coefficient in practice

    // Thrust
    double T = RHO * n * n * pow(dims.propellerDiameter, 4) * K_T;

    // Account for ahead/astern
    if (engineSetting < 0) {
        T = -T;
    }

    // Effective thrust (thrust deduction)
    Xp = (1.0 - coeffs.t_P) * T;
}

// ── Rudder forces ───────────────────────────────────────────────────────────

void MMGPhysicsModel::computeRudderForces(double u, double v, double r_rad,
                                            double rudderAngle_deg,
                                            double engineSetting,
                                            double& Xr, double& Yr, double& Nr) const {
    double delta = rudderAngle_deg * DEG_TO_RAD;

    double U = sqrt(u * u + v * v);
    if (U < 0.01 && fabs(engineSetting) < 0.01) {
        Xr = 0; Yr = 0; Nr = 0;
        return;
    }

    // Flow velocity at rudder position
    double w_P = coeffs.w_P0;
    double u_a = u * (1.0 - w_P);

    // Propeller slipstream effect on rudder
    double n = fabs(engineSetting) * dims.maxRPM / 60.0; // rev/s
    double u_R;
    if (n > 0.01) {
        double J = u_a / (n * dims.propellerDiameter);
        double K_T = coeffs.k_0 + coeffs.k_1 * J + coeffs.k_2 * J * J;
        if (K_T < 0) K_T = 0;

        // Propeller-induced velocity (singularity-free form for bollard pull J→0)
        // Derived by substituting J = u_a/(n*D) into the standard formula:
        //   u_R = u_a * eps * sqrt(eta*(1 + 8*K_T/(pi*J^2)) + (1-eta))
        // gives: u_R = eps * sqrt(eta*(u_a^2 + 8*K_T*n^2*D^2/pi) + (1-eta)*u_a^2)
        double eta = dims.propellerDiameter / (coeffs.kappa * dims.length);
        double nD = n * dims.propellerDiameter;
        double u_a_sq = u_a * u_a;
        double prop_wash = 8.0 * K_T * nD * nD / 3.14159;
        u_R = coeffs.epsilon * sqrt(eta * (u_a_sq + prop_wash) + (1.0 - eta) * u_a_sq);
    } else {
        u_R = u * (1.0 - w_P) * coeffs.epsilon;
    }
    if (fabs(u_R) < 0.01) u_R = 0.01; // Avoid division by zero

    // Effective inflow angle at rudder
    double v_R = v + coeffs.l_R_prime * dims.length * r_rad;
    double gamma_R = (delta >= 0) ? coeffs.gamma_R_positive : coeffs.gamma_R_negative;
    double alpha_R = delta - atan2(-gamma_R * v_R, u_R);

    // Rudder normal force
    double F_N = 0.5 * RHO * coeffs.A_R * coeffs.f_alpha * (u_R * u_R + v_R * v_R) * sin(alpha_R);

    // Forces and moment
    Xr = -(1.0 - coeffs.t_R) * F_N * sin(delta);
    Yr = -(1.0 + coeffs.a_H) * F_N * cos(delta);
    Nr = -(coeffs.x_R_prime + coeffs.a_H * coeffs.x_H_prime) * dims.length * F_N * cos(delta);
}

// ── Time step (RK2 midpoint method) ────────────────────────────────────────

void MMGPhysicsModel::step(double dt, const PhysicsInput& input, PhysicsState& state) {
    // Clamp time step for stability
    dt = std::min(dt, 0.05); // Max 50ms per step

    // Body-frame velocities
    double u = state.surge;
    double v = state.sway;
    double r_rad = state.yawRate * DEG_TO_RAD; // Convert to rad/s
    double heading_rad = state.heading * DEG_TO_RAD;

    // Engine setting (average if twin screw for simplified model)
    double engine = dims.singleEngine ? input.portEngine
                                       : (input.portEngine + input.stbdEngine) / 2.0;

    // Shallow water correction factor
    double hT_ratio = (input.waterDepth + dims.draught) / dims.draught; // Total depth / draught
    double swFactor = shallowWaterFactor(hT_ratio);

    // ── Compute forces at current state ──
    double Xh, Yh, Nh;
    computeHullForces(u, v, r_rad, swFactor, Xh, Yh, Nh);

    double Xp;
    computePropellerForce(u, engine, Xp);

    double Xr, Yr, Nr;
    computeRudderForces(u, v, r_rad, input.rudderAngle, engine, Xr, Yr, Nr);

    // Bank effects
    double Yb = 0, Nb = 0;
    computeBankForces(u, dims.length, dims.draught,
                      input.bankDistancePort, input.bankDistanceStbd, Yb, Nb);

    // Wind forces (Isherwood)
    double Xw = 0, Yw = 0, Nw = 0;
    computeWindForces(input.windSpeed, input.windDirection, state.heading, u, v,
                      dims.length, dims.beam, dims.draught,
                      input.lateralWindArea, input.frontalWindArea,
                      input.superstructureAft,
                      Xw, Yw, Nw);

    // Total forces
    double Fx = Xh + Xp + Xr + Xw;
    double Fy = Yh + Yr + Yb + Yw;
    double Mz = Nh + Nr + Nb + Nw;

    // Mass with added mass
    double mx = getMassX();
    double my = getMassY();
    double Iz = getInertiaZ();

    // Coriolis-centripetal coupling terms
    // (mass + m_y_added) * v * r for surge equation
    // -(mass + m_x_added) * u * r for sway equation
    double m = dims.displacement;
    double m_x_add = coeffs.m_x_prime * 0.5 * RHO * dims.length * dims.length * dims.draught;
    double m_y_add = coeffs.m_y_prime * 0.5 * RHO * dims.length * dims.length * dims.draught;

    double du_dt = (Fx + (m + m_y_add) * v * r_rad) / mx;
    double dv_dt = (Fy - (m + m_x_add) * u * r_rad) / my;
    double dr_dt = Mz / Iz;

    // ── RK2 midpoint integration ──
    // Half step
    double u_mid = u + 0.5 * dt * du_dt;
    double v_mid = v + 0.5 * dt * dv_dt;
    double r_mid = r_rad + 0.5 * dt * dr_dt;

    // Recompute forces at midpoint
    computeHullForces(u_mid, v_mid, r_mid, swFactor, Xh, Yh, Nh);
    computePropellerForce(u_mid, engine, Xp);
    computeRudderForces(u_mid, v_mid, r_mid, input.rudderAngle, engine, Xr, Yr, Nr);
    computeBankForces(u_mid, dims.length, dims.draught,
                      input.bankDistancePort, input.bankDistanceStbd, Yb, Nb);
    computeWindForces(input.windSpeed, input.windDirection, state.heading, u_mid, v_mid,
                      dims.length, dims.beam, dims.draught,
                      input.lateralWindArea, input.frontalWindArea,
                      input.superstructureAft,
                      Xw, Yw, Nw);

    Fx = Xh + Xp + Xr + Xw;
    Fy = Yh + Yr + Yb + Yw;
    Mz = Nh + Nr + Nb + Nw;

    du_dt = (Fx + (m + m_y_add) * v_mid * r_mid) / mx;
    dv_dt = (Fy - (m + m_x_add) * u_mid * r_mid) / my;
    dr_dt = Mz / Iz;

    // Full step using midpoint derivatives
    state.surge += dt * du_dt;
    state.sway += dt * dv_dt;
    double new_r_rad = r_rad + dt * dr_dt;
    state.yawRate = new_r_rad * RAD_TO_DEG;

    // Update heading
    state.heading += state.yawRate * dt;
    // Normalize heading to [0, 360)
    while (state.heading < 0) state.heading += 360.0;
    while (state.heading >= 360.0) state.heading -= 360.0;

    // Update position (convert body frame to world frame)
    double cos_h = cos(heading_rad);
    double sin_h = sin(heading_rad);
    // Note: heading 0 = North = +Z, heading 90 = East = +X
    state.posX += dt * (state.surge * sin_h + state.sway * cos_h);
    state.posZ += dt * (state.surge * cos_h - state.sway * sin_h);
}
