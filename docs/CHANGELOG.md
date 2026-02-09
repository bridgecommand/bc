# Changelog: upgrade/graphics-and-simulation-overhaul

Changes relative to `main` branch (v5.10.4-alpha.4).

## What You'll Notice When Running

### NMEA / AIS Output (visible on connected chart plotters, OpenCPN, or NMEA monitors)

**New NMEA sentences** (added to the existing round-robin cycle, sent every 100ms):

- **VHW** -- Water speed and heading: `$VWVHW,hdg,T,,M,sogKts,N,sogKmh,K*hh`
- **MWV** -- Relative wind speed and angle: `$WIMWV,relAngle,R,windSpd,N,A*hh`

**New AIS messages** (sent alongside existing Class A position reports):

- **Message 5** (every 6 minutes) -- Static and voyage data for each AI ship: vessel name, dimensions (length/breadth from 3D model), ship type. If you have OpenCPN or an AIS decoder connected, AI ships will now show **names and size data** instead of just MMSI/position.
- **Message 21** (every 3 minutes) -- Aid-to-Navigation reports for every buoy in the scenario. Buoys will appear as **AtoN targets on external AIS displays** with MMSI format 993XXXXXX and names like "BUOY 1", "BUOY 2", etc.

### Ship Physics (only for ships with MMGMode=1)

The **MMG (Maneuvering Modeling Group)** standard physics model is available. Currently enabled only for the **ProtisSingleScrew** ship. When sailing this ship you'll notice:

- More realistic turning behavior (proper turning circle dynamics)
- Propeller thrust modeled with KT/KQ polynomials, wake fraction, and thrust deduction
- Rudder forces include propeller slipstream effects
- Coriolis coupling between surge/sway/yaw

All other ships use the legacy physics model and behave identically to the base app.

### Everything Else Looks the Same

The following changes are internal infrastructure -- they don't affect what you see or hear:

- C++17 compiler standard (was C++11)
- 68 unit tests via Catch2
- GitHub Actions CI runs tests on Linux amd64/arm64
- `bc::graphics` abstraction types (Vec2, Vec3, Color, etc.) replacing Irrlicht types in 39+ headers
- OtherShip radar cross section (RCS) now overridable via `RadarCrossSection=` in boat.ini
- `.gitignore` entries for S-57 chart files (*.000, *.001, *.002)
- GDAL/ChartReader module (conditional, `WITH_GDAL` CMake option)

## Detailed Change List

### Phase 0A: Build & Code Modernization

| Task | Description | Status |
|------|-------------|--------|
| 0A-01 | C++17 upgrade | Done |
| 0A-02 | Fix C++17 warnings | Done (none in BC source) |
| 0A-03 | GitHub Actions CI | Done (existed, test step added) |
| 0A-04 | Catch2 unit testing | Done (68 tests, 184 assertions) |
| 0A-05 | CI test step | Done |
| 0A-06 | Baseline tests (Angles, Utilities) | Done |

### Phase 0B: Chart Integration

| Task | Description | Status |
|------|-------------|--------|
| 0B-01 | GDAL CMake + ChartReader module | Done (conditional WITH_GDAL) |
| 0B-14 | .gitignore for chart data | Done |

### Phase 1A: MMG Physics Model

| Task | Description | Status |
|------|-------------|--------|
| 1A-01 to 1A-09 | Full MMG implementation | Done |

MMG model includes: hull hydrodynamic forces (Kijima estimation), propeller model (KT/KQ polynomials with J-dependent thrust), rudder forces with propeller slipstream, Coriolis coupling. Enabled per-ship via `MMGMode=1` in boat.ini.

### Phase 2A: Graphics Abstraction

| Task | Description | Status |
|------|-------------|--------|
| 2A-01 to 2A-13 | Migrate headers from irrlicht.h | 39+ done, 17 remaining (blocked) |

Remaining headers are blocked by: EventReceiver base class (9), GUI files (4, deferred to ImGui), core engine coupling (3), RadarCalculation (1, deferred to GPU radar).

### Phase 4: Radar

| Task | Description | Status |
|------|-------------|--------|
| 4-01 | RCS values | Already implemented + boat.ini override added |
| 4-02 | RCS-based detection (R^4) | Already implemented |
| 4-03 | Sea clutter model | Already implemented |
| 4-04 | Rain clutter | Already implemented |
| 4-05 | Radar shadow | Already implemented |

### Phase 5: Networking & Protocols

| Task | Description | Status |
|------|-------------|--------|
| 5-01 | NMEA VHW sentence | Done |
| 5-02 | NMEA MWV sentence | Done |
| 5-03 | AIS Message 5 (static/voyage data) | Done |
| 5-04 | AIS Message 21 (AtoN buoys) | Done |

## Files Modified (key changes only)

- `src/AIS.hpp/cpp` -- Message 5 and 21 generators, 6-bit ASCII encoding, scheduling
- `src/NMEA.hpp/cpp` -- VHW, MWV sentences; Message 5/21 scheduling in updateNMEA()
- `src/SimulationModel.hpp/cpp` -- Added getOtherShipLength/Breadth, getBuoyLat/Long
- `src/OtherShip.cpp` -- RadarCrossSection ini override
- `src/MMGPhysicsModel.hpp/cpp` -- Complete MMG physics implementation
- `src/PhysicsModel.hpp` -- Abstract physics model interface
- `src/graphics/Types.hpp` -- bc::graphics::Vec2/Vec3/Color/Matrix4/Quaternion
- `src/tests/` -- 68 Catch2 unit tests
- `.github/workflows/` -- CI test steps
- Various headers -- irrlicht.h removed, replaced with forward declarations + bc::graphics types
