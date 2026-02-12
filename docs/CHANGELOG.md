# Changelog: upgrade/graphics-and-simulation-overhaul

Changes relative to `main` branch (v5.10.4-alpha.4).
Target version: **6.0.0-beta.1**

---

## What You'll Notice When Running

### New Graphics Engine (Wicked Engine backend, optional)

When built with Wicked Engine (`-DWICKED_ENGINE_DIR=... -DWICKED_ENGINE_LIB=...`):

- **PBR rendering** via Wicked Engine (DX12/Vulkan/Metal)
- **GPU ocean** with 3-cascade FFT (far swells, wind waves, ripples)
- **JONSWAP spectrum** with wind-responsive wave generation
- **Jacobian foam** on breaking wave crests with temporal decay
- **Kelvin wakes** behind moving ships (19.47-degree pattern)
- **Multi-window rendering** for the 3-monitor bridge setup
- **Dear ImGui instrument HUD** with 6 instrument displays

Without Wicked Engine, the existing Irrlicht backend is used unchanged.

### ImGui Bridge Instruments (Wicked Engine mode)

Six instrument windows rendered via Dear ImGui:

- **Compass** -- Rotating rose with cardinal/intercardinal labels, tick marks, heading triangle, numeric readout
- **Speed** -- SOG (yellow), STW (green), COG
- **Rudder** -- Arc gauge with port (red) / starboard (green) color coding, yellow needle
- **Depth** -- Numeric readout with vertical gauge bar, alarm threshold marker (dashed red line), depth trend arrow (shoaling/deepening/steady), flashing red alarm, configurable m/ft units
- **Engine** -- RPM with progress bar, thrust lever percentage
- **Wind** -- Speed (knots) and direction (FROM)

Keyboard shortcuts:
- **F5-F8** -- Switch brightness palette (Bright Day / Day / Dusk / Night)
- **F9** -- Toggle layout lock (unlock to drag and resize instrument windows)
- **F10** -- Toggle depth units between metres and feet

Layout and preferences persist via `saveLayout()` / `loadLayout()`.

### Ship Physics (MMGMode=1 ships)

The **MMG (Maneuvering Modeling Group)** physics model is enabled for 8 ships:
ProtisSingleScrew, Protis, VIC56, VIC56_360, Puffer, HMAS_Westralia, Alkmini.

Compared to legacy physics:
- Proper 3-DOF hydrodynamic hull forces (Kijima regression for coefficient estimation)
- Propeller thrust via KT(J) polynomial with wake fraction and thrust deduction
- Rudder forces with propeller slipstream effect (effective at low speed with engine ahead)
- Added mass / hydrodynamic inertia (Clarke formulae)
- 50Hz fixed-timestep physics loop (RK2 integration)
- **Shallow water effects** (Gronarz power function model, Barras squat)
- **Bank effects** (Norrbin suction/yaw near channel walls)
- **Wind forces** (Isherwood 1972 model, replaces legacy simple windage)
- Validated against SIMMAN 2008 KVLCC2 reference data

All other ships use legacy physics unchanged.

### JONSWAP Ocean Waves

- **JONSWAP spectrum** replaces Phillips for more realistic wave shapes
- Wave height coupled to wind speed via Pierson-Moskowitz Hs relationship
- Wind direction sets wave propagation direction
- Phillips spectrum kept as runtime fallback

### Spatial Audio (OpenAL backend)

When built with OpenAL (`-DWITH_OPENAL=ON`):
- **3D spatial audio** -- engine at stern, horn at bow, positioned in world space
- **Doppler effect** on passing ships
- **Engine pitch** varies with RPM (0.5x-2.0x)
- **HRTF support** for VR headsets (non-Apple platforms)
- PortAudio fallback when OpenAL not available

### NMEA / AIS Output

**New NMEA sentences** (added to existing round-robin cycle):
- **VHW** -- Water speed and heading
- **MWV** -- Relative wind speed and angle

**New AIS messages**:
- **Message 5** (every 6 min) -- Ship name, dimensions, type for AI ships
- **Message 21** (every 3 min) -- AtoN reports for buoys (visible on AIS displays)

### Chart Converter Tool

`bc-chart-converter` (requires GDAL) converts S-57 ENC charts to BC worlds:
- Extracts buoys (BOYLAT/BOYCAR/BOYISD/BOYSAW/BOYSPP), lights, landmarks
- Generates heightmap from DEPARE polygons + sounding interpolation (IDW)
- Merges Copernicus DEM for land elevation, BlueTopo for hi-res bathymetry
- Height-aware terrain texture (7 color bands + urban overlay from BUAARE)
- Full world output: terrain.ini, height.png, texture.png, buoy.ini, light.ini, landobject.ini

---

## Detailed Phase Summary

### Phase 0A: Build & Code Modernization (6/7)

- C++17 compiler standard (was C++11)
- Catch2 v3.5.2 unit testing framework (89 tests, 235 assertions)
- CI test step added to GitHub Actions workflows
- Baseline physics and utility tests
- Performance baseline profiling deferred (needs bridge hardware)

### Phase 0B: Chart Integration (12/14)

- GDAL optional dependency in CMake
- ChartReader: buoy, light, coastline, depth area, sounding, landmark extraction
- Buoy type mapping (S-57 BOYSHP/COLOUR/CATLAM to BC model names)
- Light characteristic conversion (F/Fl/LFl/Q/VQ/Iso/Oc patterns, COLOUR to RGB)
- HeightmapGenerator: DEPARE rasterization, IDW sounding interpolation, RGB encoding
- WorldGenerator: full pipeline producing all world files from a single .000 chart
- bc-chart-converter CLI tool
- NOAA chart download and OpenCPN validation deferred (manual tasks)

### Phase 0C: Chart Bathymetry Enhancement (4/4)

- Copernicus DEM GLO-30 download helper script (tools/download_dem.py)
- DEM/chart bathymetry merge (GDAL GeoTIFF, bilinear interpolation)
- BlueTopo high-res bathymetry support (fallback chain: BlueTopo > S-57 > IDW)
- Height-aware terrain texture with 7 color bands and urban overlay

### Phase 1A: Physics - MMG Model (13/13)

- PhysicsModel abstraction (IPhysicsModel interface)
- LegacyPhysicsModel extracted from OwnShip.cpp
- MMGPhysicsModel: hull forces, propeller, rudder, added mass
- Kijima/Clarke coefficient estimation from hull dimensions
- Fixed-timestep (50Hz) physics loop with accumulator
- JONSWAP spectrum in FFTWave (runtime toggle, Phillips fallback)
- Wind-wave coupling (Hs from wind speed)
- SIMMAN 2008 KVLCC2 validation tests

### Phase 1B: Audio Upgrade (8/8)

- OpenAL Soft backend (SoundOpenAL.hpp/cpp)
- ISound abstract interface, PortAudio fallback preserved
- 3D spatial positioning (engine at stern, horn at bow, listener at camera)
- Doppler effect for moving ships
- Engine RPM-linked pitch variation
- HRTF support for VR mode

### Phase 2A: Graphics Abstraction (13/14)

- IRenderer, ISceneManager, ISceneNode, IMeshNode, ICameraNode, ILightNode interfaces
- bc::graphics::Types.hpp (Vec2, Vec3, Color, Matrix4, Quaternion, Line3d)
- Irrlicht backend wrappers (src/graphics/irrlicht/)
- Scalar type cleanup (irr::f32/u32/s32 replaced across all BC source)
- 40+ headers migrated from irrlicht.h to forward declarations + abstraction types
- GUIMain and main.cpp deferred (will be replaced by ImGui / WE renderer)

### Phase 2B: Wicked Engine Integration (9/10)

- WE v0.72.33 built and linked (Metal on macOS, DX12/Vulkan on Windows/Linux)
- WickedRenderer (IRenderer impl via wi::Application + wi::RenderPath3D)
- WickedSceneManager with camera, light, mesh, terrain, billboard, skybox node types
- WickedModelImporter: OBJ loading via TinyObjLoader, .gltf/.glb/.wiscene support
- WickedMultiView: multi-window rendering for bridge monitor setup
- WickedTerrainNode: heightmap mesh from BC terrain data with bilinear queries
- WickedWater: wraps wi::Ocean with BC wind/weather mapping
- WickedImGui: Dear ImGui v1.89.2 renderer backend with HLSL shaders
- Full simulation test deferred (needs bridge hardware)

### Phase 3: GPU Ocean Rendering (7/8)

- WickedMultiCascadeOcean: 3 cascades (2000m/256 far, 100m/512 wind, 20m/256 ripples)
- WickedOceanSpectrum: JONSWAP + TMA + Phillips spectrum functions
- Jacobian-based foam (built into WE's oceanUpdateGradientFoldingCS shader)
- Temporal foam via noise-modulated animation (WE's oceanSurfacePS)
- Wave height readback via Scene::GetOceanPosAt() (triple-buffered async GPU)
- WickedKelvinWake: per-ship V-shaped wake trails with age-based fade
- Performance testing deferred (needs bridge hardware)

### Phase 4: Radar Upgrade (5/5)

All features were already implemented in the existing codebase:
- RCS-based detection (R^4 range equation), now overridable via boat.ini
- Sea clutter (sea_state^2 / range^3 with wind direction correction)
- Rain clutter (rain_rate^2 / range^2)
- Radar shadow (scan slope blocking mechanism)

### Phase 5: Networking (4/4)

- NMEA VHW and MWV sentences
- AIS Message 5 (static/voyage data, 424-bit, 6-bit ASCII encoding)
- AIS Message 21 (AtoN for buoys, 272-bit, MMSI 993XXXXXX format)

### Phase 6: VR Improvements (3/3)

- WickedVRView: stereo camera management for VR via Wicked Engine (scaffolding)
- HRTF spatial audio: auto-enabled on VR load, head-tracked listener from OpenXR
- VR hand controls: Oculus Touch profile with squeeze/trigger/thumbstick bindings
- Trigger on either hand activates ship horn
- Thumbstick Y for fine engine (left) and rudder (right) adjustment while gripping
- Full testing requires VR hardware

### Phase 7: UI Modernization (7/7)

- ImGuiOverlay class with SimulationHUDData struct
- Compass: rotating rose, cardinal labels, tick marks, heading triangle
- Speed: SOG/STW/COG display
- Rudder: arc gauge with port/starboard color coding
- Depth: gauge bar, alarm threshold, trend arrow, flashing alarm, m/ft toggle
- 4 OpenBridge brightness palettes (Bright Day/Day/Dusk/Night) with F5-F8 shortcuts
- Layout save/load persistence, F9 lock toggle, ImGui .ini for window positions

### Phase 8: Advanced Physics (3/3)

- Shallow water effects (Gronarz model, Barras squat formula)
- Bank effects (Norrbin 1974 suction/yaw model)
- Wind forces (Isherwood 1972 Fourier model, replaces legacy windage in MMG path)

### Phase 9: Testing & Release (2/6)

- Ship coefficient database (docs/SHIP_COEFFICIENTS.md) for all 14 ownships
- Documentation updates (README, keyboard shortcuts, build deps)
- Remaining items need bridge hardware or are blocked on VR

---

## Files Added

- `src/graphics/` -- Abstraction interfaces (7 headers) + Irrlicht backend (6 files)
- `src/graphics/wicked/` -- Wicked Engine backend (18 files including ImGui and VR)
- `src/gui/ImGuiOverlay.hpp/cpp` -- ImGui instrument HUD overlay
- `src/PhysicsModel.hpp` -- Physics model interface
- `src/LegacyPhysicsModel.hpp/cpp` -- Extracted legacy physics
- `src/MMGPhysicsModel.hpp/cpp` -- MMG physics implementation
- `src/MMGCoefficients.hpp/cpp` -- Coefficient estimation
- `src/SoundOpenAL.hpp/cpp` -- OpenAL audio backend
- `src/ISound.hpp` -- Audio backend interface
- `src/ChartReader.hpp/cpp` -- S-57 chart reading
- `src/HeightmapGenerator.hpp/cpp` -- Heightmap from chart data
- `src/WorldGenerator.hpp/cpp` -- Full world generation pipeline
- `src/chartConverter/` -- CLI chart converter tool
- `src/tests/` -- 89 Catch2 unit tests
- `tools/download_dem.py` -- Copernicus DEM download helper
- `docs/SHIP_COEFFICIENTS.md` -- Per-ship MMG coefficient database
- `docs/WICKED_OCEAN_ANALYSIS.md` -- WE ocean code analysis
- `docs/CHANGELOG.md` -- This file

## Files Modified (key changes)

- `src/CMakeLists.txt` -- C++17, Catch2, GDAL, OpenAL, Wicked Engine integration
- `src/OwnShip.hpp/cpp` -- MMG physics integration, abstraction types
- `src/SimulationModel.hpp/cpp` -- Audio/physics/AIS integration, abstraction types
- `src/VRInterface.hpp/cpp` -- HRTF integration, Oculus Touch controls, horn/thumbstick actions
- `src/main.cpp` -- OpenAL backend selection
- `src/AIS.hpp/cpp` -- Message 5 and 21 generators
- `src/NMEA.hpp/cpp` -- VHW, MWV sentences
- `src/FFTWave.cpp` -- JONSWAP spectrum
- `src/Water.hpp/cpp` -- Wind-wave coupling, abstraction types
- `README` -- Build deps, MMG docs, chart converter, keyboard shortcuts
- 40+ headers -- irrlicht.h removed, bc::graphics types adopted
- 8 boat.ini files -- MMGMode=1 added
