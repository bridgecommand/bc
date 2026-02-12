# Wicked Engine Ocean Analysis

**Phase 3-01 Analysis Document**
**Analyzed version:** Wicked Engine 0.72.33 (Feb 2026)
**Files:** `wiOcean.h`, `wiOcean.cpp`, `wiFFTGenerator.h/cpp`

## Overview

Wicked Engine includes a single-cascade, Phillips spectrum GPU ocean with displacement maps. The implementation is clean but basic compared to modern multi-cascade approaches.

## Spectrum Model

**Phillips Spectrum (only)**
- Formula: `P(K) = a * exp(-1/(l^2*K^2)) / K^6 * (K . windDir)^2`
- Largest wavelength: `l = v^2/g` where v = wind speed
- Short wave damping: `w = l/1000`
- Directional dependency: waves opposing wind direction are damped by `wind_dependency` factor
- Gaussian random initialization via Box-Muller transform
- No JONSWAP, TMA, or Donelan-Banner spectrum options

## FFT System

**Single 512x512 C2C FFT**
- Fixed resolution: `dmap_dim = 512` (must be power-of-2)
- One cascade only (no multi-scale)
- 3 interleaved datasets per FFT: H(t), Dx(t), Dy(t)
- Dispatched via `wi::fftgenerator::fft_512x512_c2c()`
- Patch size: 50m world units (default)

## GPU Pipeline (4 stages)

1. **Spectrum Update** (`updateSpectrumCS`): Time-evolve H(0) to H(t) using dispersion relation
2. **FFT Transform** (`fft_512x512_c2c`): Frequency → spatial domain
3. **Displacement Texture** (`updateDisplacementMapCS`): Pack into R32G32B32A32_FLOAT texture
4. **Gradient/Folding** (`updateGradientFoldingCS`): Surface normals with full mip chain

## Textures Output

| Texture | Format | Contents |
|---------|--------|----------|
| `displacementMap` | RGBA32F | Dx, Dz, Dy displacement + height |
| `gradientMap` | RGBA16F | Surface normal derivatives, full mip chain |

## Ocean Mesh

- Dynamic grid generated each frame (not pre-baked)
- Base: 160 x 90 quads, scaled by `surfaceDetail` (1-8)
- Default detail=4: 640 x 360 = 230,400 vertices
- 32-bit indices for high detail, 16-bit for occlusion test pass
- Separate low-res mesh (80 x 45) for occlusion queries

## CPU Height Readback

- Triple-buffered GPU→CPU readback textures
- 2-3 frame latency (avoids GPU stall)
- `GetDisplacedPosition(worldPos)` returns displaced position
- Bilinear interpolation on readback texture data
- Coordinate swizzle: displacement stored as (Dx, Dz, Dy) → applied as (X, Y, Z)

## Exposed Parameters (OceanParameters)

| Parameter | Default | Description |
|-----------|---------|-------------|
| `dmap_dim` | 512 | FFT resolution (power of 2) |
| `patch_length` | 50.0 | Tile size in world units |
| `time_scale` | 0.3 | Animation speed multiplier |
| `wave_amplitude` | 1000 | Overall height scaling |
| `wind_speed` | 600 | Wind speed (cm/s) |
| `wind_dir` | (0.8, 0.6) | Wind direction vector |
| `wind_dependency` | 0.07 | Directional damping |
| `choppy_scale` | 1.3 | Horizontal displacement multiplier |
| `waterColor` | (0, 0.008, 0.024, 0.6) | Diffuse water color |
| `extinctionColor` | (0, 0.9, 1, 1) | Underwater light extinction |
| `waterHeight` | 0 | Base water plane height |
| `surfaceDetail` | 4 | Mesh density multiplier |

## What's Missing (Phase 3 upgrade targets)

1. **Multi-cascade FFT** - Only 1 cascade; need 3+ for large swells + small ripples
2. **JONSWAP spectrum** - Only Phillips; need JONSWAP for realistic sea states
3. **Foam system** - No foam generation, no wave crest detection
4. **Temporal foam decay** - Not implemented
5. **Kelvin wake** - No ship wake rendering
6. **Spectral LOD** - No distance-based spectral reduction

## Architecture Assessment

**Strengths:**
- Clean separation: simulation → displacement → rendering
- Efficient GPU pipeline (4 compute passes + 1 draw)
- Built-in CPU readback for physics
- Gradient mip chain for multi-scale normals

**Weaknesses:**
- Single cascade limits visual range (50m repeat)
- Phillips spectrum lacks parameterization for real sea states
- No foam reduces visual realism
- Fixed 512x512 resolution (no adaptive)

## Phase 3 Plan

Based on this analysis, Phase 3 should:

1. **3-02**: Extend to 3-cascade FFT (512x512 each at different scales: 50m, 500m, 5000m)
2. **3-03**: Add JONSWAP spectrum as compute shader (port from Phase 1A CPU implementation)
3. **3-04**: Add Jacobian-based foam detection from displacement derivatives
4. **3-05**: Add temporal foam persistence texture with decay
5. **3-06**: Optimize CPU readback for ship physics (async, reduced resolution)
6. **3-07**: Add Kelvin wake via displacement injection
7. **3-08**: Performance benchmark: GPU time target < 3ms for 3 cascades
