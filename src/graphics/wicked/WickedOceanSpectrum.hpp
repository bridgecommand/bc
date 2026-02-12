/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2026 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation */

#ifndef BC_GRAPHICS_WICKED_OCEAN_SPECTRUM_HPP
#define BC_GRAPHICS_WICKED_OCEAN_SPECTRUM_HPP

#ifdef WITH_WICKED_ENGINE

#include <cmath>
#include <vector>

namespace bc { namespace graphics { namespace wicked {

/// Ocean wave spectrum models for initializing FFT ocean simulation.
///
/// Provides JONSWAP and TMA spectra as alternatives to WE's built-in
/// Phillips spectrum. These are used during H(0) initialization to
/// produce more physically accurate wave fields.
///
/// Phase 3-03 of the upgrade plan.
namespace OceanSpectrum {

    static constexpr float G = 9.81f;       // Gravity (m/s^2)
    static constexpr float G_CM = 981.0f;   // Gravity (cm/s^2) - WE convention
    static constexpr float PI = 3.14159265358979f;

    /// Phillips spectrum (WE default).
    /// @param k2 Squared wave number magnitude |k|^2
    /// @param kDotW Dot product of wave vector with wind direction
    /// @param windSpeed Wind speed (cm/s, WE convention)
    /// @param amplitude Amplitude constant A
    /// @param dirDepend Directional dependency (0-1, filters waves against wind)
    inline float Phillips(float k2, float kDotW, float windSpeed, float amplitude, float dirDepend) {
        if (k2 < 1e-12f) return 0.0f;
        float L = windSpeed * windSpeed / G_CM;
        float damping = L / 1000.0f;
        float phil = amplitude * std::exp(-1.0f / (L * L * k2)) / (k2 * k2 * k2) * (kDotW * kDotW);
        if (kDotW < 0) phil *= dirDepend;
        return phil * std::exp(-k2 * damping * damping);
    }

    /// JONSWAP spectrum (Joint North Sea Wave Project).
    /// More physically accurate than Phillips for fetch-limited seas.
    ///
    /// S(omega) = alpha * g^2 / omega^5 * exp(-beta * (omega_p/omega)^4) * gamma^r
    ///
    /// @param omega Angular frequency (rad/s)
    /// @param windSpeedMps Wind speed at 10m height (m/s)
    /// @param fetchKm Fetch length (km). Default 100km (moderate open sea).
    /// @param gamma Peak enhancement factor (default 3.3 for JONSWAP)
    /// @return Spectral density S(omega) in m^2*s
    inline float JONSWAP(float omega, float windSpeedMps, float fetchKm = 100.0f, float gamma = 3.3f) {
        if (omega < 1e-6f) return 0.0f;

        float fetchM = fetchKm * 1000.0f;

        // Non-dimensional fetch
        float fetchTilde = G * fetchM / (windSpeedMps * windSpeedMps);

        // Peak frequency from JONSWAP empirical relation
        float omega_p = 22.0f * std::pow(G * G / (windSpeedMps * fetchM), 1.0f / 3.0f);

        // Phillips constant (alpha) from JONSWAP
        float alpha = 0.076f * std::pow(fetchTilde, -0.22f);

        // Standard JONSWAP beta parameter
        float beta = 1.25f;

        // Spectral width parameter
        float sigma = (omega <= omega_p) ? 0.07f : 0.09f;

        // Peak enhancement
        float r_exp = -((omega - omega_p) * (omega - omega_p)) /
                      (2.0f * sigma * sigma * omega_p * omega_p);
        float r = std::exp(r_exp);

        // Pierson-Moskowitz base spectrum
        float pm = alpha * G * G / std::pow(omega, 5.0f) *
                   std::exp(-beta * std::pow(omega_p / omega, 4.0f));

        return pm * std::pow(gamma, r);
    }

    /// Convert JONSWAP spectral density to H(0) amplitude for FFT initialization.
    /// This matches WE's H(0) format: complex amplitude with Gaussian random modulation.
    ///
    /// @param kx Wave number x component
    /// @param kz Wave number z component
    /// @param windSpeedMps Wind speed (m/s)
    /// @param windDirX Normalized wind direction X
    /// @param windDirZ Normalized wind direction Z
    /// @param patchLength Size of FFT patch (meters)
    /// @param fetchKm Fetch distance (km)
    /// @param gamma JONSWAP gamma parameter
    /// @return Spectral amplitude sqrt(S(omega) * dOmega) for this wave number
    inline float JONSWAPAmplitude(float kx, float kz, float windSpeedMps,
                                    float windDirX, float windDirZ,
                                    float patchLength, float fetchKm = 100.0f,
                                    float gamma = 3.3f) {
        float k2 = kx * kx + kz * kz;
        if (k2 < 1e-12f) return 0.0f;

        float k = std::sqrt(k2);

        // Dispersion relation: omega^2 = g*k (deep water)
        float omega = std::sqrt(G * k);

        // JONSWAP spectral density
        float S = JONSWAP(omega, windSpeedMps, fetchKm, gamma);

        // Directional spreading: cosine-squared model
        // cos^2(theta - theta_wind) where theta is wave direction
        float kNormX = kx / k;
        float kNormZ = kz / k;
        float cosTheta = kNormX * windDirX + kNormZ * windDirZ;
        float directional = (cosTheta > 0) ? cosTheta * cosTheta : 0.0f;

        // Convert spectral density to amplitude
        // dk = (2*pi / patchLength)^2 for 2D FFT discretization
        float dk = (2.0f * PI / patchLength);
        float dOmega = 0.5f * G / (omega * k); // Jacobian: dOmega/dk = g/(2*omega)

        return std::sqrt(2.0f * S * directional * dOmega * dk * dk);
    }

    /// TMA spectrum (Texel-MARSEN-ARSLOE) for shallow water.
    /// Applies a depth-dependent transfer function to JONSWAP.
    ///
    /// @param omega Angular frequency
    /// @param windSpeedMps Wind speed (m/s)
    /// @param depth Water depth (meters)
    /// @param fetchKm Fetch (km)
    /// @return Spectral density S_TMA(omega)
    inline float TMA(float omega, float windSpeedMps, float depth, float fetchKm = 100.0f) {
        float S_jonswap = JONSWAP(omega, windSpeedMps, fetchKm);

        // Kitaigorodskii depth function
        float omega_h = omega * std::sqrt(depth / G);
        float phi;
        if (omega_h <= 1.0f) {
            phi = 0.5f * omega_h * omega_h;
        } else if (omega_h < 2.0f) {
            phi = 1.0f - 0.5f * (2.0f - omega_h) * (2.0f - omega_h);
        } else {
            phi = 1.0f;
        }

        return S_jonswap * phi;
    }

} // namespace OceanSpectrum

}}} // namespace bc::graphics::wicked

#endif // WITH_WICKED_ENGINE
#endif // BC_GRAPHICS_WICKED_OCEAN_SPECTRUM_HPP
