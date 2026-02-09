# Bridge Command Simulator Upgrade - Research Findings

**Date**: February 8, 2026
**Project Context**: GPL v2 C++ maritime bridge simulator (Irrlicht-based)
**Hardware**: 2 networked PCs, 3 bridge monitors, 3 auxiliary displays (radar/helm/charts)
**Critical Constraint**: All upgrades must maintain backward compatibility with existing scenarios and hardware setup

---

## Executive Summary

This document synthesizes findings from 7 parallel research agents evaluating replacement graphics engines, ocean simulation systems, physics models, and audio systems for Bridge Command's upgrade path.

### Key Findings

1. **Graphics Engine**: Wicked Engine and Ogre3D Next are viable successors to Irrlicht, each with distinct tradeoffs
2. **Ocean Simulation**: No perfect turnkey solution; GPU FFT implementation recommended using compute shaders
3. **Physics**: MMG (Maneuvering Modelling Group) equations available with published KVLCC2 coefficients; major improvement over current quadratic drag
4. **Audio**: OpenAL Soft is the only GPL-compatible spatial audio solution with HRTF support
5. **License Compatibility**: Triton SDK is a dead-end (proprietary); focus must be on open-source solutions

### Recommended Action Plan

**Phase 1 (Validate)**:
- Wicked Engine GPU FFT ocean prototype (2-3 weeks)
- OpenAL Soft integration (1 week)
- MMG physics proof-of-concept (1 week)

**Phase 2 (Evaluate)**:
- Full multi-cascade FFT with foam/spray (2-3 weeks)
- Render loop refactor to maintain multi-monitor support (2 weeks)

**Phase 3 (Decide)**:
- If Wicked Engine meets requirements → migrate forward-rendering + ECS architecture
- If performance/VR gaps persist → consider Ogre3D Next as alternative path

---

## 1. Graphics Engine Analysis

### 1.1 Wicked Engine

**Status**: v0.72.27 (February 5, 2026) - **Very actively maintained**
- 4,940 commits, 56 contributors, 6,800+ stars
- Weekly updates; last commit recent

**Architecture**:
- ECS (Entity-Component-System) with ~35+ component types
- Superior to Irrlicht's scene graph for simulation scenarios
- Native support for physics simulation decoupling

**Platform Support**:
| Platform | Support | Notes |
|----------|---------|-------|
| Windows | DX12 | Full native |
| Linux | Vulkan | Full native |
| macOS | Metal 4 | Native (NOT MoltenVK) |
| Console | Yes | Additional platforms available |

**Rendering Features**:
- Forward rendering (not deferred)
- Screen-space effects suitable for bridge console displays
- HDR capable

**Strengths**:
- MIT license (fully compatible with GPL v2)
- Clean static library integration via CMake
- Active community and responsive to issues
- Modern codebase (C++17+)
- Proven in 3D simulators

**Weaknesses**:
- No built-in VR/OpenXR support (0/10 rating)
- Limited ocean implementation (single-cascade FFT only)
- Small community (6,800 stars vs Ogre's 1,303, but more active)
- Forward rendering limits deferred shading options

**Verdict**: **Strong candidate**. Viable path with manageable ocean implementation effort (3-4 weeks for multi-cascade FFT).

### 1.2 Ogre3D Next

**Status**: v3.0.0 "Eris" (October 2024) - **Actively maintained but slower pace**
- 14,640 commits, 185 contributors, 1,303 stars
- Last commit: January 30, 2026

**Architecture**:
- Scene graph with SceneNode + MovableObject model
- Better separation of rendering from simulation
- SIMD-optimized math
- PBR materials (HLMS PBS system)

**Rendering Features**:
- Multiple APIs: Vulkan, D3D11, Metal (optimal modern coverage)
- Forward Clustered rendering + compute shaders
- GI via Voxel Cone Tracing (VCT)
- Built-in terrain system (Terra)

**VR Support**:
- OpenVR with instanced stereo (built-in)
- OpenXR NOT native (4-8 weeks to implement)
- Proven in maritime VR training (Yoy Simulators ships VR training sims with Ogre)

**Ocean Implementation**:
- NO built-in ocean (critical gap)
- Community options: OgreOcean, Hydrax (needs porting), Triton SDK (proprietary)
- Academic precedent: DEPTHWAVE maritime sim used Ogre3D + custom ocean
- Estimated effort: 4-8 weeks to implement production-quality ocean

**Migration Path**:
- Documented Irrlicht→Ogre3D migration examples exist
- Scene paradigm shift required (more extensive than Wicked Engine)
- HLMS material system learning curve
- No built-in GUI (requires separate solution)

**Strengths**:
- MIT license (fully compatible with GPL v2)
- Better scene graph for ship simulation
- Multiple rendering backends (Vulkan, D3D11, Metal)
- Proven in maritime environments
- Strong compute shader support

**Weaknesses**:
- Small, slowly-growing community (1,303 stars; last PR merged Jan 2026)
- Single core maintainer risk
- No built-in ocean (must implement from scratch)
- More complex material system (HLMS)
- No built-in GUI system

**Verdict**: **Good alternative, higher risk**. Better architecture fit but higher implementation burden (no ocean, no VR OpenXR). Consider as Phase 2 fallback.

### 1.3 Irrlicht (Current)

**Status**: Stable but aging
- Last major update: 2013
- Scene graph architecture less suitable for physics-heavy sims
- Water implementation: basic moving mesh with pre-computed FFT

**Migration Reality Check**:
- 35-40% of codebase requires significant changes (graphics)
- 60-65% requires minimal changes (simulation/network/audio)
- Test suite: Multi-monitor support, network sync, scenario loading

---

## 2. Ocean Simulation

### 2.1 GPU FFT Approach (Recommended)

**Best Practice**: Compute shader-based Stockham FFT with 3-4 cascades

**Performance Profile**:
| Grid Size | Per-Cascade Cost | Cascades | Total Time | 60fps Headroom |
|-----------|-----------------|----------|-----------|----------------|
| 256x256 | 0.3-0.5ms | 3 | 0.9-1.5ms | Good |
| 512x512 | 0.5-1.0ms | 3 | 1.5-3.0ms | Good |
| 512x512 | 0.5-1.0ms | 4 | 2.0-4.0ms | Marginal |
| 1024x1024 | 1.5-3.0ms | 2 | 3.0-6.0ms | Limited |

**Recommended**: 3 cascades at 512x512 (1.5-3.0ms total) balances quality and performance.

**Cascade Wavelength Distribution**:
- Cascade 1: 500-1000m (swell, long-period ocean)
- Cascade 2: 50-200m (wind waves, dominant energy)
- Cascade 3: 5-50m (capillary waves, surface detail)
- Optional Cascade 4: 0.5-5m (spray/foam detail)

**Spectrum Options**:
- JONSWAP (α=0.0081, γ=3.3, σ₁=0.07, σ₂=0.09) - Best for realistic North Atlantic conditions
- Phillips spectrum - Simpler, adequate for training sims
- Beaufort parameterization available: Hs = 0.27 × U² / g

**Wave-Ship Coupling**:
- Buoyancy: Heightfield queries to GPU texture for vertical displacement
- Roll/Pitch: Can be driven by wave slope (currently hardcoded sinusoids)
- Wakes: Kelvin angle + amplitude modulation (single-pass, computationally feasible)

**Implementation Options**:

#### Option A: Wicked Engine Ocean (Fast Path)
- **Base**: Wicked Engine wiOcean.cpp (MIT licensed)
- **Current**: Single-cascade FFT, basic Phillips spectrum
- **Work needed**:
  - Add 2 more cascades (1 week)
  - JONSWAP spectrum (3 days)
  - Jacobian-based foam (3 days)
  - Temporal foam decay (2 days)
- **Total effort**: 2-3 weeks
- **Notes**: MIT license, directly applicable code

#### Option B: From-Scratch Stockham FFT (Flexible Path)
- **Base**: gasgiant/FFT-Ocean tutorial (excellent reference)
- **Work needed**:
  - Core Stockham implementation (1 week)
  - 3-cascade dispatch (3 days)
  - JONSWAP spectrum (3 days)
  - Foam + spray systems (1 week)
  - Buoyancy integration (3 days)
- **Total effort**: 3-4 weeks
- **Benefits**: Full control, no dependency on external codebase
- **References**: achalpandeyy/OceanFFT (JONSWAP + Stockham reference)

### 2.2 Turnkey Solutions (Rejected)

**Triton Ocean SDK**:
- Features: Excellent (65K+ waves, Kelvin wakes, spray, foam)
- Cost: $3,900-$4,900
- **License**: Proprietary, NOT GPL-compatible
- **Verdict**: Hard blocker. Cannot use in GPL v2 project without relicensing entire Bridge Command as proprietary.

**Hydrax** (Ogre3D community):
- Status: Unmaintained, requires porting to modern Ogre
- Effort: 2-3 weeks (similar to custom implementation)

**OgreOcean**:
- Status: Community solution, limited documentation
- Effort: 4+ weeks to adapt and integrate

---

## 3. Physics Upgrade Path

### 3.1 Current Physics (Baseline)

**Integration Method**: Forward Euler (1st order), variable timestep (15-35ms from frame rate)

**Force Model**:
- Quadratic drag: F_drag = A × v² + B × v
- Simple thrust: F_thrust = engine_fraction × max_force
- Basic rudder: F_rudder = angle × (speed × A + thrust × B)

**Critical Gaps**:
- No added mass / hydrodynamic inertia (real ships: 50-200% additional mass)
- One-way wave coupling (waves displace ship, not vice versa)
- Roll/pitch from hardcoded sinusoids, not physics

**Empirical**: boat.ini contains ~50+ parameters (Mass, Inertia, DynamicsSpeedA/B, RudderA/B, PropWalk, Azimuth settings)

### 3.2 MMG Physics (Recommended Upgrade)

**Theoretical Basis**: Yasukawa & Yoshimura 2015 (Full 3-DOF maneuvering)

**Equations**: Modular approach with independent components:
- Hull hydrodynamics
- Propeller thrust + wake effects
- Rudder effectiveness (flow straightening, slipstream interaction)

**Published Coefficients** (KVLCC2 reference ship, fully available):
- Added mass: m'_x = 0.022, m'_y = 0.223, J'_z = 0.011
- Sway: Y'_v = -0.315, Y'_r = 0.083
- Yaw: N'_v = -0.137, N'_r = -0.049
- Propeller: K_T curve (k₀=0.2931, k₁=-0.2753, k₂=-0.1359), wake w_P0=0.35, thrust deduction t_P=0.22
- Rudder: Effective inflow from propeller slipstream

**Coefficient Estimation**:
- Full set available for common ship types
- Empirical formulae (Kijima, Lee, Yoshimura) can estimate all coefficients from hull dimensions: L, B, T, C_b
- Allows parameterization of diverse ship types

**Open-Source Implementations**:
- **mmgdynamics** (Python, pip installable) - Verified against SIMMAN 2008/2020
- **ShipMMG** - C++ alternative
- **MSS Toolbox** (MATLAB) - Academic reference

**Validation Data**:
- SIMMAN 2008/2020 experiments (free-running model tests)
- Turning circle advance: ~3.2 × Length_pp (typical for KVLCC2)
- Available experimental data for validation

**Integration Path**:
1. Implement 3-DOF surge/sway/yaw equations (1 week)
2. Replace drag model with MMG (3 days)
3. Integrate empirical coefficient tables (2 days)
4. Validate against known turning circles (2-3 days)
5. Extend to 6-DOF if roll/pitch coupling needed (1 week additional)

**Backward Compatibility**:
- boat.ini parameterization can be preserved as MMG coefficient definitions
- Existing scenarios continue to work with equivalent ship definitions
- Can implement as optional physics mode initially

**Effort**: 2-3 weeks for functional 3-DOF implementation

---

## 4. Audio System Upgrade

### 4.1 Current State

**Implementation**: PortAudio + libsndfile
- 4 sound types: engine, wave, horn, alarm
- No spatial audio
- No Doppler effect
- No HRTF support

### 4.2 OpenAL Soft (Recommended)

**Status**: v1.25.1 (January 20, 2026) - Very actively maintained
- 10,412 commits, 93 contributors, 2,600+ stars
- Modern C++20 codebase
- Last update: January 20, 2026

**License**: **LGPL 2.1 - Fully GPL-compatible**

**3D Audio Capabilities**:
- Full spatial positioning (azimuth, elevation, distance)
- Distance attenuation modeling
- Doppler effect
- Directional/cone sources
- EFX reverb, occlusion, low-pass filters
- Ambisonics up to 4th order

**HRTF Support**:
- Built-in, customizable via SOFA profiles
- Crucial for VR immersion and directional audio cues

**Platform Support**:
| Platform | Implementation | Status |
|----------|-----------------|--------|
| Windows | WASAPI | Native |
| Linux | PipeWire, PulseAudio, ALSA | Native |
| macOS | CoreAudio | Native |
| Android/iOS | OpenSL ES | Supported |

**Integration Effort**: 1-2 weeks
- Replace PortAudio calls with OpenAL Soft
- Implement spatial source positioning for engine/wave sounds
- Add Doppler for relative motion effects

**Alternatives Evaluation**:
- **FMOD**: Proprietary (NOT GPL-compatible)
- **Wwise**: Proprietary (NOT GPL-compatible)
- **PortAudio**: Current system, lacks spatial audio
- **OpenAL Soft**: Only viable option combining GPL compatibility + spatial audio + HRTF

**Verdict**: Clear winner, no alternatives in GPL space.

---

## 5. Physics Simulation Details

### 5.1 Wave-Ship Coupling (Current)

**One-way coupling**:
- Ocean FFT computes displacement field
- Ship reads height at position
- Only vertical displacement applied (heave)
- Roll/pitch driven by hardcoded sinusoid functions, not physics

**Recommended Improvements**:
1. **Height queries**: Compute shader reads heightfield texture at ship position (already viable in Wicked Engine)
2. **Wave slope calculation**: Compute normal vector from heightfield to get local wave tilt
3. **Buoyancy distribution**: Multiple sample points along hull waterline
4. **Dynamics integration**: Feed tilt/buoyancy into 6-DOF rigid body solver

**Feasibility**: Medium (1-2 weeks for basic version, 3-4 weeks for accurate pressure distribution)

### 5.2 Multi-Cascade FFT Ocean Implementation

**Key Implementation References**:
- **Wicked Engine**: wiOcean.cpp (MIT, ready to adapt)
- **Gasgiant FFT-Ocean**: Best tutorial implementation (comprehensive comments)
- **achalpandeyy/OceanFFT**: JONSWAP parameterization + Stockham algorithm

**Data Flow**:
```
SpectrumBuffer [JONSWAP spectrum]
  → ComputeShader (Stockham FFT)
    → DisplacementTexture [3-channel R32G32B32F]
    → NormalTexture [packed normal + Jacobian]
    → FoamTexture [temporal fade + threshold]
      → Visualization (ocean mesh + reflection probes)
```

**Performance Targets**:
- GPU compute: 1.5-3.0ms total (3 cascades at 512x512)
- CPU readback for buoyancy queries: 0.2-0.5ms (async)
- Memory: ~30-50 MB for textures + constant buffers

---

## 6. Comparative Analysis Matrix

| Criterion | Wicked Engine | Ogre3D Next | Irrlicht (Current) |
|-----------|---------------|-------------|-------------------|
| **Graphics Rendering** | | | |
| Modern API support (DX12/Vulkan/Metal) | Yes | Yes | Partial (aged) |
| Active maintenance | Very high (weekly) | Moderate (monthly) | Stagnant |
| VR support (OpenXR) | No (0/10) | Possible (4-8w) | No |
| Learning curve | Moderate | Moderate-High | High |
| **Integration** | | | |
| License (GPL compatible) | MIT ✓ | MIT ✓ | Zlib ✓ |
| Static library integration | Excellent | Good | Fair |
| Scene architecture fit | ECS (optimal) | Scene graph (good) | Scene graph (legacy) |
| **Ocean Simulation** | | | |
| Built-in ocean | Single-cascade | None | Basic FFT |
| Multi-cascade effort | 2-3 weeks | 4-8 weeks | 2-3 weeks |
| Compute shader support | Excellent | Excellent | None |
| **Audio** | | | |
| Current capability | None | None | PortAudio (2D) |
| OpenAL Soft integration | Direct | Direct | Direct |
| HRTF/3D spatial audio | Yes (OpenAL) | Yes (OpenAL) | Yes (OpenAL) |
| **Networking** | | | |
| ENet UDP support | Straightforward | Straightforward | Native |
| Multi-monitor/viewport | Custom impl. | Custom impl. | Native |
| **Physics** | | | |
| MMG integration | Easy | Easy | Easy |
| Current model | Quadratic drag | Quadratic drag | Quadratic drag |
| Estimated effort | 2-3 weeks | 2-3 weeks | 2-3 weeks |
| **Risk Profile** | | | |
| Maintenance risk | Low | Medium | High |
| Implementation risk | Medium | High | Low |
| Performance risk | Low | Low | None (known) |
| Community size | Growing (6.8k) | Small (1.3k) | Stagnant (outdated) |
| **Total Migration Effort** | 4-6 weeks | 8-12 weeks | N/A (current) |

---

## 7. Backward Compatibility Analysis

### 7.1 Critical Systems (Must Not Break)

**Networking** (HIGH PRIORITY):
- ENet UDP stack with "BC" full-state messages (13 sections)
- "OS" lightweight own-ship-only sync
- "Scn" scenario data at connection
- Multi-instance primary/secondary setup
- **Migration path**: Abstract network layer (1 week), maintain protocol unchanged
- **Risk**: Low if protocol unchanged; High if graphics engine forces restructuring

**Multi-Monitor Support** (HIGH PRIORITY):
- Fake fullscreen borderless windows
- Monitor targeting via bc5.ini
- Viewport switching (radar → 3D → VR → GUI overlay)
- **Current implementation**: Irrlicht-specific viewport API
- **Migration path**: Graphics-agnostic viewport abstraction (1 week)
- **Risk**: Medium - requires careful mapping of monitor configuration

**Scenario Loading** (MEDIUM PRIORITY):
- boat.ini format (50+ ship parameters)
- Scenario files (environmental data, waypoints, initial conditions)
- **Current usage**: Direct INI parsing
- **Migration path**: Keep parser, extend to MMG coefficient mapping (3 days)
- **Risk**: Low if format preserved

**GUI System** (MEDIUM PRIORITY):
- Overlay menus, HUD elements
- Radar screen, chart display
- Currently uses Irrlicht's GUI environment
- **Options**:
  1. Wicked Engine: Requires custom GUI layer (2 weeks)
  2. Ogre3D: Requires CEGUI or similar (2-3 weeks)
  3. Current approach: Keep separate 2D overlay (1 week to adapt)
- **Risk**: Medium

### 7.2 Scenario Backward Compatibility Strategy

**boat.ini preservation**:
```
[Current format preserved]
Mass = 10000
Inertia = 50000
DynamicsSpeedA = 0.5
DynamicsSpeedB = 0.002
RudderA = 0.15
RudderB = 0.003

[New MMG coefficients added]
MMGMode = 1  # 0 = legacy drag, 1 = MMG
m_prime_x = 0.022
m_prime_y = 0.223
Y_prime_v = -0.315
...
```

**Compatibility mode**:
- If `MMGMode = 0`: Use legacy quadratic drag (existing behavior)
- If `MMGMode = 1`: Use MMG equations with published coefficients
- Default: Legacy mode (no breaking changes)

**Scenario files**: No changes required (physics model is transparent to scenario data)

---

## 8. Recommended Action Plan

### Phase 1: Validation (Weeks 1-5)

**Objective**: Prove feasibility of key subsystems in isolation

**Week 1-2: Wicked Engine Integration**
- Clone Wicked Engine repository
- Integrate as static library (CMake)
- Port simple 3D scene (one monitor rendering)
- Verify DX12/Vulkan/Metal rendering paths work
- **Deliverable**: Wicked Engine triangle rendering on bridge console

**Week 2-3: GPU FFT Ocean Prototype**
- Implement single compute shader with Stockham FFT
- Port wiOcean.cpp as reference
- Test with JONSWAP spectrum
- **Deliverable**: Real-time GPU-accelerated ocean visualization

**Week 3-4: OpenAL Soft Integration**
- Replace PortAudio with OpenAL Soft
- Implement spatial positioning (engine sound at bow, wave sounds around)
- Test HRTF loading
- **Deliverable**: 3D audio test with moving source and listener

**Week 4-5: MMG Physics Proof-of-Concept**
- Implement 3-DOF surge/sway/yaw equations
- Load KVLCC2 coefficients
- Test against known turning circle data
- **Deliverable**: Physics simulation producing realistic maneuvering

**Phase 1 Success Criteria**:
- ✓ Graphics rendering at 60fps on target hardware
- ✓ Multi-cascade ocean (at least 2 cascades)
- ✓ Audio spatial positioning working
- ✓ Ship turning circle ±10% from reference

### Phase 2: Full Implementation (Weeks 6-12)

**Week 6-7: Multi-Cascade Ocean with Foam**
- Extend to 3-cascade system (swell + wind + capillary)
- Implement Jacobian-based foam detection
- Add temporal foam decay
- Integrate buoyancy height queries
- **Deliverable**: Production-quality ocean visualization

**Week 7-8: Networking & Multi-Monitor Refactor**
- Abstract viewport/monitor layer from graphics engine
- Verify ENet protocol unchanged
- Test primary/secondary networked scenario
- Implement screen configuration persistence
- **Deliverable**: Multi-monitor setup working with new graphics engine

**Week 9-10: GUI System Migration**
- Evaluate GUI options (custom layer vs CEGUI)
- Migrate radar overlay, menus, HUD
- Test on all 3 monitor types
- **Deliverable**: Full GUI working on bridge console

**Week 11-12: Physics & Scenario Integration**
- Extend MMG to 6-DOF (include roll/pitch from wave slopes)
- Validate boat.ini loading with backward compatibility
- Test existing scenarios with new physics
- Implement compatibility mode (legacy vs MMG)
- **Deliverable**: Existing scenarios loadable and playable

### Phase 3: Decision Point (Week 13)

**Evaluate against success criteria**:
- Performance (target: 60fps on existing hardware)
- Rendering quality (better than Irrlicht)
- Physics realism (MMG accuracy)
- VR readiness (for future expansion)
- Code maintainability (vs complexity)

**Decision branches**:

**Path A: Continue with Wicked Engine**
- Multi-monitor works well, ECS architecture proven
- Custom GUI adequate, performance good
- VR future-proof if OpenXR added later
- Weeks 13-16: Polish, optimization, testing
- Go-live: Week 17

**Path B: Switch to Ogre3D Next**
- Only if Wicked Engine hits performance walls or VR becomes critical
- More scene graph complexity but better rendering options
- Extra 4 weeks: Ogre integration + ocean implementation
- Go-live: Week 21

**Path C: Hybrid (Wicked Engine + Component Updates)**
- Keep Irrlicht graphics (defer migration)
- Upgrade ocean (GPU FFT), audio (OpenAL), physics (MMG)
- Minimal breaking changes
- Effort: 6-8 weeks
- Risk: Technical debt accumulates; Irrlicht shader maintenance burden

---

## 9. Risk Assessment

### 9.1 High-Risk Items

**Risk 1: Multi-Monitor Viewport Switching**
- **Severity**: HIGH (breaks core functionality)
- **Probability**: MEDIUM (graphics abstraction layer needed)
- **Mitigation**: Early Phase 1 testing; abstract viewport layer before graphics swap
- **Backup plan**: Keep Irrlicht rendering for monitor handling initially

**Risk 2: Network Protocol Incompatibility**
- **Severity**: CRITICAL (breaks networked training scenarios)
- **Probability**: LOW (ENet unchanged, just transport layer)
- **Mitigation**: Extensive protocol validation testing; automated test suite for all 3 message types
- **Backup plan**: None - must work correctly from start

**Risk 3: Performance Regression on Older Hardware**
- **Severity**: MEDIUM (may fail on secondary PC if underpowered)
- **Probability**: MEDIUM (GPU FFT demands compute capability)
- **Mitigation**: Target RTX 2060 equivalent as minimum; profile early
- **Fallback**: Single-cascade ocean for lower-end GPUs

**Risk 4: GUI System Complexity**
- **Severity**: MEDIUM (affects user interaction and training)
- **Probability**: MEDIUM (no built-in system in new engines)
- **Mitigation**: Evaluate CEGUI vs custom 2D overlay early (Phase 1)
- **Fallback**: Keep Irrlicht GUI layer, blend with new graphics

### 9.2 Medium-Risk Items

**Risk 5: Compute Shader Compatibility**
- **Severity**: MEDIUM (FFT performance depends on it)
- **Probability**: LOW (modern GPUs all support compute)
- **Mitigation**: Test on target hardware; provide fallback FFT implementation
- **Fallback**: CPU-based FFT (slower, acceptable for training)

**Risk 6: Physics Model Validation**
- **Severity**: MEDIUM (incorrect coefficients break training)
- **Probability**: MEDIUM (requires rigorous testing vs real data)
- **Mitigation**: Use published KVLCC2 coefficients; validate against SIMMAN 2008/2020
- **Fallback**: Provide "legacy mode" to revert to current quadratic drag

**Risk 7: Third-Party Library Maintenance**
- **Severity**: MEDIUM (depends on Wicked Engine active development)
- **Probability**: LOW (very active community, 56 contributors)
- **Mitigation**: Choose engine with active community; monitor upstream changes
- **Fallback**: Ogre3D Next as alternative (different risk profile)

### 9.3 Low-Risk Items

**Risk 8: License Compatibility**
- **Severity**: LOW (MIT + LGPL both GPL-compatible)
- **Probability**: LOW (licenses already verified)
- **Mitigation**: Document license decisions; legal review for LGPL
- **Fallback**: None needed

**Risk 9: Scenario File Format Breakage**
- **Severity**: LOW (backward compatibility strategy in place)
- **Probability**: LOW (physics model is transparent to scenarios)
- **Mitigation**: Version boat.ini format; compatibility mode in code
- **Fallback**: Scenario converter script if needed

**Risk 10: Audio System Migration**
- **Severity**: LOW (OpenAL Soft mature and stable)
- **Probability**: LOW (proven LGPL compatibility)
- **Mitigation**: Standard library integration
- **Fallback**: Keep PortAudio for 2D audio fallback

---

## 10. Technology Comparison Table: Ocean Solutions

| Feature | Wicked Engine wiOcean | Custom Stockham FFT | Triton SDK | Hydrax | Current Irrlicht |
|---------|----------------------|-------------------|-----------|--------|-----------------|
| **Cascades** | 1 (builtin) | 3-4 (implementable) | 4+ (proprietary) | 2-3 (unmaintained) | 1 (FFT) |
| **Spectrum** | Phillips | JONSWAP/Phillips | PM/JONSWAP/Phillips | Phillips | Phillips |
| **Kelvin Wakes** | No | Possible (1w) | Yes | Yes | No |
| **Foam** | Basic | Jacobian-based | Advanced | Basic | No |
| **Spray** | No | Possible (1w) | Yes | No | No |
| **GPU Compute** | Wicked FFT | Native | Proprietary | Deprecated | None |
| **Buoyancy Queries** | Possible | Native texture read | Yes | Yes | CPU-only |
| **License** | MIT | Custom (MIT) | Proprietary ✗ | Unmaintained | Zlib |
| **Implementation Time** | 2-3 weeks | 3-4 weeks | $4,000 + integration | 2-3 weeks (porting) | Baseline |
| **VR-Ready** | Needs work | Yes | Yes | Needs porting | No |
| **Active Maintenance** | Yes | Manual | Commercial | No | No |
| **Recommendation** | Primary path | Fallback | Rejected | Not viable | Replace |

---

## 11. Implementation Effort Summary

### Total Project Timeline

**Assumption**: Full-time team of 2-3 developers

| Phase | Duration | Effort | Risk |
|-------|----------|--------|------|
| Phase 1 (Validation) | 5 weeks | 40-50 person-days | Medium |
| Phase 2 (Implementation) | 7 weeks | 70-90 person-days | Medium |
| Phase 3 (Testing & Polish) | 2-3 weeks | 30-40 person-days | Low |
| **Total** | **14-15 weeks** | **140-180 person-days** | **Medium** |

### Effort Breakdown by Component

| Component | Effort | Status |
|-----------|--------|--------|
| Graphics engine (Wicked) | 2-3 weeks | Primary path |
| Ocean (GPU FFT, 3-cascade) | 3-4 weeks | Proven approach |
| Physics (MMG 3-DOF) | 2-3 weeks | Published algorithms |
| Audio (OpenAL Soft) | 1-2 weeks | Straightforward |
| Networking (preserve) | 1 week | Minimal changes |
| Multi-monitor/GUI | 2-3 weeks | Moderate complexity |
| Testing & validation | 2-3 weeks | Ongoing |

---

## 12. Go/No-Go Decision Criteria

### Green Light Decision Points

Project should proceed to Phase 2 if **all** of the following are true:

1. **Performance**: Phase 1 prototype achieves 60fps on target hardware (RTX 2060 equivalent)
2. **Ocean quality**: Multi-cascade FFT with foam visually comparable to commercial sims
3. **Networking**: ENet protocol unchanged; primary/secondary sync verified
4. **Audio**: Spatial audio and HRTF functional in test scenario
5. **Physics**: MMG turning circle within ±10% of reference data
6. **Schedule**: Phase 1 completed within 5 weeks ±2 days

### Yellow Light (Escalation)

Proceed with caution if:
- Phase 1 runs 5+ days over schedule (evaluate resourcing)
- Single-cascade ocean inadequate (approve 1-week extension for multi-cascade prototype)
- GUI migration complexity higher than estimated (approve CEGUI evaluation)

### Red Light (Decision Point)

Halt Phase 2 and reconvene if:
- Performance drops below 45fps on target hardware
- Networking protocol incompatibility detected
- Graphics API compatibility issues on target platforms
- Compute shader not supported on secondary PC GPU

---

## 13. Licensing & Legal Summary

### Verified GPL v2 Compatibility

| Component | License | Status | Notes |
|-----------|---------|--------|-------|
| Wicked Engine | MIT | ✓ Compatible | Permissive |
| Ogre3D Next | MIT | ✓ Compatible | Permissive |
| OpenAL Soft | LGPL 2.1 | ✓ Compatible | Dynamic linking allowed |
| GPU FFT (custom) | MIT | ✓ Compatible | Recommended |
| Bridge Command | GPL v2 | Current | Upstream license |

### Incompatible (Rejected)

| Component | License | Status | Notes |
|-----------|---------|--------|-------|
| Triton SDK | Proprietary | ✗ Incompatible | Commercial only |
| FMOD | Proprietary | ✗ Incompatible | Not GPL-compatible |
| Wwise | Proprietary | ✗ Incompatible | Not GPL-compatible |

**Conclusion**: All recommended paths maintain GPL v2 compliance. No licensing blockers.

---

## 14. Appendix: Reference Data

### MMG Coefficients (KVLCC2 Reference)

```
Hull Hydrodynamics:
  m'_x = 0.022        (added mass, surge direction)
  m'_y = 0.223        (added mass, sway direction)
  J'_z = 0.011        (added moment inertia, yaw)
  Y'_v = -0.315       (sway force due to sway velocity)
  Y'_r = 0.083        (sway force due to yaw rate)
  N'_v = -0.137       (yaw moment due to sway velocity)
  N'_r = -0.049       (yaw moment due to yaw rate)

Propeller:
  K_T(J) = 0.2931 - 0.2753*J - 0.1359*J²  (thrust coefficient)
  w_P0 = 0.35         (wake fraction)
  t_P = 0.22          (thrust deduction factor)

Rudder:
  Effective inflow from propeller slipstream
  Flow straightening factor: x_P = 0.048 (typical)
  Rudder area effectiveness: A_R / A_P = 1/30 (typical)
```

### JONSWAP Ocean Spectrum Parameters

```
α = 0.0081           (spectral shape, Phillips constant)
γ = 3.3              (peak enhancement factor)
σ₁ = 0.07            (spectral width below peak)
σ₂ = 0.09            (spectral width above peak)
f_p = 1.3 / (√Hs)    (peak frequency, Hs in meters)

Beaufort to Wave Height:
  Hs (m) = 0.27 * (U_wind (m/s))² / g

Example: 20-knot wind (10.3 m/s) → Hs ≈ 3.3m
```

### Computational FFT Grid Recommendation

```
3-Cascade System (Recommended):
  Cascade 0 (swell):    256x256 grid, domain 500-1000m   → 0.3-0.5ms
  Cascade 1 (wind):     512x512 grid, domain 50-200m    → 0.5-1.0ms
  Cascade 2 (ripples):  512x512 grid, domain 5-50m      → 0.5-1.0ms

  Total GPU time: 1.5-3.0ms @ 60fps (16.67ms budget)
  CPU overhead: 0.2-0.5ms (buoyancy queries, async readback)

  Margin: 13-15ms available for other rendering, physics, audio
```

### Test Hardware Target

```
Primary Monitor:  RTX 2060 (minimum), RTX 3080 (recommended)
Secondary PC:     RTX 2060 (minimum), or integrated GPU fallback
                  → Falls back to single-cascade or CPU FFT

Memory footprint: 30-50 MB for ocean textures (3 cascades)
Bandwidth:        4 Mtexels/frame for foam write
```

---

## 15. Conclusion

### Summary of Recommendations

1. **Graphics Engine**: Wicked Engine (Primary Path)
   - Active development, MIT license, ECS architecture
   - GPU FFT ocean achievable in 2-3 weeks
   - Fallback to Ogre3D Next if VR becomes critical priority

2. **Ocean System**: GPU FFT with 3 cascades
   - Adapt Wicked Engine wiOcean.cpp as foundation
   - JONSWAP spectrum for realistic conditions
   - Jacobian-based foam for visual fidelity

3. **Physics Model**: MMG 3-DOF with published coefficients
   - Massive realism improvement over current quadratic drag
   - Backward-compatible through boat.ini extensions
   - Validated against published experimental data

4. **Audio System**: OpenAL Soft
   - Only GPL-compatible spatial audio solution
   - HRTF support for immersive experience
   - 1-2 week integration effort

5. **Network/Scenarios**: Preserve existing architecture
   - ENet protocol unchanged
   - Backward compatibility through versioned boat.ini
   - No breaking changes to scenario format

### Project Success Factors

- **Early validation** (Phase 1) of critical subsystems
- **Performance profiling** on target hardware throughout
- **Extensive testing** of networking and multi-monitor scenarios
- **Clear go/no-go criteria** at decision points
- **Fallback plans** for high-risk items (GUI, compute shader compatibility)

### Expected Outcomes

- **Visual fidelity**: Professional oceanography simulation quality
- **Physics realism**: Scientifically validated ship maneuvering
- **Immersion**: Spatial audio with HRTF for operator awareness
- **Maintainability**: Modern engine with active community support
- **VR-ready**: Foundation for future OpenXR support
- **Backward compatible**: Existing scenarios and hardware unchanged

**Estimated go-live**: Week 17-18 (4 months from Phase 1 start)

---

---

## 16. Chart Integration & OpenCPN Tile Loading

### 16.1 Critical Requirement

The 3D rendered world MUST match nautical chart data for land, buildings, and especially buoys. Scenarios need to span multiple map tiles with multiple ships (own + AI). OpenCPN chart tiles must be loadable.

### 16.2 S-57 ENC Format

**Standard**: IHO S-57 Edition 3.1, based on ISO/IEC 8211 binary encoding.

**File Structure**:
- Base file: `.000` extension
- Update files: `.001`, `.002`, etc. (applied automatically by GDAL)
- Coordinate encoding: integers divided by COMF (default 10,000,000) giving ~0.3m precision
- Horizontal datum: **always WGS 84** (EPSG:4326) -- matches GPS directly
- Vertical datum: varies (MLLW for US, LAT for international charts)

**Key Object Classes for 3D Simulation**:

| Category | Object Classes | Use in Simulator |
|----------|---------------|------------------|
| **Buoys** | BOYLAT, BOYCAR, BOYISD, BOYSAW, BOYSPP | Point geometry + BOYSHP → 3D model, COLOUR → paint scheme |
| **Lights** | LIGHTS | LITCHR + SIGPER + SIGGRP → flash pattern, SECTR1/2 → sectors |
| **Coastline** | COALNE, LNDARE | Coastline polylines + land polygons → terrain mesh boundary |
| **Depths** | DEPARE, DEPCNT, SOUNDG | Depth polygons + sounding points → seafloor mesh |
| **Landmarks** | LNDMRK | CATLMK → 3D model (tower, chimney, church, etc.), HEIGHT |
| **Buildings** | BUISGL, BUAARE | Area footprint + HEIGHT → extruded 3D building |
| **Obstructions** | OBSTRN, UWTROC, WRECKS | Hazard placement |

**S-57 Attribute Encoding Reference**:

| Attribute | Code | Values |
|-----------|------|--------|
| BOYSHP (shape) | 4 | 1=conical, 2=can, 3=spherical, 4=pillar, 5=spar, 6=barrel, 7=super-buoy |
| COLOUR | 75 | 1=white, 2=black, 3=red, 4=green, 5=blue, 6=yellow (list type, comma-separated) |
| COLPAT (pattern) | 76 | 1=horizontal stripes, 2=vertical, 3=diagonal, 4=squared, 5=stripes, 6=border |
| CATCAM (cardinal) | 13 | 1=north, 2=east, 3=south, 4=west |
| CATLAM (lateral) | 36 | 1=port, 2=starboard, 3=pref channel to stbd, 4=pref channel to port |
| LITCHR (light char) | 107 | 1=F, 2=Fl, 3=LFl, 4=Q, 5=VQ, 7=Iso, 8=Oc, 12=Mo (29 total values) |
| SIGPER | - | Signal period in seconds |
| SIGGRP | - | Signal group e.g. "(2)" |
| SECTR1/SECTR2 | - | Sector limits in degrees |
| VALNMR | - | Nominal range in NM |

### 16.3 Parsing Library: GDAL/OGR (Recommended)

**License**: MIT/X11 -- fully GPL-compatible

**Key Facts**:
- Built-in S-57 driver, no extra plugins needed
- Each S-57 object class becomes a named OGR layer
- Handles update files automatically
- Available on all platforms via package managers

**C++ Integration**:
```cpp
#include "ogrsf_frmts.h"

GDALAllRegister();
const char* openOptions[] = {
    "SPLIT_MULTIPOINT=ON",    // Split sounding clusters
    "ADD_SOUNDG_DEPTH=ON",    // Add DEPTH attribute to soundings
    "LNAM_REFS=ON",           // Feature-to-feature references
    nullptr
};

GDALDatasetUniquePtr poDS(
    GDALDataset::Open("chart.000", GDAL_OF_VECTOR, nullptr, openOptions));

// Extract buoys
OGRLayer* poBuoys = poDS->GetLayerByName("BOYLAT");
for (auto& feat : poBuoys) {
    OGRPoint* pt = feat->GetGeometryRef()->toPoint();
    double lon = pt->getX(), lat = pt->getY();
    int boyshp = feat->GetFieldAsInteger("BOYSHP");
    const char* colour = feat->GetFieldAsString("COLOUR");
}
```

**CMake Integration**:
```cmake
find_package(GDAL CONFIG REQUIRED)
target_link_libraries(enc_reader PRIVATE GDAL::GDAL)
```

**Installation**: `brew install gdal` (macOS), `apt install libgdal-dev` (Ubuntu), `vcpkg install gdal`

### 16.4 S-57 to Bridge Command Mapping

Direct mapping from S-57 features to existing Bridge Command world format:

| S-57 Feature | Bridge Command File | Mapping |
|-------------|--------------------|---------|
| BOYLAT/BOYCAR/BOYISD/BOYSAW | buoy.ini | Type from BOYSHP+COLOUR, Long/Lat from geometry |
| LIGHTS | light.ini | RGB from COLOUR, range from VALNMR, sequence from LITCHR+SIGPER+SIGGRP |
| LNDMRK/BUISGL | landobject.ini | Type from CATLMK/FUNCTN, position from geometry |
| DEPARE/SOUNDG | heightmap PNG | Depth areas → bathymetry raster, soundings → point interpolation |
| COALNE/LNDARE | heightmap PNG | Coastline → land/sea boundary mask |

**Light sequence conversion**: LITCHR + SIGPER + SIGGRP → Bridge Command L/D (light/dark) pattern at 0.25s intervals.

### 16.5 Other Chart Formats

| Format | Type | Use Case |
|--------|------|----------|
| **BSB/KAP** | Raster | Chart background textures (GDAL has built-in driver) |
| **S-63** | Encrypted S-57 | Commercial charts -- s63lib (github.com/pavelpasha/s63lib) for decryption |
| **MBTiles** | SQLite raster tiles | Satellite imagery overlays, read via SQLite (z/x/y tile scheme) |
| **CM93** | Legacy C-Map vector | OpenCPN has support, not recommended for new integration |

### 16.6 Free S-57 Chart Sources

| Source | Coverage | License | Quality |
|--------|----------|---------|---------|
| **NOAA ENC** (charts.noaa.gov/ENCs/) | All US coastal + Great Lakes | Public domain | Excellent |
| **CHS** (charts.gc.ca) | Canadian waters | Free | Excellent |
| **Brazil DHN** | Brazilian coast | Free | Good |
| **Finland Traficom** | Finnish waters | Open data | Good |
| **European Inland ENC** | 13+ EU countries | Free | Variable |

**NOAA is the best test data source**: ~755 MB for complete US coverage, no registration required.

---

## 17. Bathymetry & Elevation Data Sources

### 17.1 Data Source Hierarchy (by resolution)

| Priority | Source | Resolution | Coverage | License |
|----------|--------|-----------|----------|---------|
| 1 | **NOAA BlueTopo** | 1-4m (harbors) | US navigable waters | Public domain |
| 2 | **NOAA CRM** | ~30m (1 arc-sec) | US coastal | Public domain |
| 3 | **EMODnet DTM** | ~115m | European seas | Open data |
| 4 | **Copernicus DEM GLO-30** | 30m (land only) | Global land | Free (attribution) |
| 5 | **SRTM GL1** | 30m (land only) | 60N-56S land | Public domain |
| 6 | **GEBCO 2024** | ~450m (15 arc-sec) | Global ocean+land | Public domain |
| 7 | **ETOPO 2022** | ~450m | Global ocean+land | Public domain |

### 17.2 Programmatic Access

**GEBCO**: Download from download.gebco.net or OpenTopography REST API
```
https://portal.opentopography.org/API/globaldem?demtype=GEBCOIceTopo&south=...&north=...&west=...&east=...&outputFormat=GTiff&API_Key=KEY
```

**BlueTopo**: AWS S3 (no authentication)
```bash
aws s3 sync s3://noaa-ocs-nationalbathymetry-pds/BlueTopo/US4WA/ ./bluetopo/ --no-sign-request
```

**Copernicus DEM**: AWS S3 (no authentication): `s3://copernicus-dem-30m/`

**EMODnet**: WCS service
```
https://ows.emodnet-bathymetry.eu/wcs?service=WCS&version=1.1.2&request=GetCoverage&identifier=emodnet:mean&boundingbox=...
```

### 17.3 Heightmap Generation Pipeline

**Merging land + sea data with GDAL**:
```bash
# Merge: land from Copernicus, ocean from GEBCO
gdal_calc.py -A gebco_bathy.tif -B copernicus_land.tif \
  --outfile=merged.tif --calc="A*(A<=0) + B*(B>0)"

# For multi-source with priority (BlueTopo > CRM > GEBCO)
gdalbuildvrt -resolution highest merged.vrt \
  bluetopo_harbor.tif coastal_relief.tif gebco_fallback.tif
gdal_translate merged.vrt final_merged.tif
```

**Export to Bridge Command RGB heightmap**:
Height = (R*256 + G + B/256) - 32768 metres. Provides sub-metre vertical resolution.

### 17.4 Resolution Requirements

| Zone | Required Resolution | Recommended Source |
|------|--------------------|--------------------|
| Harbor/berth | 1-5m | BlueTopo |
| Harbor approach | 5-10m | BlueTopo, NOAA CRM |
| Coastal navigation | 10-30m | CRM/EMODnet + Copernicus DEM |
| Near-coastal ocean | 30-100m | ETOPO, EMODnet |
| Open ocean | 100-500m | GEBCO |

---

## 18. 3D World Generation from Chart Data

### 18.1 Building/Landmark Generation

**S-57 Source**: LNDMRK (landmarks with CATLMK category) and BUISGL (buildings with FUNCTN and HEIGHT) -- sparse, only navigationally significant structures.

**OSM Supplementary Source**: Building footprints with `building=*`, `height=*`, `building:levels=*`, `roof:shape=*` tags.

**Tools**:
| Tool | License | Output | Notes |
|------|---------|--------|-------|
| **OSM2World** | LGPL-3.0 | glTF, OBJ | Best open-source option, 250+ OSM tags, roof shapes |
| **GlobalBuildingAtlas** | Open data | LoD1 models | 2.68B buildings with predicted heights at 3m resolution |
| **Overpass API** | - | OSM XML/JSON | Direct OSM data download for a bounding box |

### 18.2 Multi-Tile World Generation

**LOD Approach**:
- Primary terrain covers full area at lower resolution
- Secondary terrains provide higher detail for harbors/approaches
- Bridge Command already supports this via terrain.ini multi-terrain stacking

**Tile Stitching**: Adjacent tiles share edge vertices (overlap by 1 pixel). Use skirt geometry to hide LOD cracks.

**Streaming**: For very large areas, use tile cache + background loading + virtual texturing.

### 18.3 Projection Systems

| Projection | Best For | Limitation |
|------------|----------|------------|
| **Local Tangent Plane (ENU)** | Single port/waterway (<50km) | Distorts beyond ~100km |
| **UTM** | Medium areas within one zone | Breaks at zone boundaries |
| **ECEF + floating origin** | Globe-scale, multi-area | More complex implementation |

**Current Bridge Command**: Uses simple linear projection (`longToX()`, `latToZ()` in Terrain.cpp). Works for small areas but will distort for large multi-tile regions. For the OpenCPN requirement, consider upgrading to ECEF floating origin or at minimum UTM.

### 18.4 Recommended Integration Architecture

**Option A: Standalone Chart-to-World Converter (RECOMMENDED)**

Build a converter (Python + GDAL) that:
1. Reads OpenCPN's config file to discover chart directories
2. Uses GDAL/OGR to parse S-57 charts directly
3. Extracts bathymetry, coastline, navigation aids
4. Generates Bridge Command world files (heightmap + texture + INI files)

This extends the existing `bc-world/GMRT_OSM_Importer` pattern.

**Advantages**: No runtime dependency on OpenCPN, offline processing, full control over conversion pipeline, builds on existing tooling.

**S-57 to Bridge Command pipeline**:
```
S-57 .000 files (from OpenCPN chart directories)
    → GDAL/OGR parse
        → DEPARE/SOUNDG → bathymetry heightmap (RGB PNG)
        → COALNE/LNDARE → coastline mask + land elevation
        → BOYLAT/BOYCAR → buoy.ini (Type/Long/Lat)
        → LIGHTS → light.ini (RGB/range/sequence/sector)
        → LNDMRK/BUISGL → landobject.ini (Type/Long/Lat/Rotation)
    + OSM2World → 3D building models
    + Copernicus DEM / BlueTopo → merged heightmap
    → terrain.ini + heightmap.png + texture.png
```

### 18.5 OpenCPN Architecture Reference

OpenCPN (GPLv2) bundles a stripped GDAL subset for S-57 parsing. Key internal libraries:
- `libs/gdal/` -- S-57 reader
- `libs/s52plib/` -- S-52 symbology rendering
- `libs/iso8211/` -- ISO 8211 binary format parser
- `libs/libtess2/` -- Polygon tessellation

**SENC Cache**: OpenCPN pre-processes S-57 → proprietary `.S57` binary cache (not reusable). Use GDAL directly instead.

**Chart Discovery**: OpenCPN stores chart directories in `opencpn.conf` under `[ChartDirectories]`:
- Linux: `~/.opencpn/opencpn.conf`
- macOS: `~/Library/Preferences/opencpn.ini`
- Windows: `C:\ProgramData\opencpn\opencpn.ini`

---

## 19. Updated Recommended Action Plan

### Phase 0 (New): Chart Integration Proof-of-Concept (Weeks 0-2)

**Week 0-1: S-57 Parser Integration**
- Add GDAL dependency to CMake build
- Implement S-57 reader extracting buoys, lights, coastlines, depths
- Test with NOAA ENC data (download Washington State: charts.noaa.gov/ENCs/WA.zip)
- **Deliverable**: Parse S-57 and output buoy.ini + light.ini from chart data

**Week 1-2: Heightmap Generation from Chart Data**
- Implement GDAL-based heightmap pipeline (DEPARE + SOUNDG → bathymetry)
- Merge with Copernicus DEM for land elevation
- Export as RGB-encoded PNG heightmap
- **Deliverable**: Auto-generated Bridge Command world from S-57 chart data

**Phase 0 Success Criteria**:
- Buoys placed from S-57 match positions on OpenCPN chart display
- Lights have correct characteristics (flash pattern, colour, sector)
- Coastline matches between 3D rendered view and chart
- Heightmap bathymetry is consistent with charted depths

### Phases 1-3 (Unchanged)
Continue with graphics engine, ocean, physics, audio upgrades as previously planned.

---

---

## 20. MoltenVK Performance & macOS Graphics Strategy (R7)

### 20.1 MoltenVK Current Status

**Version**: Vulkan 1.4 support (almost complete subset)
- SPIR-V → Metal Shading Language (MSL) transpilation
- Compute shaders: **Fully supported**
- Tessellation: **Supported** (via Metal tessellation)
- Subgroup operations: **Supported** (requires Vulkan 1.1+)
- **Geometry shaders: NOT SUPPORTED** (Metal has no geometry shader stage; PoC underway using Mesh shaders)

### 20.2 Key Limitation: No Geometry Shaders

Metal does not expose a geometry shader stage. MoltenVK cannot emulate this. A PoC effort is underway to use Mesh shaders as a workaround, but it's experimental. **For Bridge Command, this is not a blocker** since neither the current codebase nor the planned FFT ocean uses geometry shaders.

### 20.3 macOS Strategy Decision

Both candidate engines use **native Metal** backends (not MoltenVK):
- **Wicked Engine**: Native Metal 4 backend (added 2025)
- **Ogre3D Next**: Native Metal backend

**Decision**: MoltenVK is NOT needed for Bridge Command's upgrade path. Both engines bypass the Vulkan-to-Metal translation layer entirely, delivering native Metal performance.

### 20.4 Apple Platform Considerations

- OpenGL deprecated on macOS since 2018 (still functional but no updates)
- Metal is Apple's only supported modern graphics API
- MoltenVK useful for Vulkan-only codebases needing macOS support
- For new development: prefer native Metal backends over MoltenVK translation layer

---

## 21. Fossen 6-DOF Model & MSS Toolbox (R8)

### 21.1 6-DOF Extension Path

The current Bridge Command physics is 3-DOF (surge/sway/yaw). Fossen's framework extends this to full 6-DOF by adding:
- **Heave** (vertical motion): Requires hydrostatic restoring force + wave excitation
- **Roll** (rotation about longitudinal axis): Requires roll restoring moment + radiation damping
- **Pitch** (rotation about transverse axis): Requires pitch restoring moment + wave coupling

### 21.2 Fossen's 6-DOF Equation Structure

```
(M_RB + M_A) * ν̇ + C(ν)ν + D(ν)ν + g(η) = τ_env + τ_control

Where:
  M_RB  = Rigid body mass matrix (6×6)
  M_A   = Added mass matrix (6×6, frequency-dependent)
  C(ν)  = Coriolis + centripetal matrix
  D(ν)  = Damping matrix (linear + quadratic)
  g(η)  = Restoring forces (hydrostatic)
  τ_env = Environmental forces (waves, wind, current)
  τ_control = Propeller + rudder forces
```

### 21.3 MSS Toolbox

**Repository**: github.com/cybergalactic/MSS
- **License**: MIT (fully GPL-compatible)
- **Stars**: 631 | **Forks**: 186 | **Commits**: 1,921
- **Language**: MATLAB/GNU Octave
- Supplements: "Handbook of Marine Craft Hydrodynamics and Motion Control" (Fossen, 2021, Wiley)

**Capabilities**:
- GNC library (guidance, navigation, control)
- INS library (inertial navigation, error-state Kalman filters)
- HYDRO library (processes WAMIT/ShipX output for 6-DOF simulation)
- FDI toolbox (radiation-force model identification)
- Ship models: KVLCC2 tanker, container ship, frigate, AUVs, USVs, ROVs

### 21.4 Python Vehicle Simulator

**Repository**: github.com/cybergalactic/FossenHandbook
- Supplements MSS with Python implementations
- Models: DSRV, frigate, otter USV, ROV Zefakkel, semisubmersible
- Clarke83 ship parametric model available

### 21.5 Additional Coefficients for 6-DOF

Beyond MMG 3-DOF coefficients, 6-DOF requires:
- **Added mass matrix** (A₃₃, A₄₄, A₅₅ for heave/roll/pitch): From BEM solver (Capytaine) or strip theory
- **Radiation damping** (B₃₃, B₄₄, B₅₅): Frequency-dependent, from same BEM analysis
- **Hydrostatic restoring**: GM_T (transverse metacentric height), GM_L (longitudinal), waterplane area
- **Wave excitation forces**: Froude-Krylov + diffraction forces (RAOs from BEM or strip theory)

### 21.6 Practical Recommendation for Bridge Command

| Feature | 3-DOF (Current Plan) | 6-DOF (Future) |
|---------|---------------------|----------------|
| Maneuvering (turning, stopping) | Excellent | Excellent |
| Wave response (roll/pitch) | Hardcoded sinusoids | Physics-based |
| Seasickness cues | Limited | Realistic |
| Heavy weather training | Approximate | Realistic |
| Implementation effort | 2-3 weeks | 4-6 weeks additional |

**Decision**: Start with 3-DOF MMG (sufficient for maneuvering training). Extend to 6-DOF later if realistic wave response is needed for heavy weather training scenarios.

---

## 22. Capytaine BEM Solver (R9)

### 22.1 Overview

**Capytaine** is a Python BEM (Boundary Element Method) solver for linear potential flow around floating bodies. It computes the hydrodynamic coefficients needed for 6-DOF ship simulation.

- **Version**: 2.3.1 (October 2025)
- **License**: GPL-compatible (Fortran core from NEMOH is Apache 2.0)
- **Based on**: NEMOH v2 (École Centrale de Nantes)

### 22.2 What It Computes

From a hull mesh (STL/GDF format), Capytaine solves the boundary integral equation to produce:
- **Added mass matrix** A(ω): 6×6, frequency-dependent
- **Radiation damping matrix** B(ω): 6×6, frequency-dependent
- **Diffraction forces**: Wave excitation per direction per frequency
- **RAOs** (Response Amplitude Operators): Ship motion response to waves

### 22.3 How It Fits Bridge Command

```
Hull mesh (STL) → Capytaine → Added mass/damping tables → boat.ini coefficients
                                                         → 6-DOF simulation
```

This is an **offline preprocessing** step, not runtime. Run Capytaine once per vessel type to generate coefficient tables, then load those tables into the simulator.

### 22.4 Alternatives

| Tool | License | Language | Notes |
|------|---------|----------|-------|
| **Capytaine** | GPL-compat (Apache core) | Python/Fortran | Best option, active development |
| **NEMOH** | Apache 2.0 | Fortran | Original code Capytaine is based on |
| **HAMS** | Apache 2.0 | Fortran | Alternative BEM solver |
| **WAMIT** | Commercial | Fortran | Industry standard, not free |
| **ShipX (Veres)** | Commercial | - | Strip theory, SINTEF |

### 22.5 Decision

Use Capytaine (or NEMOH directly) for generating 6-DOF hydrodynamic coefficients IF the 6-DOF extension is pursued. This is a preprocessing tool, not a runtime dependency.

---

## 23. OpenBridge Maritime UI Design System (R11)

### 23.1 Overview

OpenBridge is an open-source design system for maritime user interfaces, developed by the Ocean Industries Concept Lab at the Oslo School of Architecture and Design.

- **Version**: 6.1
- **License**: Apache 2.0
- **GitHub**: Ocean-Industries-Concept-Lab/openbridge-webcomponents (75 stars, 25 forks)
- **Consortium**: 27+ industry partners (Kongsberg, Rolls-Royce, DNV, etc.)

### 23.2 Key Features

**Design Principles**:
- Human-centered design for safety-critical maritime operations
- 4 brightness palettes: Bright Day, Day, Dusk, Night
- IEC 62288 compliant (navigation display standards)
- IEC 62923-1/2 compliant (Bridge Alert Management)

**Technology**:
- Web Components (TypeScript) -- framework-agnostic
- Storybook component browser
- Figma UI kits for design
- CSS custom properties for theming

**Component Library**:
- Navigation instruments (compass rose, speed indicators)
- Alert management (BAM standard)
- Control panels, buttons, toggles
- Data display widgets
- Chart overlays

### 23.3 Relevance to Bridge Command

OpenBridge provides **design guidelines and UI patterns** for maritime bridge interfaces. While the web component implementation isn't directly usable in a C++ desktop app, the design principles are valuable:

1. **Color palettes**: Adopt the 4 brightness modes (bright day/day/dusk/night) for Bridge Command's GUI
2. **Instrument layouts**: Reference for radar overlay, compass, speed indicators
3. **Alert patterns**: BAM-compliant alert presentation
4. **Accessibility**: Maritime-specific accessibility guidelines

**Decision**: Adopt OpenBridge design principles as a reference for Bridge Command's GUI redesign. The actual web components won't be used (wrong technology stack), but the visual design language, color palettes, and interaction patterns should be followed.

---

## 24. O3DE Engine Evaluation (R14)

### 24.1 Overview

O3DE (Open 3D Engine) is an Apache 2.0 licensed engine managed by the Linux Foundation. It features the Atom renderer with Vulkan, DX12, and Metal backends.

### 24.2 Architecture Highlights

- **Atom Renderer**: Render graph-based architecture with explicit resource synchronization
- **Shader Resource Groups (SRGs)**: Resources grouped by update frequency (per-scene, per-pass, per-draw, per-material)
- **AZSL**: Custom shader language (thin extension over HLSL), transpiled via AZSLc compiler
- **SPIR-V**: Via DXC (DirectX Shader Compiler) for Vulkan
- **VMA**: AMD Vulkan Memory Allocator for GPU memory management
- **Variable Rate Shading**: Supported across backends

### 24.3 Assessment for Bridge Command

| Factor | Rating | Notes |
|--------|--------|-------|
| Graphics quality | Excellent | Modern PBR, render graph |
| Ocean simulation | None | No built-in ocean system |
| VR support | Good | OpenXR via Gem |
| Build complexity | Very High | Large monolithic build, many dependencies |
| Migration effort | Very High | Complete rewrite required, different paradigm |
| Community | Moderate | Active but smaller than expected for Linux Foundation project |
| License | Apache 2.0 | GPL-compatible |

### 24.4 Decision

**REJECTED** as an engine candidate for Bridge Command. Reasons:
1. Extremely complex build system and dependency graph
2. No ocean simulation (same gap as Ogre, but with much higher integration cost)
3. Overkill for a training simulator (designed for AAA games)
4. Much higher migration effort than Wicked Engine or Ogre-Next
5. Community smaller than expected despite Linux Foundation backing

O3DE's render graph architecture and SRG concepts are interesting references for understanding modern rendering, but the engine itself is not suitable for Bridge Command's upgrade.

---

## 25. IALA Radar Simulation Standards (R17)

### 25.1 Key Standards

**IMO MSC.192(79)** - Revised Performance Standards for Radar Equipment:
- Defines detection requirements for navigation radar
- Specifies target detection ranges by RCS (Radar Cross Section)
- Covers both X-band (9.41 GHz, 3 cm) and S-band (3.05 GHz, 10 cm)

### 25.2 Detection Range Requirements

| Target Type | Typical RCS (m²) | X-band Detection Range |
|-------------|------------------|----------------------|
| IALA buoy (small) | 10-20 | 2-4 NM |
| IALA buoy (large) | 20-50 | 4-6 NM |
| Small craft (<10m) | 1-10 | 2-4 NM |
| Medium vessel (50m) | 100-500 | 8-12 NM |
| Large vessel (200m) | 1,000-10,000 | 15-24 NM |
| Coastline | Varies | 6-12 NM |
| Radar reflector (IMO) | 10 (min required) | 3-5 NM |

### 25.3 Radar Display Requirements

Per MSC.192(79):
- Minimum display diameter: 250mm (effective)
- North-up and head-up orientations required
- Range scales: 0.25, 0.5, 0.75, 1.5, 3, 6, 12, 24, 48, 96 NM
- Target trails: True and relative motion
- Guard zones: Configurable areas with alarm capability

### 25.4 Implications for Bridge Command Radar

Current BC radar is CPU-computed. For realistic radar simulation:
1. **RCS-based returns**: Each target should have an RCS value; detection probability varies with range
2. **Sea clutter**: Proportional to sea state, stronger at shorter ranges
3. **Rain clutter**: Attenuation + backscatter, stronger for X-band
4. **Shadow sectors**: Behind large objects
5. **Side lobe effects**: False echoes at bearing offset
6. **SART/RACON**: Transponder responses on radar

**Decision**: Use MSC.192(79) RCS values and detection ranges as targets for radar simulation upgrade. Implement progressive: basic RCS-based returns first, then add clutter and propagation effects.

---

## 26. VRX/Gazebo Buoyancy Implementation (R12)

### 26.1 asv_wave_sim Plugin

**Repository**: github.com/srmainwaring/asv_wave_sim
- **License**: GPL-3.0
- **Language**: C++ (96.7%)
- **Stars**: 182 | **Forks**: 53
- **Dependencies**: CGAL (mesh manipulation), FFTW (FFT computation)

### 26.2 Wave Algorithms

Three wave generation methods available:
1. **Sinusoid**: Simple harmonic waves (basic)
2. **Trochoid**: More realistic wave shape (moderate)
3. **FFT**: Full spectral wave generation (best quality)

FFT parameters: `tile_size`, `cell_count` (power of 2), `wind_speed`, `wind_angle_deg`, `steepness`

### 26.3 Multi-Point Buoyancy Model

The Surface plugin applies buoyancy forces at configurable hull points:
```xml
<plugin filename="libSurface.so" name="maritime::Surface">
    <hull_length>4.9</hull_length>
    <hull_radius>0.213</hull_radius>
    <points>
        <point>0.6 1.03 0</point>   <!-- Forward -->
        <point>-1.4 1.03 0</point>  <!-- Aft -->
    </points>
</plugin>
```

Each point samples the wave height at its location, computing buoyancy force from hull volume below water. This produces realistic roll/pitch from wave excitation.

### 26.4 Gazebo Hydrodynamics Plugin

Uses Fossen's equations with these parameters:
- `xDotU`, `yDotV`, `nDotR`: Added mass coefficients
- `xU`, `xAbsU`: Linear + quadratic surge damping
- `yV`, `yAbsV`: Linear + quadratic sway damping
- `nR`, `nAbsR`: Linear + quadratic yaw damping
- `zW`, `kP`, `mQ`: Heave/roll/pitch damping

### 26.5 Key Takeaways for Bridge Command

1. **Multi-point buoyancy** is the key to realistic wave response (not single-point displacement)
2. **Cylindrical hull approximation** is sufficient for training simulators
3. **Wave height sampling at multiple hull points** drives roll/pitch naturally
4. **Fossen's damping model** is the standard approach (same equations as MSS toolbox)

**Note**: Cannot directly use asv_wave_sim code (GPL-3.0 vs BC's GPL-2.0), but the concepts and approach are freely adoptable.

---

## 27. Ship Simulator Architecture Papers (R15)

### 27.1 SIMMAN Workshops

**SIMMAN 2008** (Copenhagen): First workshop on verification and validation of ship maneuvering simulation methods.
- Test cases: KVLCC1, KVLCC2 (tanker), KCS (container ship), DTMB 5415 (surface combatant)
- Data: Captive model tests + free-running model tests
- Published: Quantitative comparisons between simulation methods and EFD data

**SIMMAN 2020** (Korea): Updated workshop.
- Added ONRT (Office of Naval Research Tumblehome) as new test case
- simman2020.kr website with proceedings
- ITTC Recommended Procedures 7.5-02-06-06 references SIMMAN benchmark data

### 27.2 Commercial Simulator Architectures

**Kongsberg K-Sim Navigation**:
- Advanced physics + hydrodynamic modeling + cutting-edge visual system
- Image generation: Separate visual system (upgradeable independently)
- Interoperability: DIS (Distributed Interactive Simulation) + HLA (High-Level Architecture) for joint exercises
- Multi-vessel training: Supports multiple vessels in same scenario
- Compliance: IMO/STCW/DNV standards
- Modular: K-Sim Navigation + K-Sim Engine + K-Sim Safety interconnectable

**Wärtsilä NTPRO 5000**:
- Visual system: 3D bow waves, scene reflections, water translucency, light refraction, white caps, foam, splashes
- Part-task simulator: Typically 3 visual channels
- Ice navigation simulation available
- Full mission simulator meets IMO Convention + DNV standards
- Inland waterway variant available

### 27.3 Architecture Patterns from Industry

| Pattern | Kongsberg | Wärtsilä | Bridge Command (Target) |
|---------|-----------|----------|------------------------|
| Visual system | Separate IG | Integrated | Wicked Engine (separate) |
| Physics model | Proprietary 6-DOF | Proprietary 6-DOF | MMG 3-DOF → 6-DOF |
| Networking | DIS/HLA | Proprietary | ENet UDP (preserve) |
| Multi-vessel | Yes | Yes | Yes (existing) |
| Chart integration | ENC native | ENC native | S-57 via GDAL (planned) |
| Compliance | IMO/DNV | IMO/DNV | Training-grade |

### 27.4 Academic References

- **Darles et al. (2011)**: "Ocean Simulation and Rendering Methods in Computer Graphics" - Comprehensive survey of FFT, Gerstner, and particle-based ocean methods
- **Tessendorf (2001)**: "Simulating Ocean Water" - Foundational paper for FFT ocean simulation
- **Yasukawa & Yoshimura (2015)**: MMG standard method introduction paper

---

**Document Version**: 3.0
**Last Updated**: February 8, 2026
**All 22 research topics COMPLETE**
**Next Step**: Begin Phase 0 (Chart Integration PoC) or Phase 1 (Graphics Engine Validation)
