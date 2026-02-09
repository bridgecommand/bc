# Bridge Command - Complete Upgrade Plan

**Branch:** `upgrade/graphics-and-simulation-overhaul`
**Date:** February 2026
**Version:** 5.10.4-alpha.4 (current)

---

> **CRITICAL CONSTRAINT:** This simulator runs on a real ship bridge setup: 2 PCs locally networked, 3 TV monitors as bridge windows, and 3 additional monitors for radar, helm repeater, and charts. **All upgrades MUST be backward-compatible with this multi-PC, multi-monitor configuration.** The Primary/Secondary networking between the 2 PCs must always work. Every change must be testable without breaking legacy functionality. When in doubt, preserve existing behavior and add new capabilities behind feature flags or abstractions.
>
> **CRITICAL CONSTRAINT:** OpenCPN map tiles must be loadable. The 3D rendered world MUST visually match the nautical chart data -- land, buildings, and most importantly **buoys** must be accurately placed. Scenarios must support planning with multiple ships (own + AI) that can span many map tiles.

---

## Table of Contents

1. [Current State Assessment](#1-current-state-assessment)
2. [Graphics Engine Evaluation](#2-graphics-engine-evaluation)
3. [Ship Simulation Physics Upgrade](#3-ship-simulation-physics-upgrade)
4. [Ocean/Water Rendering Upgrade](#4-oceanwater-rendering-upgrade)
5. [Radar Simulation Modernization](#5-radar-simulation-modernization)
6. [Audio System Upgrade](#6-audio-system-upgrade)
7. [VR/OpenXR Improvements](#7-vropenxr-improvements)
8. [Networking & Protocol Enhancements](#8-networking--protocol-enhancements)
9. [UI/GUI Modernization](#9-uigui-modernization)
10. [Build System & Code Modernization](#10-build-system--code-modernization)
11. [Research Required](#11-research-required)
12. [Phased Implementation Roadmap](#12-phased-implementation-roadmap)

---

## 1. Current State Assessment

### Architecture Overview

Bridge Command is a mature C++ maritime bridge simulator (~44K lines of code) using the **Irrlicht 3D engine** with the following subsystems:

| Subsystem | Technology | Files | Status |
|-----------|-----------|-------|--------|
| Graphics/Rendering | Irrlicht (OpenGL/DX9) | 295 files, 6,095 `irr::` refs | Aging, functional |
| Water/Ocean | FFT (Tessendorf/Keith Lantz), CPU-based | FFTWave.cpp, MovingWater.cpp | Good math, slow execution |
| Ship Physics | MMG standard + legacy empirical | OwnShip.cpp, MMGPhysicsModel.cpp | MMG done for conventional ships |
| Networking | ENet UDP + custom protocol | NetworkPrimary/Secondary | Solid |
| VR | OpenXR (Win/Linux) | VRInterface.cpp (1,841 lines) | Modern |
| Audio | PortAudio + libsndfile | Sound.cpp (6,287 lines) | Basic, no spatial audio |
| Radar | CPU software rendering + ARPA | RadarCalculation.cpp (1,947 lines) | Functional |
| GUI | Irrlicht built-in GUI | GUIMain.cpp (1,869 lines) | Legacy |
| Build | CMake 3.8+ | CMakeLists.txt | Adequate |
| Standard | C++17 | All files | Modern |

### Key Strengths
- Comprehensive maritime features (tidal streams, AIS, NMEA 0183, ARPA radar)
- Cross-platform (Windows/Linux/macOS)
- Good model variety (17 playable ships, 41 AI vessels, 35 buoy types, 6 worlds)
- VR support via modern OpenXR
- Dual primary/secondary networking for multi-station training
- Well-parameterized ship models via boat.ini

### Key Limitations
- Irrlicht engine is effectively unmaintained (last release 2021, OpenGL 2.x/3.x era)
- No Vulkan, no PBR rendering, no compute shaders
- CPU-based FFT ocean (bottleneck at N=64/128)
- ~~Simplified ship dynamics~~ -- MMG model now available (enable with `MMGMode=1` in boat.ini)
- No spatial/3D audio
- macOS OpenGL deprecated by Apple (frozen at GL 4.1)
- ~~C++11 standard~~ -- upgraded to C++17
- Monolithic SimulationModel class

---

## 2. Graphics Engine Evaluation

### Why Replace Irrlicht?
- Last official release: 2021, no active development
- No Vulkan/DX12/Metal support
- No PBR (Physically-Based Rendering)
- No compute shaders (critical for GPU ocean FFT)
- macOS OpenGL is deprecated with potential future removal
- Limited shader model support

### Engine Candidates Ranked for Maritime Simulation

#### Tier 1: Best Fit

| Engine | License | Water Quality | VR | API | Verdict |
|--------|---------|--------------|-----|-----|---------|
| **Wicked Engine** | MIT | Excellent (FFT ocean, foam, SSS, reflections) | Limited | Vulkan, DX12 | Best ocean rendering, C++17, single-dev risk |
| **O3DE** | Apache 2.0 | Good (Atom renderer, ocean component) | OpenXR Gem | Vulkan, DX12, Metal | Most future-proof, simulation-focused, steep learning curve |
| **Ogre3D Next (2.x)** | MIT | Moderate (shader-based, needs custom work) | Community plugin | Vulkan, GL, DX11, Metal | Easiest migration from Irrlicht, similar scene graph |

#### Tier 2: Viable Alternatives

| Engine | License | Fit | Notes |
|--------|---------|-----|-------|
| **Godot 4.x** | MIT | Medium | Excellent engine, but C++ requires GDExtension (not native workflow) |
| **bgfx** | BSD 2-clause | Medium | Rendering abstraction only, no scene graph/GUI -- everything DIY |
| **Filament** | Apache 2.0 | Medium | Google-backed PBR, but no VR, rendering-only |
| **Diligent Engine** | Apache 2.0 | Medium | Modern C++ API, but small community |

#### Tier 3: Not Recommended

| Engine | Why Not |
|--------|---------|
| **Unreal Engine 5** | Royalty model incompatible with GPL, forced editor workflow, massive codebase |
| **IrrlichtMt** | Minetest-focused fork, limited improvement over current Irrlicht |
| **Unity** | C# only, license issues |

### Commercial Middleware Option
**Triton Ocean SDK** (Sundog Software) could be integrated as drop-in water rendering middleware without a full engine change. Used by many professional maritime simulators. Supports OpenGL and Vulkan. Commercial license required.

### Recommended Strategy

Given the 6,095 `irr::` references across 295 files, a direct swap is impractical. The recommended approach:

1. **Create an abstraction layer** over Irrlicht types (ISceneNode, ISceneManager, IVideoDriver, IMesh, vector3df, etc.)
2. **Gradually migrate** code to use the abstraction instead of raw `irr::` types
3. **Implement the new backend** behind the abstraction (target: **Ogre3D Next** for lowest migration cost, or **Wicked Engine** for best result)
4. This mirrors the approach Minetest used when modernizing Irrlicht

### Research Needed
- [ ] Build and test Wicked Engine ocean demo on all target platforms
- [ ] Build and test Ogre3D Next with a basic maritime scene (ocean, sky, terrain, ship model)
- [ ] Evaluate Ogre3D Next scene graph mapping to Irrlicht scene graph (document 1:1 equivalents)
- [ ] Test MoltenVK (Vulkan on macOS) performance with ocean rendering
- [ ] Evaluate Triton Ocean SDK licensing terms and integration complexity
- [ ] Profile current Irrlicht rendering to identify specific bottlenecks

---

## 3. Ship Simulation Physics Upgrade

### Current Model Limitations

The current model (`OwnShip.cpp`) uses simplified empirical forces:
- Quadratic drag: `F = A*v^2 + B*v` (too simplified for varying hull forms)
- Simple thrust model: `thrust = engine_setting * maxForce`
- Basic rudder: `torque = rudder_angle * (speed * A + thrust * B)`
- No added mass/inertia (real ships have 50-200% additional hydrodynamic mass)
- No hull-propeller-rudder interaction
- No shallow water effects, bank effects, or ship-ship interaction
- Roll/pitch from simplified sinusoidal model, not wave excitation

### Target: MMG Standard Maneuvering Model

The MMG (Maneuvering Modeling Group) model is the industry standard used by Kongsberg, Wartsila, and VSTEP. It decomposes forces as:

```
F_total = F_hull + F_propeller + F_rudder + F_external
```

**Hull forces** use hydrodynamic derivatives:
```
X_hull = X_uu * u'^2 + X_vv * v'^2 + X_rr * r'^2 + X_vr * v' * r'
Y_hull = Y_v * v' + Y_r * r' + Y_vvv * v'^3 + Y_rrr * r'^3
N_hull = N_v * v' + N_r * r' + N_vvv * v'^3 + N_rrr * r'^3
```

**Propeller model** includes:
- Open-water characteristics (KT, KQ curves)
- Wake fraction and thrust deduction
- Hull-propeller interaction coefficients

**Rudder model** includes:
- Effective inflow velocity from propeller slipstream
- Hull-rudder interaction (flow straightening)
- Non-linear lift characteristics

### Upgrade Steps (Priority Order)

#### Phase 1: Quick Wins
1. **Add added mass/inertia** -- Clarke's formulae: `m_y_added ~ pi * rho * T^2 * L / 2`. Minimal code change, significant accuracy improvement.
2. **Replace Phillips spectrum with JONSWAP** in FFTWave.cpp -- change the spectral function only, more realistic wind-driven seas.
3. **Couple wind to waves** -- auto-derive wave parameters from wind speed/direction instead of separate weather parameter.

#### Phase 2: Core Physics Upgrade
4. **Implement MMG hull force model** -- replace quadratic drag with hydrodynamic derivative-based forces.
5. **Implement proper rudder model** -- account for propeller slipstream effect on rudder effectiveness.
6. **Fixed-timestep physics loop** -- decouple physics from rendering timestep (target 50Hz physics). Currently `OwnShip::update()` uses rendering deltaTime directly.
7. **Wave-ship interaction forces** -- sample wave heights at hull contact points to compute pitch/roll/heave excitation.

#### Phase 3: Advanced Features
8. **Shallow water effects** -- modify hydrodynamic coefficients based on depth/draught ratio (Lackenby/Barras squat, Ankudinov modified derivatives).
9. **Bank effects** -- Norrbin model for suction force and yaw moment near channel banks.
10. **Ship-ship interaction** -- passing/overtaking forces and moments.
11. **Improved wind forces** -- Isherwood or Blendermann methods for hull/superstructure wind coefficients.

### Ship Model Data Format Upgrade

Current `boat.ini` needs expansion for MMG model. New parameters needed:
```ini
# Hydrodynamic derivatives (non-dimensional)
X_uu, X_vv, X_rr, X_vr
Y_v, Y_r, Y_vvv, Y_rrr, Y_vvr, Y_vrr
N_v, N_r, N_vvv, N_rrr, N_vvr, N_vrr

# Added mass coefficients
m_x_added, m_y_added, J_z_added

# Propeller characteristics
WakeFraction, ThrustDeduction, KT_coefficients, KQ_coefficients

# Rudder characteristics
RudderArea, RudderAspectRatio, FlowStraighteningCoeff
```

### Research Needed
- [ ] Source published MMG coefficients for common vessel types (tanker, container, bulk carrier, tug)
- [ ] Review Yasukawa & Yoshimura (2015) "Introduction of MMG Standard Method" paper
- [ ] Study Fossen's "Handbook of Marine Craft Hydrodynamics and Motion Control" for 6-DOF model
- [ ] Evaluate Capytaine (Python BEM solver) for pre-computing added mass/damping coefficients
- [ ] Review VRX/Gazebo maritime plugins buoyancy code as reference implementation
- [ ] Determine if existing boat.ini parameter sets can be converted to MMG coefficients

---

## 4. Ocean/Water Rendering Upgrade

### Current Implementation
- `FFTWave.cpp`: CPU-based FFT using Phillips spectrum, N=64 or N=128 grid
- `MovingWater.cpp`: Irrlicht scene node with shader-based reflections
- Shaders: GLSL + HLSL, basic Fresnel reflections, optional planar reflection
- Tiled water approach for infinite ocean appearance

### Upgrade Path

#### Phase 1: Improve Existing System
1. **Switch Phillips to JONSWAP spectrum** -- change spectral function in `cOcean::phillipsSpectrum()`, more realistic fetch-limited seas
2. **Improve water shaders** -- add PBR subsurface scattering, foam from Jacobian, Fresnel-correct reflections
3. **Add screen-space reflections (SSR)** as alternative to planar reflections (better performance)

#### Phase 2: GPU Acceleration
4. **Move FFT to GPU compute shaders** -- enables 2048x2048 or 4096x4096 grids at 60+ fps vs. current CPU bottleneck at 64x64
5. **Multi-cascade ocean** -- 3-4 FFT grids at different spatial scales (100km swells to 10cm capillary waves)
6. **GPU-generated foam** -- compute foam coverage from wave Jacobian in compute shader

#### Phase 3: Advanced Features
7. **Ship wake simulation** -- Kelvin wake pattern behind own ship and other ships
8. **Bow wave / spray particles** -- particle effects at ship-water intersection
9. **Atmospheric scattering** -- for realistic horizon/sky-water boundary

### Research Needed
- [ ] Profile current CPU FFT performance (identify exact bottleneck)
- [ ] Study Wicked Engine's ocean implementation as reference (MIT license, can study code)
- [ ] Research Tessendorf multi-cascade approach for scale separation
- [ ] Evaluate JONSWAP spectrum parameters for Beaufort scale mapping
- [ ] Study ship wake rendering techniques (Kelvin wave pattern)

---

## 5. Radar Simulation Modernization

### Current Implementation
- CPU software rendering using scan arrays
- Angular and range resolution configurable
- ARPA/MARPA target tracking
- Gain, sea clutter, rain clutter controls
- EBL/VRM/cursor controls, parallel index lines
- Head-up / Course-up / North-up modes

### Upgrade Path

#### Phase 1: Performance
1. **GPU-accelerated radar rendering** -- move scan computation to GPU ray-casting
2. **Render-to-texture radar display** -- use GPU to directly render radar image

#### Phase 2: Realism
3. **Material-based RCS (Radar Cross Section)** -- different return strengths for steel, fiberglass, land, sea surface
4. **Radar shadow computation** -- shadows behind land masses and large vessels
5. **Sea clutter model upgrade** -- replace basic noise with K-distribution or compound Gaussian models

#### Phase 3: Advanced
6. **Dual S-band/X-band radar** -- two radars at different frequencies (3 GHz vs 9.4 GHz)
7. **Chart radar overlay** -- radar image overlaid on electronic chart display

### Research Needed
- [ ] Profile current radar rendering performance
- [ ] Research GPU ray-casting approaches for radar simulation
- [ ] Study RCS values for common maritime materials
- [ ] Review IALA standards for radar simulation in training

---

## 6. Audio System Upgrade

### Current State
- PortAudio + libsndfile
- 4 sound streams: engine, waves, horn, alarm
- Simple mixing with volume control
- No 3D spatial audio
- No Doppler effect, no environmental effects

### Upgrade Path

1. **Replace with OpenAL Soft** -- open source 3D audio library, supports HRTF for VR
2. **Spatial audio** -- position sounds in 3D space (other ships' horns, engine, waves around hull)
3. **HRTF support for VR** -- critical for immersive VR bridge experience
4. **Environmental audio effects** -- reverb in enclosed harbors, wind noise variation
5. **Doppler effect** -- for approaching/departing vessels
6. **Engine sound synthesis** -- RPM-linked engine tone variation instead of static loop

### Research Needed
- [ ] Evaluate OpenAL Soft vs FMOD vs Wwise (licensing for GPL project)
- [ ] Research HRTF profiles for OpenXR VR integration
- [ ] Study maritime audio simulation in commercial simulators

---

## 7. VR/OpenXR Improvements

### Current State
- OpenXR integration in VRInterface.cpp (1,841 lines)
- Stereo rendering, controller tracking, eye tracking (optional)
- macOS disabled (no OpenXR runtime)
- Renders through Irrlicht's OpenGL backend

### Upgrade Path

1. **Vulkan rendering path for VR** -- lower overhead, better VR performance
2. **Hand tracking UI interaction** -- use hand tracking for bridge controls
3. **Passthrough AR mode** -- for mixed reality training setups
4. **Multi-view rendering (quad-view)** -- already partially supported, optimize
5. **VR-specific UI** -- 3D instrument panels, virtual binoculars
6. **Haptic feedback** -- for throttle/rudder feel via controllers

### Research Needed
- [ ] Test Vulkan VR rendering performance vs current OpenGL path
- [ ] Evaluate Meta Quest / Pico / PCVR performance targets
- [ ] Research passthrough API in OpenXR (XR_FB_passthrough extension)

---

## 8. Networking & Protocol Enhancements

### Current State
- ENet UDP networking (primary/secondary model)
- NMEA 0183 output (serial + UDP): RMC, GLL, GGA, HDT, ROT, DPT, VHW, MWV, etc. (16 sentence types)
- AIS Class A position reports (Message 1, ITU-R M.1371-5)
- AIS Message 5 static/voyage data (ship name, dimensions, every 6 min)
- AIS Message 21 Aid-to-Navigation reports (buoy positions, every 3 min)
- Multiplayer hub for multi-station exercises

### Upgrade Path

#### NMEA Enhancements

1. ~~**VHW (water speed/heading)**~~ -- Done
2. ~~**MWV (wind speed/direction)**~~ -- Done
3. **Add remaining sentences**: VDR (set & drift), MTW (water temperature)
4. **NMEA 0183 input expansion** -- accept more sentence types for external control

#### AIS Enhancements

1. ~~**AIS Message 5**~~ -- Done (ship name, dimensions, ship type, GPS EPFD)
2. **AIS Message 18** -- Class B position reports for pleasure craft
3. ~~**AIS Message 21**~~ -- Done (buoy AtoN reports with position)

#### Network Architecture

1. **WebSocket protocol** -- for web-based instructor stations
2. **Network encryption** -- TLS for deployment on shared networks
3. **Cloud deployment support** -- containerized instructor station

### Research Needed
- [ ] Survey NMEA sentence requirements for common ECDIS software integration
- [ ] Review IEC 61162-1 standard for complete sentence coverage
- [ ] Evaluate WebSocket libraries compatible with GPL (e.g., libwebsockets)

---

## 9. UI/GUI Modernization

### Current State
- Built with Irrlicht's internal GUI system
- 2D instrument panel rendered over 3D view
- Compass, speed, radar, engine/RPM, rudder, depth displays
- Separate launcher, editor, controller, repeater applications

### Upgrade Path

1. **Adopt Dear ImGui** -- immediate-mode GUI, perfect for simulation overlays, MIT license
2. **OpenBridge Design System** -- follow Norwegian maritime UI standards for bridge displays
3. **Modernize instrument rendering** -- vector-based instruments instead of bitmap
4. **Resizable/configurable instrument layout** -- user-customizable bridge panel
5. **ECDIS display integration** -- built-in basic chart display
6. **Touch-screen support** -- for tablet-based control stations

### Research Needed
- [ ] Review OpenBridge design guidelines (https://www.openbridge.no/)
- [ ] Evaluate Dear ImGui integration with candidate graphics engines
- [ ] Study ECDIS display requirements (IEC 61174)

---

## 10. Build System & Code Modernization

### Current State
- CMake 3.8+, C++11 standard
- Bundled libraries: Irrlicht, ENet 1.3.14, ASIO, PortAudio, libsndfile, OpenXR SDK, serial
- Platforms: Windows (MSVC/MinGW), Linux, macOS (arm64/x86_64)

### Upgrade Path

1. **Upgrade to C++17** -- structured bindings, std::filesystem, std::optional, if constexpr
2. **Modern CMake practices** -- target-based configuration, FetchContent for dependencies
3. **Package management** -- vcpkg or Conan for third-party libraries
4. **CI/CD pipeline** -- GitHub Actions for automated builds on all platforms
5. **Static analysis** -- clang-tidy, cppcheck integration
6. **Unit testing framework** -- Catch2 or GoogleTest for physics model validation
7. **Refactor SimulationModel** -- break monolithic class into subsystems:
   - DynamicsEngine (ship physics)
   - EnvironmentEngine (weather, tides, currents)
   - InstrumentEngine (radar, NMEA, AIS)
   - TrafficEngine (other ships, buoys)

### Research Needed
- [ ] Audit all third-party library versions for updates
- [ ] Evaluate vcpkg vs Conan for cross-platform package management
- [ ] Design subsystem interfaces for SimulationModel refactoring

---

## 11. Research Required

### Critical Path Research (Must Do First)

| # | Topic | Purpose | Method |
|---|-------|---------|--------|
| R1 | Build & test Wicked Engine ocean demo | Validate water rendering quality | Build from source, test on Win/Linux/macOS |
| R2 | Build & test Ogre3D Next basic scene | Validate migration feasibility | Create maritime scene prototype |
| R3 | Map Irrlicht to Ogre3D API equivalents | Plan abstraction layer | Code review, document 1:1 mappings |
| R4 | Source MMG coefficients for vessel types | Enable physics upgrade | Literature review, Yasukawa & Yoshimura 2015 |
| R5 | Profile current rendering performance | Identify real bottlenecks | GPU/CPU profiling on target platforms |
| R6 | Evaluate Triton Ocean SDK | Potential quick-win water upgrade | Contact Sundog Software, get eval license |

### Secondary Research

| # | Topic | Purpose |
|---|-------|---------|
| R7 | MoltenVK performance testing | Validate macOS Vulkan path |
| R8 | Fossen 6-DOF model study | Advanced physics reference |
| R9 | Capytaine BEM solver evaluation | Pre-compute hydrodynamic coefficients |
| R10 | OpenAL Soft GPL compatibility | Audio engine licensing |
| R11 | OpenBridge design system review | UI modernization reference |
| R12 | VRX/Gazebo buoyancy code review | Reference physics implementation |

### Information Gathering

| # | Topic | Sources |
|---|-------|---------|
| R13 | Wicked Engine GitHub issues/roadmap | https://github.com/turanszkij/WickedEngine |
| R14 | O3DE maritime/simulation use cases | https://github.com/o3de/o3de |
| R15 | Modern ship simulator architecture papers | IEEE, MARSIM conference proceedings |
| R16 | GPU FFT ocean implementations | NVIDIA samples, Shadertoy references |
| R17 | IALA radar simulation standards | IALA guidelines |

---

## 12. Phased Implementation Roadmap

### Phase 0: Foundation (Months 1-2)
**Goal:** Prepare codebase for modernization without breaking anything

- [x] Upgrade to C++17
- [x] Add CI/CD pipeline (GitHub Actions) -- with test step on Linux amd64/arm64
- [x] Add unit testing framework (Catch2) -- 68 tests, 184 assertions
- [ ] Profile rendering and physics performance (establish baselines) -- needs running on bridge hardware
- [x] Complete critical path research (R1-R6) -- see RESEARCH_FINDINGS.md

### Phase 1: Quick Physics Wins (Months 2-4)
**Goal:** Significantly improve simulation realism with minimal code changes

- [x] Implement MMG maneuvering model (hull, propeller, rudder forces)
- [x] Add missing NMEA sentences (VHW, MWV)
- [x] Add AIS Message 5 (static/voyage data), Message 21 (AtoN buoys)
- [ ] Replace Phillips spectrum with JONSWAP in FFTWave.cpp
- [ ] Couple wind speed to wave parameters
- [ ] Add AIS Message 18 (Class B)

### Phase 2: Graphics Abstraction Layer (Months 4-8)
**Goal:** Decouple codebase from Irrlicht, enabling engine swap

- [x] Design abstraction types: `bc::graphics::Vec2/Vec3/Color/Matrix4/Quaternion`
- [x] Migrate 39+ headers from `#include "irrlicht.h"` to abstraction types + forward declarations
- [ ] Migrate remaining 17 headers (blocked: 9 EventReceivers need event abstraction, 4 GUI files deferred to ImGui, 3 core engine deeply coupled, 1 RadarCalculation deferred)
- [ ] Implement Irrlicht backend for full abstraction (verify no regression)
- [ ] Begin implementing new engine backend (Wicked Engine)

### Phase 3: Core Physics Upgrade (Months 4-8, parallel with Phase 2)
**Goal:** Implement MMG standard maneuvering model

- [x] Implement MMG hull force model with hydrodynamic derivatives (Kijima estimation)
- [x] Implement proper propeller model (KT/KQ, wake fraction, thrust deduction)
- [x] Implement propeller-slipstream rudder model
- [x] Expand boat.ini format for new coefficients (MMGMode=1)
- [ ] Wave-ship interaction forces (sample wave field at hull points)
- [ ] Validate against published turning circle / zigzag test data

### Phase 4: New Rendering Engine (Months 8-14)
**Goal:** Complete engine migration, GPU-accelerated rendering

- [ ] Complete new engine backend implementation
- [ ] GPU-based FFT ocean rendering (compute shaders)
- [ ] Multi-cascade ocean (3-4 FFT grids at different scales)
- [ ] PBR water shading (subsurface scattering, foam, caustics)
- [ ] Improved sky rendering (atmospheric scattering)
- [ ] Ship wake simulation (Kelvin wake patterns)
- [ ] GPU-accelerated radar rendering

### Phase 5: Audio & VR (Months 10-14, parallel with Phase 4)
**Goal:** Immersive audio and improved VR experience

- [ ] Replace PortAudio with OpenAL Soft (3D spatial audio)
- [ ] Implement HRTF for VR
- [ ] Add spatial sound positioning (other ships, waves, horns)
- [ ] Vulkan rendering path for VR
- [ ] VR hand tracking for bridge controls
- [ ] Haptic feedback integration

### Phase 6: Advanced Features (Months 14-18)
**Goal:** Professional training-grade features

- [ ] Shallow water effects
- [ ] Bank effects (Norrbin model)
- [ ] Ship-ship interaction forces
- [ ] Dual S-band/X-band radar
- [ ] UI modernization (Dear ImGui or OpenBridge)
- [ ] ECDIS display integration
- [ ] Multi-directional seas (wind-sea + swell)
- [ ] Volumetric fog rendering

### Phase 7: Polish & Release (Months 18-20)
**Goal:** Bridge Command 6.0

- [ ] Performance optimization pass
- [ ] Cross-platform testing (Windows, Linux, macOS)
- [ ] VR headset compatibility testing
- [ ] Documentation update
- [ ] Ship model coefficient database for all existing vessels
- [ ] New scenarios showcasing upgraded features
- [ ] Community beta testing

---

## Appendix A: Key References

### Ship Physics
- Yasukawa & Yoshimura (2015), "Introduction of MMG Standard Method for Ship Maneuvering Predictions", J. Marine Science and Technology
- Fossen, T.I., "Handbook of Marine Craft Hydrodynamics and Motion Control", Wiley
- Clarke et al., "The Application of Manoeuvring Criteria in Hull Design"
- MSS (Marine Systems Simulator): https://github.com/cybergalactic/MSS

### Graphics Engines
- Wicked Engine: https://github.com/turanszkij/WickedEngine
- O3DE: https://github.com/o3de/o3de
- Ogre3D Next: https://github.com/OGRECave/ogre-next
- bgfx: https://github.com/bkaradzic/bgfx
- Filament: https://github.com/google/filament

### Ocean Rendering
- Tessendorf, J. (2001), "Simulating Ocean Water"
- Triton Ocean SDK: https://sundog-soft.com/triton/

### Physics Libraries
- Project Chrono: https://projectchrono.org/
- Capytaine (BEM solver): https://github.com/capytaine/capytaine
- Jolt Physics: https://github.com/jrouwe/JoltPhysics

### Maritime Simulation
- VRX/Gazebo maritime plugins: https://github.com/osrf/vrx
- OpenBridge design system: https://www.openbridge.no/
- OpenCPN: https://opencpn.org/

### Standards
- NMEA 0183: IEC 61162-1
- AIS: ITU-R M.1371-5
- ECDIS: IEC 61174
- STCW: IMO Convention on Standards of Training

---

## Appendix B: Risk Assessment

| Risk | Impact | Likelihood | Mitigation |
|------|--------|-----------|------------|
| Engine migration too complex | High | Medium | Abstraction layer approach allows incremental migration |
| MMG coefficients unavailable for all ship types | Medium | Low | Fallback to simplified model per vessel; use generic coefficients |
| Wicked Engine single-developer risk | Medium | Medium | O3DE as backup option; abstraction layer enables switching |
| macOS Vulkan (MoltenVK) issues | Low | Low | Keep OpenGL fallback; test early |
| Performance regression during migration | Medium | Medium | Maintain parallel backends; benchmark continuously |
| Breaking changes in OpenXR | Low | Low | Current implementation is standard-compliant |
| GPL compatibility with chosen libraries | High | Low | All recommended libraries have compatible licenses (MIT/Apache/BSD) |

---

## Appendix C: File Count by Subsystem (Migration Effort Estimate)

| Subsystem | Core Files | Irrlicht Coupling | Migration Effort |
|-----------|-----------|-------------------|-----------------|
| Ship Physics (OwnShip, Ship, OtherShips) | 8 | Medium (vectors, scene nodes) | Medium |
| Water/Ocean (FFTWave, MovingWater) | 4 | High (scene node, shaders) | High |
| Terrain | 6 | Very High (custom scene nodes) | Very High |
| Radar | 4 | Medium (texture rendering) | Medium |
| GUI/Instruments | 12 | Very High (GUI widgets) | Very High |
| Camera/Viewport | 2 | High (camera nodes) | High |
| Lighting/NavLights | 4 | High (light nodes, billboards) | High |
| Sky/Weather/Rain | 4 | High (skybox, particles) | High |
| VR Interface | 2 | High (rendering pipeline) | High |
| Networking | 6 | Low (data only) | Low |
| NMEA/AIS | 4 | None | None |
| Scenario/Config | 6 | Low | Low |
| Main Loop | 1 | Very High | High |
| **Total** | **~63 core files** | | |
