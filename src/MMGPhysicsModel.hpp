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

#ifndef __MMGPHYSICSMODEL_HPP_INCLUDED__
#define __MMGPHYSICSMODEL_HPP_INCLUDED__

#include "PhysicsModel.hpp"

// MMG (Maneuvering Modeling Group) hydrodynamic coefficients
// All non-dimensional (primed) values, reference: Yasukawa & Yoshimura 2015
struct MMGCoefficients {
    // Added mass coefficients (non-dimensional)
    double m_x_prime = 0.022;  // Surge added mass / (0.5 * rho * L^2 * T)
    double m_y_prime = 0.223;  // Sway added mass / (0.5 * rho * L^2 * T)
    double J_z_prime = 0.011;  // Yaw added inertia / (0.5 * rho * L^4 * T)

    // Hull force derivatives
    double X_vv_prime = -0.040; // Surge-sway coupling
    double X_vr_prime = 0.002;  // Surge-sway-yaw coupling
    double X_rr_prime = 0.011;  // Surge-yaw coupling
    double Y_v_prime = -0.315;  // Sway force due to sway velocity
    double Y_r_prime = 0.083;   // Sway force due to yaw rate
    double Y_vvv_prime = -1.607; // Higher order sway
    double Y_vvr_prime = 0.379;  // Coupling term
    double Y_vrr_prime = -0.391; // Coupling term
    double Y_rrr_prime = 0.008;  // Higher order yaw
    double N_v_prime = -0.137;  // Yaw moment due to sway velocity
    double N_r_prime = -0.049;  // Yaw moment due to yaw rate
    double N_vvv_prime = -0.030; // Higher order sway
    double N_vvr_prime = -0.294; // Coupling term
    double N_vrr_prime = 0.055;  // Coupling term
    double N_rrr_prime = -0.013; // Higher order yaw

    // Resistance coefficient
    double X_0_prime = -0.022;  // Straight-ahead resistance at design speed

    // Propeller coefficients (polynomial K_T = k0 + k1*J + k2*J^2)
    double k_0 = 0.2931;
    double k_1 = -0.2753;
    double k_2 = -0.1359;

    // Propeller interaction
    double t_P = 0.220;    // Thrust deduction factor
    double w_P0 = 0.400;   // Wake fraction at straight ahead
    double x_P_prime = -0.48; // Propeller position (fraction of L from midship, -ve = astern)

    // Rudder coefficients
    double t_R = 0.387;    // Steering resistance deduction
    double a_H = 0.312;    // Rudder force increase factor
    double x_H_prime = -0.464; // Point of application of additional lateral force
    double x_R_prime = -0.50;  // Rudder position (fraction of L from midship)
    double f_alpha = 2.747;    // Rudder lift gradient (dC_L/dalpha)
    double epsilon = 1.09;     // Ratio of wake fraction at propeller to rudder
    double kappa = 0.50;       // Correction factor for propeller-rudder interaction
    double l_R_prime = -0.710; // Distance from rudder to ship CG / L
    double gamma_R_positive = 0.640; // Flow straightening coeff (positive rudder)
    double gamma_R_negative = 0.395; // Flow straightening coeff (negative rudder)
    double A_R = 136.7;   // Rudder lateral area (m^2) - KVLCC2 from SIMMAN 2008

    // Estimate coefficients from ship dimensions using Kijima's formulae
    void estimateFromDimensions(const ShipDimensions& dims);
};

class MMGPhysicsModel : public IPhysicsModel {
public:
    MMGPhysicsModel();
    explicit MMGPhysicsModel(const ShipDimensions& dims);
    MMGPhysicsModel(const ShipDimensions& dims, const MMGCoefficients& coeffs);

    void step(double dt, const PhysicsInput& input, PhysicsState& state) override;
    const ShipDimensions& getDimensions() const override { return dims; }

    // Access coefficients for testing/tuning
    const MMGCoefficients& getCoefficients() const { return coeffs; }
    void setCoefficients(const MMGCoefficients& c) { coeffs = c; }

private:
    ShipDimensions dims;
    MMGCoefficients coeffs;

    static constexpr double RHO = 1025.0;  // Seawater density (kg/m^3)
    static constexpr double DEG_TO_RAD = 3.14159265358979323846 / 180.0;
    static constexpr double RAD_TO_DEG = 180.0 / 3.14159265358979323846;

    // Compute mass including added mass
    double getMassX() const;
    double getMassY() const;
    double getInertiaZ() const;

    // Force calculations (return forces/moments in Newtons/Newton-metres)
    void computeHullForces(double u, double v, double r,
                           double& Xh, double& Yh, double& Nh) const;
    void computePropellerForce(double u, double engineSetting,
                                double& Xp) const;
    void computeRudderForces(double u, double v, double r,
                              double rudderAngle, double engineSetting,
                              double& Xr, double& Yr, double& Nr) const;
};

#endif // __MMGPHYSICSMODEL_HPP_INCLUDED__
