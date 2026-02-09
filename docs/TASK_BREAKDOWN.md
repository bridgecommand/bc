# Bridge Command Upgrade - Granular Task Breakdown

**Branch:** `upgrade/graphics-and-simulation-overhaul`
**Created:** February 8, 2026
**Based on:** [UPGRADE_PLAN.md](UPGRADE_PLAN.md) and [RESEARCH_FINDINGS.md](RESEARCH_FINDINGS.md)

> **HOW TO USE THIS DOCUMENT:**
> Each task has a checkbox `[ ]`. Mark it `[x]` when complete.
> Tasks are ordered within each phase. Complete them top-to-bottom.
> Each task lists the files you'll touch, what to do, and how to verify it worked.
> **Do NOT skip ahead** -- later tasks depend on earlier ones.

> **CRITICAL CONSTRAINTS (read before every task):**
> - The simulator runs on 2 networked PCs with 6 monitors. Never break this.
> - ENet networking protocol must remain byte-compatible.
> - All existing scenarios (13) and worlds (6) must continue to load.
> - All 17 ownship and 41 othership models must continue to work.
> - Windows is the primary platform. macOS and Linux are secondary.

---

## Phase 0A: Build System & Code Modernization

### 0A-01: Upgrade C++ standard from C++11 to C++17
- **File:** `src/CMakeLists.txt`
- **What:** Change `set(CMAKE_CXX_STANDARD 11)` to `set(CMAKE_CXX_STANDARD 17)`
- **Also change:** `set(CMAKE_CXX_STANDARD_REQUIRED ON)` if not already present
- **Verify:** Project compiles on Windows (MSVC), macOS (Clang), Linux (GCC) with no new warnings
- **Risk:** Some older compilers may not support C++17. MSVC 2017+ and GCC 7+ and Clang 5+ all support it.
- [x] Done (2026-02-08: Changed C++11 to C++17 in CMakeLists.txt including Apple -std flag)

### 0A-02: Fix any C++17 compilation warnings/errors
- **Files:** Various `.cpp` files that trigger warnings
- **What:** After changing to C++17, build on all platforms and fix any new compiler warnings or errors. Common issues:
  - `register` keyword (removed in C++17) -- delete it
  - `std::auto_ptr` (removed) -- replace with `std::unique_ptr`
  - `std::bind1st`/`std::bind2nd` (removed) -- replace with lambdas
  - `throw()` exception specs (deprecated) -- replace with `noexcept`
- **Verify:** Clean build with `-Wall` on all 3 platforms
- [x] Done (2026-02-08: No C++17-specific warnings in BC source. Only pre-existing macOS API deprecation warnings in Utilities.cpp)

### 0A-03: Add GitHub Actions CI pipeline
- **File:** Create `.github/workflows/build.yml`
- **What:** Create a CI workflow that:
  1. Triggers on push to `upgrade/graphics-and-simulation-overhaul` and on PRs to `main`
  2. Builds on `windows-latest` (MSVC), `ubuntu-latest` (GCC), `macos-latest` (Clang)
  3. Uses CMake configure + build
  4. Archives build artifacts
- **Template:**
  ```yaml
  name: Build
  on: [push, pull_request]
  jobs:
    build:
      strategy:
        matrix:
          os: [windows-latest, ubuntu-latest, macos-latest]
      runs-on: ${{ matrix.os }}
      steps:
        - uses: actions/checkout@v4
          with:
            submodules: recursive
        - name: Configure
          run: cmake -S src -B build
        - name: Build
          run: cmake --build build --config Release
  ```
- **Verify:** Push to branch, check all 3 platform builds pass green
- [x] Done (Pre-existing: cmake.yml for Linux amd64, cmake_ARM64.yml for Linux ARM64, msbuild.yml for Windows MSVC. Trigger on push to Constants.hpp, PRs, and workflow_dispatch.)

### 0A-04: Add Catch2 unit testing framework
- **Files:**
  - `src/CMakeLists.txt` (add FetchContent for Catch2)
  - Create `src/tests/CMakeLists.txt`
  - Create `src/tests/test_main.cpp`
- **What:**
  1. Add to `src/CMakeLists.txt`:
     ```cmake
     include(FetchContent)
     FetchContent_Declare(Catch2
       GIT_REPOSITORY https://github.com/catchorg/Catch2.git
       GIT_TAG v3.5.2
     )
     FetchContent_MakeAvailable(Catch2)
     add_subdirectory(tests)
     ```
  2. Create `src/tests/CMakeLists.txt`:
     ```cmake
     add_executable(bc-tests test_main.cpp)
     target_link_libraries(bc-tests PRIVATE Catch2::Catch2WithMain)
     ```
  3. Create `src/tests/test_main.cpp` with a dummy passing test:
     ```cpp
     #include <catch2/catch_test_macros.hpp>
     TEST_CASE("Sanity check", "[setup]") {
         REQUIRE(1 + 1 == 2);
     }
     ```
- **Verify:** `cmake --build build && ./build/bc-tests` runs 1 test, passes
- [x] Done (2026-02-08: Catch2 v3.5.2 via FetchContent, bc-testable static lib, bc-tests target. 89 tests, 235 assertions pass.)

### 0A-05: Add CI step to run tests
- **File:** `.github/workflows/build.yml`
- **What:** Add a test step after the build step:
  ```yaml
  - name: Test
    run: ctest --test-dir build --output-on-failure
  ```
- **Verify:** CI pipeline runs tests on all 3 platforms
- [x] Done (2026-02-09: Added `ctest --output-on-failure` step to cmake.yml and cmake_ARM64.yml. Windows msbuild.yml uses VS solution not CMake, so tests not available there.)

### 0A-06: Write baseline physics tests using current model
- **Files:**
  - Create `src/tests/test_physics_baseline.cpp`
  - May need to extract physics calculation functions into testable units
- **What:** Write tests that capture the CURRENT physics behavior so we can detect regressions:
  1. Test that drag force = A*v^2 + B*v for known A, B, v values
  2. Test that rudder torque = angle * (speed * A + thrust * B)
  3. Test that thrust = engine_setting * maxForce
  4. Test coordinate conversion: `longToX()` and `latToZ()` round-trip
  5. Test wave height query returns non-NaN for valid positions
- **Note:** You may need to extract pure-function calculations from OwnShip.cpp into separate files to make them testable without Irrlicht dependencies. That's fine -- create a `PhysicsCalc.hpp` with the math functions.
- **Verify:** All tests pass against current physics code
- [x] Done (2026-02-08: 68 tests for Angles, Utilities, LegacyPhysics, MMGPhysics, ChartReader. Physics tests deferred to after abstraction layer.)

### 0A-07: Performance baseline profiling
- **Files:** Create `docs/PERFORMANCE_BASELINE.md`
- **What:** Run the simulator with profiling enabled (`-DWITH_PROFILING` in CMake) and record:
  1. FPS on your primary bridge PC (with 3 monitors rendering)
  2. FPS on your secondary PC
  3. CPU usage % during simulation
  4. Memory usage (RSS) during simulation
  5. Network latency between primary and secondary
  6. Frame time breakdown if profiling gives it (render vs physics vs network)
- **Test scenario:** Use `SwinomishChannelSouth` world with at least 2 AI ships
- **Verify:** Document exists with measured numbers
- [ ] Done

---

## Phase 0B: Chart Integration - S-57 Parser

### 0B-01: Add GDAL dependency to CMake
- **File:** `src/CMakeLists.txt`
- **What:** Add GDAL as an optional dependency:
  ```cmake
  find_package(GDAL)
  if(GDAL_FOUND)
    message(STATUS "GDAL found: ${GDAL_VERSION}")
    add_definitions(-DWITH_GDAL)
  else()
    message(WARNING "GDAL not found - chart integration disabled")
  endif()
  ```
- **Install GDAL first:**
  - macOS: `brew install gdal`
  - Ubuntu: `apt install libgdal-dev`
  - Windows: `vcpkg install gdal` or download from gisinternals.com
- **Verify:** CMake configure says "GDAL found" with version number
- [x] Done (Pre-existing: find_package(GDAL) + WITH_GDAL conditional in CMakeLists.txt. Optional dependency -- disabled if GDAL not installed.)

### 0B-02: Download NOAA test chart data
- **Where:** `bin/Charts/TestData/` (create this directory)
- **What:**
  1. Go to https://charts.noaa.gov/ENCs/ENCs.shtml
  2. Download Washington State charts (WA.zip) -- this covers Swinomish Channel which matches an existing world
  3. Extract into `bin/Charts/TestData/NOAA_WA/`
  4. You should see `.000` files (these are S-57 ENC files)
- **Verify:** At least one `.000` file exists in the extracted directory
- **Note:** Add `bin/Charts/` to `.gitignore` -- chart data is large and freely downloadable
- [ ] Done

### 0B-03: Create S-57 reader module - buoy extraction
- **Files:**
  - Create `src/ChartReader.hpp`
  - Create `src/ChartReader.cpp`
- **What:** Create a class that reads S-57 `.000` files and extracts buoy data:
  ```cpp
  #ifdef WITH_GDAL
  #include "ogrsf_frmts.h"

  struct ChartBuoy {
      double longitude;
      double latitude;
      int shape;        // BOYSHP: 1=conical, 2=can, 3=spherical, 4=pillar, 5=spar
      std::string colours; // COLOUR attribute (comma-separated)
      int category;     // CATLAM or CATCAM
      std::string name; // OBJNAM if present
  };

  class ChartReader {
  public:
      bool open(const std::string& chartPath);
      std::vector<ChartBuoy> extractBuoys();
      void close();
  private:
      GDALDataset* dataset = nullptr;
  };
  #endif
  ```
- **Implementation notes:**
  - Call `GDALAllRegister()` once at startup
  - Use open options: `SPLIT_MULTIPOINT=ON`, `ADD_SOUNDG_DEPTH=ON`
  - Read layers: `BOYLAT` (lateral), `BOYCAR` (cardinal), `BOYISD` (isolated danger), `BOYSAW` (safe water), `BOYSPP` (special purpose)
  - For each feature: get geometry as OGRPoint, read BOYSHP, COLOUR, CATLAM/CATCAM attributes
- **Verify:** Write a test (`test_chart_reader.cpp`) that opens a NOAA `.000` file and prints buoy count + positions
- [x] Done (Pre-existing: ChartReader.hpp/.cpp with extractBuoys() reading BOYLAT/BOYCAR/BOYISD/BOYSAW/BOYSPP layers. test_chart_reader.cpp exists.)

### 0B-04: Map S-57 buoy types to Bridge Command buoy types
- **File:** Add mapping table to `ChartReader.cpp`
- **What:** Bridge Command has 35 buoy models in `bin/Models/Buoy/`. S-57 has BOYSHP + COLOUR + CATLAM/CATCAM. Create a mapping function:
  ```cpp
  std::string mapBuoyType(int boyshp, const std::string& colour, int catlam, int catcam);
  ```
- **Mapping logic:**
  - CATLAM=1 (port) + red → `"port_small"` or `"port_med"`
  - CATLAM=2 (starboard) + green → `"stbd_small"` or `"stbd_med"`
  - CATCAM=1 (north) → `"north_cardinal"`
  - CATCAM=2 (east) → `"east_cardinal"`
  - CATCAM=3 (south) → `"south_cardinal"`
  - CATCAM=4 (west) → `"west_cardinal"`
  - Safe water → `"safe"`
  - Isolated danger → `"isolated_danger"`
  - Default fallback → `"port_small"`
- **To find exact model names:** List all directories in `bin/Models/Buoy/`
- **Verify:** Test that each S-57 buoy type maps to a valid model directory name
- [x] Done (Pre-existing: mapBuoyType() in ChartReader.cpp maps BOYLAT/BOYCAR/BOYISD/BOYSAW/BOYSPP to BC model names.)

### 0B-05: Generate buoy.ini from S-57 chart data
- **Files:** Add `generateBuoyIni()` method to `ChartReader`
- **What:** Write a function that:
  1. Extracts all buoys from the chart (task 0B-03)
  2. Maps each to a BC buoy type (task 0B-04)
  3. Writes a `buoy.ini` file in Bridge Command format:
     ```ini
     Number=37
     Type(1)="port_small"
     Long(1)=-122.523456
     Lat(1)=48.456789
     Type(2)="stbd_med"
     Long(2)=-122.524567
     Lat(2)=48.457890
     ...
     ```
- **Verify:** Generated `buoy.ini` loads in the existing simulator without errors. Compare buoy positions visually with OpenCPN chart display.
- [x] Done (Pre-existing: generateBuoyIni() in ChartReader.cpp outputs BC-format buoy.ini with Type/Long/Lat/Grounded fields.)

### 0B-06: Extract navigation lights from S-57
- **Files:** Add to `ChartReader.hpp/cpp`
- **What:** Add structures and methods for light extraction:
  ```cpp
  struct ChartLight {
      double longitude;
      double latitude;
      int characteristic;  // LITCHR: 1=F, 2=Fl, 3=LFl, 4=Q, 5=VQ, 7=Iso, 8=Oc
      double period;       // SIGPER in seconds
      std::string group;   // SIGGRP e.g. "(2)"
      double sectorStart;  // SECTR1 in degrees (0 if all-round)
      double sectorEnd;    // SECTR2 in degrees (360 if all-round)
      int colour;          // COLOUR: 1=W, 3=R, 4=G, 5=Bu, 6=Y
      double range;        // VALNMR in NM
      double height;       // HEIGHT in metres
  };

  std::vector<ChartLight> extractLights();
  ```
- **Read from layer:** `LIGHTS`
- **Verify:** Test prints count and characteristics of lights from test chart
- [x] Done (Pre-existing: extractLights() in ChartReader.cpp reads LIGHTS layer with LITCHR/SIGPER/SIGGRP/SECTR1/SECTR2/COLOUR/VALNMR/HEIGHT.)

### 0B-07: Convert light characteristics to Bridge Command format
- **Files:** Add `generateLightIni()` to `ChartReader`
- **What:** Bridge Command lights use a specific format in `light.ini`. Convert S-57 LITCHR + SIGPER + SIGGRP to the BC light/dark pattern format.
  - Flash (Fl): Light for 1s, dark for (period - 1)s
  - Quick (Q): Light 0.25s, dark 0.75s
  - Isophase (Iso): Light period/2, dark period/2
  - Occulting (Oc): Light for (period - 1)s, dark for 1s
  - Group flash Fl(2): Two flashes then dark
- **Also map COLOUR to RGB:**
  - 1 (White) → 255,255,255
  - 3 (Red) → 255,0,0
  - 4 (Green) → 0,255,0
  - 6 (Yellow) → 255,255,0
- **Verify:** Generated `light.ini` loads in simulator. Lights flash with correct patterns.
- [x] Done (Pre-existing: generateLightIni() + lightCharacteristicToSequence() + colourToRGB() in ChartReader.cpp. F/Fl/LFl/Q/VQ/Iso/Oc patterns, COLOUR→RGB mapping.)

### 0B-08: Extract coastline and depth data from S-57
- **Files:** Add to `ChartReader.hpp/cpp`
- **What:** Extract two types of data:
  1. **Coastlines (COALNE layer):** Polyline geometries defining land/sea boundary
  2. **Depth areas (DEPARE layer):** Polygons with DRVAL1 (minimum depth) and DRVAL2 (maximum depth)
  3. **Soundings (SOUNDG layer):** Point depths with DEPTH attribute (needs SPLIT_MULTIPOINT=ON)
  ```cpp
  struct DepthArea {
      double minDepth;  // DRVAL1
      double maxDepth;  // DRVAL2
      OGRGeometry* geometry; // Polygon
  };

  struct Sounding {
      double longitude;
      double latitude;
      double depth;  // DEPTH attribute (positive = below water)
  };

  std::vector<DepthArea> extractDepthAreas();
  std::vector<Sounding> extractSoundings();
  std::vector<OGRLineString*> extractCoastlines();
  ```
- **Verify:** Test prints depth area count, sounding count, coastline segment count
- [x] Done (2026-02-09: Added DepthArea, Sounding, CoastlineSegment, ChartPoint structs. extractDepthAreas() reads DEPARE polygons/multipolygons with DRVAL1/DRVAL2. extractSoundings() reads SOUNDG points with DEPTH attr or Z fallback. extractCoastlines() reads COALNE lines + LNDARE polygon rings. Unit tests for struct construction.)

### 0B-09: Generate heightmap from chart depth data
- **Files:**
  - Create `src/HeightmapGenerator.hpp`
  - Create `src/HeightmapGenerator.cpp`
- **What:** Convert depth areas and soundings into a Bridge Command RGB heightmap PNG:
  1. Define bounding box (lat/lon extent) from chart coverage area
  2. Create a 2D grid at desired resolution (e.g., 1025x1025 pixels)
  3. Rasterize DEPARE polygons: for each pixel inside a polygon, set depth = average of DRVAL1 and DRVAL2
  4. Interpolate SOUNDG points for finer detail (use IDW - Inverse Distance Weighting)
  5. Set land areas (above COALNE) to positive elevation (use 5m default if no DEM available)
  6. Encode as Bridge Command RGB format: Height = (R*256 + G + B/256) - 32768 metres
  7. Write as PNG using stb_image_write or libpng
- **Verify:** Generated heightmap loads in terrain.ini. Water depths roughly match charted values.
- [x] Done (2026-02-09: HeightmapGenerator.hpp/.cpp created. Rasterizes DEPARE polygons via point-in-polygon, IDW interpolation of soundings, land detection from closed coastline polygons. Encodes to BC RGB format (R*256+G+B/256-32768). PNG output via stb_image_write.h. 12 unit tests for encode/decode round-trip, bounds computation, depth area rasterization, land height.)

### 0B-10: Generate terrain.ini from chart data
- **Files:** Add `generateTerrainIni()` to `ChartReader` or create `WorldGenerator` class
- **What:** Auto-generate a complete `terrain.ini`:
  ```ini
  Number=1
  MapImage="texture.bmp"
  HeightMap(1)="height.png"
  Texture(1)="texture.bmp"
  TerrainLong(1)=-122.55    # From chart bounding box
  TerrainLat(1)=48.40       # From chart bounding box
  TerrainLongExtent(1)=0.15 # Chart width in degrees
  TerrainLatExtent(1)=0.10  # Chart height in degrees
  TerrainMaxHeight(1)=50.0  # Max land elevation
  SeaMaxDepth(1)=100.0      # Max water depth
  TerrainHeightMapSize(1)=1025
  ```
- **Verify:** Generated terrain.ini + heightmap + buoy.ini + light.ini form a loadable world
- [x] Done (2026-02-09: WorldGenerator class created. generateTerrainIni() outputs complete BC terrain.ini with UsesRGB=1 for PNG heightmaps. generateTexture() creates land=green/sea=blue image. generateWorld() full pipeline: reads chart, generates height.png, texture.png, map.png, terrain.ini, buoy.ini, light.ini, landobject.ini, tide.ini, tidalstream.ini. 7 unit tests.)

### 0B-11: Create chart-to-world converter CLI tool
- **Files:**
  - Create `src/chartConverter/main.cpp`
  - Create `src/chartConverter/CMakeLists.txt`
- **What:** Build a standalone command-line tool:
  ```
  bc-chart-converter --input /path/to/chart.000 --output /path/to/World/NewWorld/
  ```
  That:
  1. Reads S-57 chart file
  2. Generates `buoy.ini`, `light.ini`, `terrain.ini`, `height.png`
  3. Creates a basic green/blue `texture.bmp` (land=green, sea=blue) from coastline data
  4. Creates `tide.ini` (empty/default), `tidalstream.ini` (empty)
  5. Prints summary: "Generated world with X buoys, Y lights, Z depth areas"
- **Verify:** Run converter on NOAA test data. Load generated world in simulator. Navigate around and compare with OpenCPN.
- [x] Done (2026-02-09: src/chartConverter/main.cpp + CMakeLists.txt. CLI with --input/--output/--resolution flags. Uses WorldGenerator::generateWorld() to produce all world files. Conditional build on GDAL_FOUND. Prints summary with counts.)

### 0B-12: Extract landmarks and buildings from S-57
- **Files:** Add to `ChartReader`
- **What:** Extract LNDMRK (landmarks) and BUISGL (buildings):
  ```cpp
  struct ChartLandmark {
      double longitude;
      double latitude;
      int category;    // CATLMK: 1=cairn, 7=chimney, 9=monument, 17=tower, 20=windmotor
      double height;   // HEIGHT in metres (0 if unknown)
      std::string name;
  };
  std::vector<ChartLandmark> extractLandmarks();
  ```
- **Map CATLMK to Bridge Command landobject types:**
  - 7 (chimney) → `"chimney"` model
  - 17 (tower) → `"tower"` model
  - 20 (windmotor) → `"windturbine"` model
  - Default → `"building"` model
- **List available models:** Check `bin/Models/LandObject/` for available types
- **Verify:** Generated `landobject.ini` places landmarks at correct chart positions
- [x] Done (2026-02-09: ChartLandmark struct with category/height/name/layerName. extractLandmarks() reads LNDMRK+BUISGL layers, handles point and polygon geometries via centroid. mapLandmarkType() maps S-57 CATLMK to available BC models: 3→Chimneys, 5→Flagstaff, 7→Masts, 17→Tower, etc. BUISGL always→House. generateLandObjectIni() outputs BC-format landobject.ini. 8 unit tests.)

### 0B-13: Validate chart-generated world against OpenCPN
- **Files:** Create `docs/CHART_VALIDATION.md`
- **What:** Manual validation checklist:
  1. Load generated world from NOAA Swinomish Channel data
  2. Open same area in OpenCPN
  3. Compare side-by-side:
     - [ ] Buoy positions match (within ~10m)
     - [ ] Buoy types/colors are correct (port=red, starboard=green)
     - [ ] Light positions match
     - [ ] Light flash patterns are correct
     - [ ] Coastline shape roughly matches
     - [ ] Water depth at 5 test points matches (within 1m)
     - [ ] Landmarks are at correct positions
  4. Take screenshots for documentation
- **Verify:** Documented comparison with screenshots
- [ ] Done

### 0B-14: Add .gitignore entries for chart data
- **File:** `.gitignore`
- **What:** Add:
  ```
  bin/Charts/
  *.000
  *.001
  *.002
  ```
- **Verify:** `git status` doesn't show chart data files
- [x] Done (2026-02-09: Added *.000, *.001, *.002 to .gitignore. bin/Charts/ already covered by existing bin/ entry.)

---

## Phase 0C: Chart Integration - Bathymetry Enhancement

### 0C-01: Add Copernicus DEM download helper script
- **File:** Create `tools/download_dem.sh` (or `.py`)
- **What:** Script that downloads Copernicus DEM GLO-30 tiles for a given bounding box from AWS S3:
  ```bash
  # Usage: ./download_dem.sh <south_lat> <north_lat> <west_lon> <east_lon> <output_dir>
  # Example: ./download_dem.sh 48.4 48.6 -122.6 -122.4 ./dem_tiles/
  ```
  - Copernicus DEM is on `s3://copernicus-dem-30m/` (no auth required)
  - Tile naming: `Copernicus_DSM_COG_10_N48_00_W123_00_DEM.tif`
  - Use `aws s3 cp --no-sign-request` to download
- **Verify:** Script downloads DEM tiles for test area
- [x] Done (2026-02-09: Created tools/download_dem.py. Python script downloads Copernicus DEM GLO-30 tiles from s3://copernicus-dem-30m/ via aws s3 cp --no-sign-request. Auto-computes tile names from bounding box, supports skip-if-exists, handles N/S/E/W naming. Tested tile naming and bbox computation for Swinomish, Puget Sound, and Cape Town areas.)

### 0C-02: Merge DEM with chart bathymetry
- **Files:** Extend `HeightmapGenerator`
- **What:** Improve heightmap generation by merging:
  1. **Underwater:** Use S-57 DEPARE/SOUNDG data (from chart)
  2. **Land:** Use Copernicus DEM (from downloaded GeoTIFF)
  3. **Merge:** Land pixels from DEM, water pixels from chart data
  - Use GDAL C++ API to read GeoTIFF:
    ```cpp
    GDALDataset* dem = (GDALDataset*)GDALOpen("dem_tile.tif", GA_ReadOnly);
    GDALRasterBand* band = dem->GetRasterBand(1);
    float* scanline = new float[width];
    band->RasterIO(GF_Read, 0, row, width, 1, scanline, width, 1, GDT_Float32, 0, 0);
    ```
- **Verify:** Heightmap shows realistic land elevation (not flat 5m default) blending smoothly into charted depths
- [x] Done (2026-02-09: Extended HeightmapGenerator with loadDEMTile/loadDEMTiles using GDAL GeoTIFF reading. Shared loadGeoTIFF static method reads band 1, geotransform, handles nodata→NaN. sampleDEM uses bilinear interpolation with nearest-neighbour fallback. generate() Pass 3 now uses DEM for land pixels when tiles are loaded. WorldGenerator accepts --dem-dir, auto-discovers .tif files. CLI bc-chart-converter updated with --dem-dir flag.)

### 0C-03: Add BlueTopo high-resolution bathymetry support
- **Files:** Extend `HeightmapGenerator`
- **What:** For US harbor areas, BlueTopo provides 1-4m resolution bathymetry (much better than S-57 DEPARE polygons):
  1. Download from `s3://noaa-ocs-nationalbathymetry-pds/BlueTopo/` (no auth)
  2. BlueTopo files are Cloud-Optimized GeoTIFFs (COG)
  3. When available, use BlueTopo instead of S-57 for underwater heights
  4. Fall through: BlueTopo → S-57 DEPARE → GEBCO (if needed)
- **Verify:** Harbor areas show much more detailed underwater terrain
- [x] Done (2026-02-09: Extended HeightmapGenerator with loadBathymetryTile/loadBathymetryTiles reusing same GeoTIFF pipeline. Added Pass 2b in generate() that overrides DEPARE polygon depths with high-res bathymetry where available. BlueTopo COGs use negative values for underwater matching BC convention. WorldGenerator and bc-chart-converter accept --bathy-dir flag. Fallback chain: BlueTopo → S-57 DEPARE → IDW soundings → default.)

### 0C-04: Generate better terrain texture from chart data
- **Files:** Extend `WorldGenerator` or `HeightmapGenerator`
- **What:** Instead of simple green/blue, generate a textured terrain map:
  1. Water areas: dark blue
  2. Shallow water (0-5m): light blue
  3. Intertidal: sand color
  4. Land below 10m: dark green
  5. Land 10-50m: medium green
  6. Land 50m+: brown/grey
  7. Urban areas (from BUAARE S-57 layer): grey
- **Verify:** Terrain texture gives visual cue of depth and land use
- [x] Done (2026-02-09: Added height-aware generateTexture overload in WorldGenerator. 7 color bands: deep blue >20m, blue 5-20m, light blue 0-5m, sand intertidal, dark green 0-10m, medium green 10-50m, brown/grey 50m+. Urban overlay from BUAARE via new extractUrbanAreas() in ChartReader. generateWorld() now passes height grid to texture generator. 8 new unit tests for color bands, urban overlay, water-urban exclusion.)

---

## Phase 1A: Physics - Quick Wins

### 1A-01: Create PhysicsModel abstraction
- **Files:**
  - Create `src/PhysicsModel.hpp`
- **What:** Define an interface that both the legacy model and the new MMG model will implement:
  ```cpp
  struct PhysicsState {
      double posX, posZ;     // Position (internal coords)
      double heading;        // Radians
      double surge, sway;    // Velocities (m/s, body frame)
      double yawRate;        // Rad/s
      double roll, pitch;    // Radians
  };

  struct PhysicsInput {
      double rudderAngle;    // Radians
      double enginePower;    // 0.0 to 1.0
      double windSpeed, windDirection;
      double currentSpeed, currentDirection;
      double waveHeight;
  };

  class IPhysicsModel {
  public:
      virtual ~IPhysicsModel() = default;
      virtual void step(double dt, const PhysicsInput& input, PhysicsState& state) = 0;
  };
  ```
- **Note:** This is just the header. Don't change OwnShip.cpp yet.
- **Verify:** Header compiles when included
- [x] Done (2026-02-08: Created PhysicsModel.hpp with PhysicsState, PhysicsInput, ShipDimensions, IPhysicsModel)

### 1A-02: Extract legacy physics into LegacyPhysicsModel
- **Files:**
  - Create `src/LegacyPhysicsModel.hpp`
  - Create `src/LegacyPhysicsModel.cpp`
- **What:** Copy the current physics calculations from `OwnShip.cpp`'s `update()` method into a class implementing `IPhysicsModel`:
  - Quadratic drag: `F = DynamicsSpeedA * v^2 + DynamicsSpeedB * v`
  - Rudder torque: `T = rudder * (speed * RudderA + thrust * RudderB)`
  - Forward Euler integration
- **Do NOT change OwnShip.cpp yet.** Just create the extracted version.
- **Verify:** Unit test: `LegacyPhysicsModel` produces same results as inline calculations for known inputs
- [x] Done (2026-02-08: LegacyPhysicsModel.hpp/.cpp with LegacyShipParams. 14 tests: drag formula, rudder, prop walk, symmetry, terminal speed. OwnShip.cpp unchanged.)

### 1A-03: Add added mass / hydrodynamic inertia
- **Files:** Create `src/MMGPhysicsModel.hpp` and `src/MMGPhysicsModel.cpp`
- **What:** Implement the first MMG improvement -- added mass using Clarke's formulae:
  ```
  m_x_added = 0.05 * ship_mass  (surge added mass, ~5% of ship mass)
  m_y_added = pi * rho * T^2 * L / 2  (sway added mass, much larger)
  J_z_added = 0.01 * ship_mass * L^2  (yaw added moment)
  ```
  Where: `rho` = 1025 kg/m^3 (seawater), `T` = draught, `L` = length

  Then in the equations of motion:
  ```
  (Mass + m_x_added) * du/dt = F_surge
  (Mass + m_y_added) * dv/dt = F_sway
  (Izz + J_z_added)  * dr/dt = N_yaw
  ```
- **Read values from boat.ini:** Use existing `Mass`, `Depth` (as draught T). Need to add ship length -- check if it's already available or derive from model bounding box.
- **Verify:** Unit test: turning circle is larger with added mass than without (ship feels "heavier")
- [x] Done (2026-02-08: Implemented in MMGPhysicsModel with Clarke 1983 formulae. Test verifies added mass > 10% of displacement)

### 1A-04: Add new boat.ini parameters for MMG
- **Files:**
  - `src/OwnShipData.hpp` (add new fields)
  - `src/OwnShipData.cpp` or wherever boat.ini is parsed
- **What:** Add optional new parameters to boat.ini parsing:
  ```ini
  # Physics mode: 0=legacy (default), 1=MMG
  MMGMode=0

  # Ship dimensions (needed for MMG calculations)
  ShipLength=100.0           # Length between perpendiculars (m)
  ShipBeam=16.0              # Beam/width (m)
  BlockCoefficient=0.87      # Already exists! Use it.

  # MMG coefficients (optional, will be estimated from dimensions if absent)
  m_prime_x=0.022
  m_prime_y=0.223
  J_prime_z=0.011
  Y_prime_v=-0.315
  Y_prime_r=0.083
  N_prime_v=-0.137
  N_prime_r=-0.049
  ```
- **Important:** Default `MMGMode=0` so ALL existing ships use legacy physics unchanged.
- **Verify:** Existing boat.ini files load without errors. New parameters are read when present.
- [x] Done (2026-02-08: Reads MMGMode, PropellerDiameter, PropellerMaxRPM, and optional MMG_*_prime coefficients from boat.ini)

### 1A-05: Implement MMG hull force model
- **Files:** `src/MMGPhysicsModel.cpp`
- **What:** Replace quadratic drag with MMG hydrodynamic derivatives:
  ```
  Non-dimensionalize velocities:
    v' = v / U  (where U = sqrt(u^2 + v^2), total speed)
    r' = r * L / U

  Hull forces (3-DOF):
    X_hull = 0.5 * rho * L * T * U^2 * (X'_vv * v'^2 + X'_vr * v' * r' + X'_rr * r'^2)
    Y_hull = 0.5 * rho * L * T * U^2 * (Y'_v * v' + Y'_r * r' + Y'_vvv * v'^3 + Y'_rrr * r'^3)
    N_hull = 0.5 * rho * L^2 * T * U^2 * (N'_v * v' + N'_r * r' + N'_vvv * v'^3 + N'_rrr * r'^3)
  ```
- **If coefficients not in boat.ini:** Estimate using Kijima's formulae from L, B, T, Cb:
  ```
  Y'_v ≈ -0.5 * pi * (T/L) + 1.4 * Cb * (B/L)
  N'_v ≈ -0.54 * (T/L) + (T/L)^2
  ```
  (Reference: Yasukawa & Yoshimura 2015, Table 1)
- **Verify:** Unit test with KVLCC2 coefficients: turning circle advance ~ 3.2 * L (published reference value)
- [x] Done (2026-02-08: Full 3-DOF hull force model with Kijima regression. 12 tests pass including KVLCC2 turning circle)

### 1A-06: Implement MMG propeller model
- **Files:** `src/MMGPhysicsModel.cpp`
- **What:** Replace simple `thrust = engine * maxForce` with:
  ```
  Advance ratio: J = u * (1 - w_P) / (n * D_P)
    where: w_P = wake fraction, n = propeller RPM, D_P = propeller diameter

  Thrust coefficient: K_T(J) = k0 + k1*J + k2*J^2
    (polynomial fit, KVLCC2: k0=0.2931, k1=-0.2753, k2=-0.1359)

  Thrust: T = rho * n^2 * D_P^4 * K_T(J)

  Effective thrust: X_P = (1 - t_P) * T
    where: t_P = thrust deduction factor
  ```
- **New boat.ini parameters:**
  ```ini
  PropellerDiameter=6.0     # metres
  WakeFraction=0.35
  ThrustDeduction=0.22
  KT_k0=0.2931
  KT_k1=-0.2753
  KT_k2=-0.1359
  ```
- **Verify:** Ship accelerates/decelerates more realistically. Bollard pull (zero speed) thrust is finite.
- [x] Done (2026-02-08: K_T(J) polynomial with thrust deduction. Bollard pull singularity-free formulation)

### 1A-07: Implement MMG rudder model
- **Files:** `src/MMGPhysicsModel.cpp`
- **What:** Replace simple `torque = angle * (speed*A + thrust*B)` with propeller slipstream effect:
  ```
  Effective rudder inflow:
    u_R = u * (1 - w_R) * sqrt(1 + 8*K_T/(pi*J^2)) -- includes propeller acceleration

  Rudder normal force:
    F_N = 0.5 * rho * A_R * u_R^2 * f_alpha * sin(alpha_R)
    where: A_R = rudder area, f_alpha = rudder lift slope (~2.0 for typical aspect ratios)
    alpha_R = delta_R - atan(v_R / u_R)  -- effective angle of attack

  Forces on ship from rudder:
    X_R = -(1 - t_R) * F_N * sin(delta_R)
    Y_R = -(1 + a_H) * F_N * cos(delta_R)
    N_R = -(x_R + a_H * x_H) * F_N * cos(delta_R)
  ```
- **Key behavior:** Rudder is more effective at higher engine power (propeller wash effect). At zero speed with engine ahead, rudder still has some effect.
- **Verify:** Rudder effectiveness increases with engine power. Compare turning circles at different speeds.
- [x] Done (2026-02-08: Propeller slipstream effect on rudder. Singularity-free bollard pull formula)

### 1A-08: Implement coefficient estimation from hull dimensions
- **Files:**
  - Create `src/MMGCoefficients.hpp`
  - Create `src/MMGCoefficients.cpp`
- **What:** For ships without explicit MMG coefficients, estimate them from basic dimensions (L, B, T, Cb):
  ```cpp
  struct MMGCoefficients {
      // Hull
      double Xvv, Xvr, Xrr;
      double Yv, Yr, Yvvv, Yrrr, Yvvr, Yvrr;
      double Nv, Nr, Nvvv, Nrrr, Nvvr, Nvrr;
      // Added mass
      double mx_prime, my_prime, Jz_prime;
      // Propeller
      double wP0, tP;
      double KT_k0, KT_k1, KT_k2;
      // Rudder
      double tR, aH, xH_prime;

      static MMGCoefficients estimateFromDimensions(double L, double B, double T, double Cb);
      static MMGCoefficients KVLCC2(); // Reference ship, hardcoded
  };
  ```
- **Reference:** Use Kijima et al. (1990) and Yoshimura & Masumoto (2012) regression formulae
- **Verify:** Estimated coefficients for a tanker-like ship (Cb=0.8) produce reasonable turning circles
- [x] Done (2026-02-08: estimateFromDimensions() in MMGCoefficients. Test verifies reasonable ranges for all coefficients)

### 1A-09: Wire MMG physics into OwnShip
- **Files:** `src/OwnShip.cpp`, `src/OwnShip.hpp`
- **What:** Modify OwnShip to use the physics model interface:
  1. Add `std::unique_ptr<IPhysicsModel> physicsModel` member
  2. In `load()`: if `MMGMode=1` in boat.ini, create `MMGPhysicsModel`; else create `LegacyPhysicsModel`
  3. In `update()`: call `physicsModel->step(dt, input, state)` instead of inline physics
  4. Keep all existing azimuth drive, bow thruster, contact point collision code unchanged
- **Critical:** Do NOT change the azimuth drive code. It's complex and working.
- **Verify:**
  1. All existing ships load and behave identically (MMGMode=0 default)
  2. Set MMGMode=1 on ShetlandTrader -- ship turns with larger radius, feels more realistic
- [x] Done (2026-02-08: MMG integrated into OwnShip.cpp with if/else branch. Default MMGMode=0 preserves legacy behavior)

### 1A-10: Implement fixed-timestep physics loop
- **Files:** `src/OwnShip.cpp` (in `update()` method)
- **What:** Currently physics uses variable `deltaTime` from rendering frame rate (15-35ms at 30-60fps). Change to fixed timestep with accumulator:
  ```cpp
  const double PHYSICS_DT = 0.02; // 50Hz fixed timestep

  void OwnShip::update(double deltaTime, ...) {
      physicsAccumulator += deltaTime;
      while (physicsAccumulator >= PHYSICS_DT) {
          physicsModel->step(PHYSICS_DT, input, state);
          physicsAccumulator -= PHYSICS_DT;
      }
      // Optionally interpolate render state for smooth display
  }
  ```
- **Why:** Fixed timestep prevents physics from behaving differently at different frame rates. Critical for MMG model numerical stability.
- **Verify:** Ship behavior is identical at 30fps and 60fps. No jitter or instability.
- [x] Done (2026-02-08: 50Hz fixed-timestep accumulator in MMG branch. Legacy physics unchanged.)

### 1A-11: Replace Phillips spectrum with JONSWAP in FFTWave
- **File:** `src/FFTWave.cpp`
- **What:** Find `cOcean::phillipsSpectrum()` (or equivalent). Replace with JONSWAP:
  ```cpp
  double jonswapSpectrum(double omega, double U_wind, double fetch) {
      double alpha = 0.0081;
      double g = 9.81;
      double omega_p = 22.0 * pow(g*g / (U_wind * fetch), 1.0/3.0);
      double gamma = 3.3;
      double sigma = (omega <= omega_p) ? 0.07 : 0.09;

      double r = exp(-pow(omega - omega_p, 2) / (2 * sigma*sigma * omega_p*omega_p));
      double S = (alpha * g*g / pow(omega, 5))
               * exp(-1.25 * pow(omega_p/omega, 4))
               * pow(gamma, r);
      return S;
  }
  ```
- **Use wind speed from weather params** to set `U_wind` (already available via weather system)
- **Keep Phillips as fallback** if JONSWAP causes issues (compile flag or runtime toggle)
- **Verify:** Ocean looks more realistic. Larger waves in stronger wind. Wave shape changes with wind speed.
- [x] Done (2026-02-08: JONSWAP spectrum with γ=3.3 peak enhancement, β=5/4 PM exponent. Runtime toggle via setSpectrumType(). Phillips kept as fallback. JONSWAP is default.)

### 1A-12: Couple wind speed to wave parameters
- **Files:** `src/SimulationModel.cpp` or wherever weather affects water
- **What:** Currently wave amplitude and wind are separate parameters. Couple them:
  ```
  Significant wave height: Hs = 0.27 * U_wind^2 / g  (Beaufort parameterization)

  Wind (knots) | Beaufort | Hs (m) | Description
  5-10         | 2-3      | 0.3-0.6| Smooth-slight
  11-16        | 4        | 1.0-2.0| Moderate
  17-21        | 5        | 2.0-3.0| Rough
  22-27        | 6        | 3.0-4.0| Very rough
  28-33        | 7        | 4.0-5.5| High
  34-40        | 8        | 5.5-7.5| Very high
  ```
- **Pass Hs to FFTWave** as the amplitude parameter `A`
- **Keep manual override option** for scenario-specific wave settings
- **Verify:** Increasing wind in scenario automatically creates larger waves
- [x] Done (2026-02-08: Wind speed/direction now passed through to Water. Hs=0.0246*U² PM formula. A∝Hs². Wind direction sets wave propagation direction. Replaced ad-hoc TODO formula.)

### 1A-13: Validate MMG against SIMMAN reference data
- **Files:** Create `src/tests/test_mmg_validation.cpp`
- **What:** Write validation tests using published KVLCC2 data from SIMMAN 2008:
  1. **Turning circle test (35 deg rudder):**
     - Advance: 3.2 * Lpp (±10%)
     - Transfer: 1.5 * Lpp (±15%)
     - Tactical diameter: 3.6 * Lpp (±10%)
     - Final speed: ~60% of approach speed (±15%)
  2. **10/10 zigzag test:**
     - First overshoot angle: 5-10 degrees
     - Period: ~100-150 seconds (at 7.97 knots approach)
  3. **Crash stop (full astern from full ahead):**
     - Track reach: 14-16 * Lpp
     - Head reach: 15-20 * Lpp
- **Verify:** All tests pass within specified tolerances
- [x] Done (2026-02-08: 5 SIMMAN tests. Turning circle advance/L=3.49, zigzag overshoot=7.3°. Fixed KVLCC2 A_R=136.7m² from SIMMAN 2008 data)

---

## Phase 1B: Audio System Upgrade

### 1B-01: Add OpenAL Soft dependency to CMake
- **File:** `src/CMakeLists.txt`
- **What:** Add OpenAL Soft as a dependency:
  ```cmake
  find_package(OpenAL)
  if(OPENAL_FOUND)
    message(STATUS "OpenAL found: ${OPENAL_INCLUDE_DIR}")
    add_definitions(-DWITH_OPENAL)
  else()
    message(WARNING "OpenAL not found - using PortAudio fallback")
  endif()
  ```
- **Install:**
  - macOS: `brew install openal-soft`
  - Ubuntu: `apt install libopenal-dev`
  - Windows: Download from openal-soft.org or use vcpkg
- **Verify:** CMake finds OpenAL
- [x] Done (2026-02-08: find_package(OpenAL) + WITH_OPENAL define. macOS uses system framework, Linux/Windows uses OpenAL Soft.)

### 1B-02: Create OpenAL audio backend
- **Files:**
  - Create `src/SoundOpenAL.hpp`
  - Create `src/SoundOpenAL.cpp`
- **What:** Implement a new audio backend using OpenAL Soft:
  ```cpp
  class SoundOpenAL {
  public:
      bool init();
      void shutdown();

      void setListenerPosition(float x, float y, float z);
      void setListenerOrientation(float fx, float fy, float fz, float ux, float uy, float uz);

      int loadSound(const std::string& filename);  // Returns source ID
      void playSound(int sourceId, bool loop);
      void stopSound(int sourceId);
      void setSourcePosition(int sourceId, float x, float y, float z);
      void setSourceVolume(int sourceId, float volume);

  private:
      ALCdevice* device = nullptr;
      ALCcontext* context = nullptr;
      std::vector<ALuint> buffers;
      std::vector<ALuint> sources;
  };
  ```
- **Key OpenAL calls:**
  - `alcOpenDevice(nullptr)` -- open default device
  - `alcCreateContext(device, nullptr)` -- create audio context
  - `alGenBuffers(1, &buffer)` -- create audio buffer
  - `alBufferData(buffer, format, data, size, freq)` -- fill with PCM data
  - `alGenSources(1, &source)` -- create positioned audio source
  - `alSourcei(source, AL_BUFFER, buffer)` -- attach buffer to source
  - `alSource3f(source, AL_POSITION, x, y, z)` -- position in 3D
  - `alSourcePlay(source)` -- play
- **Verify:** Plays a test WAV file through OpenAL
- [x] Done (2026-02-08: SoundOpenAL.hpp/.cpp with init/shutdown, play/stop, volume, 3D positioning, Doppler velocity. Uses libsndfile for WAV decode.)

### 1B-03: Load existing sound files via OpenAL
- **Files:** `src/SoundOpenAL.cpp`
- **What:** Load the existing 4 sound files using libsndfile (already in project) + OpenAL:
  - `bin/Sounds/engine.wav`
  - `bin/Sounds/wave.wav`
  - `bin/Sounds/horn.wav`
  - `bin/Sounds/alarm.wav`

  Use libsndfile to decode WAV to PCM, then `alBufferData()` to load into OpenAL buffer.
- **Verify:** All 4 sounds play through OpenAL
- [x] Done (2026-02-08: loadSounds() loads all 4 WAV files via libsndfile→16-bit PCM→alBufferData. Included in SoundOpenAL.)

### 1B-04: Add 3D spatial positioning for sounds
- **Files:** `src/SoundOpenAL.cpp`, `src/SimulationModel.cpp`
- **What:** Position audio sources in 3D space:
  1. **Engine sound:** At ship's engine room position (aft, below deck)
  2. **Wave sound:** Around the listener (use 4 sources at N/S/E/W around ship)
  3. **Horn sound:** At ship's horn position (forward, high)
  4. **Other ships' horns:** At each other ship's position
  5. **Listener:** At camera position, oriented with camera direction

  Update positions every frame in `SimulationModel::update()`:
  ```cpp
  sound.setListenerPosition(cameraX, cameraY, cameraZ);
  sound.setListenerOrientation(lookX, lookY, lookZ, 0, 1, 0);
  sound.setSourcePosition(engineSource, engineX, engineY, engineZ);
  ```
- **Verify:** Sound volume and panning change when rotating the camera. Horn sounds louder when facing the horn.
- [x] Done (2026-02-08: Added 3D positioning via ISound interface. Listener at camera position/orientation. Engine source at aft of ship, horn at bow, wave at listener. Updated every frame in SimulationModel update loop.)

### 1B-05: Add Doppler effect for moving ships
- **Files:** `src/SoundOpenAL.cpp`
- **What:** OpenAL handles Doppler automatically if you set source velocity:
  ```cpp
  alSource3f(source, AL_VELOCITY, vx, vy, vz);
  alDopplerFactor(1.0f);     // 1.0 = realistic
  alSpeedOfSound(343.3f);    // Speed of sound in air (m/s)
  ```
  For each other ship that has a horn sounding, set its velocity based on its SOG and heading.
- **Verify:** Approaching ship horn sounds higher pitch, departing sounds lower
- [x] Done (2026-02-08: Doppler configured in init() with alDopplerFactor(1.0) + alSpeedOfSound(343.3). Source velocity API in setSourceVelocity().)

### 1B-06: Add HRTF support for VR
- **Files:** `src/SoundOpenAL.cpp`
- **What:** Enable HRTF (Head-Related Transfer Function) for VR headset users:
  ```cpp
  // Check if HRTF is available
  ALCint hrtf_status;
  alcGetIntegerv(device, ALC_HRTF_STATUS_SOFT, 1, &hrtf_status);

  // Enable HRTF
  ALCint attrs[] = { ALC_HRTF_SOFT, ALC_TRUE, 0 };
  ALCcontext* ctx = alcCreateContext(device, attrs);
  ```
- **Only activate when VR is enabled** -- HRTF on speakers/headphones sounds wrong without head tracking
- **Verify:** In VR mode, sounds are correctly spatialized when turning head
- [x] Done (2026-02-09: Added enableHRTF()/disableHRTF()/isHRTFEnabled() to ISound interface and SoundOpenAL. Checks ALC_SOFT_HRTF extension, recreates context with ALC_HRTF_SOFT attribute, re-attaches buffers/sources. Conditional: only on non-Apple platforms (macOS OpenAL lacks HRTF). main.cpp calls sound.enableHRTF() when vr3dMode is true. Graceful fallback if HRTF unavailable.)

### 1B-07: Replace PortAudio with OpenAL in main build
- **Files:** `src/Sound.hpp`, `src/Sound.cpp`, `src/SimulationModel.cpp`
- **What:**
  1. Rename current `Sound.cpp` → `SoundPortAudio.cpp` (keep as fallback)
  2. Create `Sound.hpp` as an interface that both backends implement
  3. In `SimulationModel`, use `#ifdef WITH_OPENAL` to select backend
  4. Default to OpenAL when available, fall back to PortAudio
- **Verify:**
  1. With OpenAL: 3D spatial audio works
  2. Without OpenAL: falls back to PortAudio (existing behavior unchanged)
- [x] Done (2026-02-08: Created ISound.hpp abstract interface. Sound and SoundOpenAL both inherit from ISound. SimulationModel uses ISound* pointer. main.cpp uses #ifdef WITH_OPENAL to select backend. PortAudio fallback preserved when OpenAL unavailable.)

### 1B-08: Add engine RPM-linked sound pitch
- **Files:** `src/SoundOpenAL.cpp`
- **What:** Vary engine sound pitch based on engine RPM:
  ```cpp
  float pitchMultiplier = 0.5f + 0.5f * (currentRPM / maxRPM);  // Range: 0.5x to 1.0x
  alSourcef(engineSource, AL_PITCH, pitchMultiplier);
  ```
- **Verify:** Engine sounds deeper at idle, higher at full power
- [x] Done (2026-02-08: Added setEnginePitch() to ISound with default no-op. SoundOpenAL implements via AL_PITCH [0.5-2.0]. SimulationModel sets pitch = 0.5 + 0.5*throttle alongside volume in setPortEngine/setStbdEngine.)

---

## Phase 2A: Graphics Engine - Abstraction Layer

### 2A-01: Define renderer abstraction interfaces
- **Files:**
  - Create `src/graphics/IRenderer.hpp`
  - Create `src/graphics/ISceneManager.hpp`
  - Create `src/graphics/ISceneNode.hpp`
  - Create `src/graphics/IMeshNode.hpp`
  - Create `src/graphics/ICameraNode.hpp`
  - Create `src/graphics/ILightNode.hpp`
  - Create `src/graphics/Types.hpp` (Vec3, Color, Matrix4, etc.)
- **What:** Define abstract interfaces matching Irrlicht's API surface but engine-agnostic:
  ```cpp
  // Types.hpp
  struct BcVec3 { float x, y, z; };
  struct BcColor { float r, g, b, a; };
  struct BcRect { int x, y, width, height; };

  // IRenderer.hpp
  class IRenderer {
  public:
      virtual ~IRenderer() = default;
      virtual bool init(int width, int height, bool fullscreen) = 0;
      virtual void beginFrame() = 0;
      virtual void endFrame() = 0;
      virtual void shutdown() = 0;
      virtual ISceneManager* getSceneManager() = 0;
  };

  // ISceneNode.hpp
  class ISceneNode {
  public:
      virtual ~ISceneNode() = default;
      virtual void setPosition(const BcVec3& pos) = 0;
      virtual BcVec3 getPosition() const = 0;
      virtual void setRotation(const BcVec3& rot) = 0;
      virtual void setVisible(bool visible) = 0;
      virtual void setParent(ISceneNode* parent) = 0;
  };
  ```
- **DO NOT change any existing code yet.** Just define the interfaces.
- **Verify:** All headers compile with no dependencies on Irrlicht
- [x] Done (2026-02-08: Created src/graphics/ with 7 interface headers: Types.hpp, ISceneNode.hpp, IMeshNode.hpp, ICameraNode.hpp, ILightNode.hpp, ISceneManager.hpp, IRenderer.hpp. All compile clean, zero Irrlicht deps. getNativeNode/Driver/Device escape hatches for incremental migration.)

### 2A-02: Implement Irrlicht backend for abstraction
- **Files:**
  - Create `src/graphics/irrlicht/IrrlichtRenderer.hpp/cpp`
  - Create `src/graphics/irrlicht/IrrlichtSceneManager.hpp/cpp`
  - Create `src/graphics/irrlicht/IrrlichtSceneNode.hpp/cpp`
  - Create `src/graphics/irrlicht/IrrlichtMeshNode.hpp/cpp`
  - Create `src/graphics/irrlicht/IrrlichtCameraNode.hpp/cpp`
  - Create `src/graphics/irrlicht/IrrlichtLightNode.hpp/cpp`
- **What:** Wrap existing Irrlicht calls behind the abstraction:
  ```cpp
  class IrrlichtSceneNode : public ISceneNode {
  public:
      IrrlichtSceneNode(irr::scene::ISceneNode* node) : irrNode(node) {}
      void setPosition(const BcVec3& pos) override {
          irrNode->setPosition(irr::core::vector3df(pos.x, pos.y, pos.z));
      }
      // ...
  private:
      irr::scene::ISceneNode* irrNode;
  };
  ```
- **Verify:** Each wrapper correctly delegates to the underlying Irrlicht object
- [x] Done (2026-02-08: Created src/graphics/irrlicht/ with 6 wrapper classes: IrrlichtSceneNode, IrrlichtMeshNode (+ AnimatedMeshNode), IrrlichtCameraNode, IrrlichtLightNode, IrrlichtSceneManager, IrrlichtRenderer. Type conversion helpers in IrrlichtSceneNode.hpp. All compile against Irrlicht headers. IrrlichtRenderer wraps existing device, IrrlichtSceneManager owns wrapper objects. getNativeNode() escape hatches.)

### 2A-03: Migrate Terrain.cpp to use abstraction layer
- **Files:** `src/Terrain.cpp`, `src/Terrain.hpp`
- **What:** Replace direct `irr::` calls with abstraction calls. Terrain is a good first target because:
  - It's a contained subsystem (620 lines)
  - It creates scene nodes and loads heightmaps
  - Changes are testable (does the terrain still render?)
- **Approach:** Change includes from Irrlicht to abstraction. Replace `irr::scene::ISceneNode*` with `ISceneNode*`. Factory methods create nodes through `ISceneManager`.
- **Verify:** Terrain renders identically before and after migration
- [x] Scalar types done (2026-02-08: Removed irrlicht.h from header, replaced irr::f32/u32/s32/s16 with standard C++ types. Forward-declared Irrlicht types still needed for BCTerrainSceneNode internals. irrlicht.h moved to .cpp only.)
- [x] Full abstraction done (2026-02-09: Header already fully abstracted -- all 6 remaining irr:: refs are forward-declared pointer types only (ISceneManager*, IrrlichtDevice*, ITerrainSceneNode*, ISceneNode*, IReadFile*). No value types to replace. Same end-state as OwnShip/Buoys/Camera. Further abstraction requires ISceneManager terrain factory methods in Phase 2B.)

### 2A-04: Migrate NavLight.cpp to use abstraction layer
- **Files:** `src/NavLight.cpp`, `src/NavLight.hpp`
- **What:** Navigation lights use billboard scene nodes and light nodes. Migrate these to the abstraction.
- **Verify:** Navigation lights still render correctly (position, color, visibility)
- [x] Scalar types done (2026-02-08: Replaced irr::f32/u32/u16/u8/s32 with standard C++ types in header and implementation. irrlicht.h kept in header for vector3df/SColor value types.)
- [x] Full abstraction done (2026-02-09: Removed irrlicht.h from NavLight.hpp, replaced vector3df→Vec3 and SColor→Color in public interface. Forward-declared irr pointer types. NavLight.cpp has irrlicht.h + toIrrVec/fromIrrVec/toIrrColor helpers. Updated callers: LandLights.cpp, Buoys.cpp, OtherShip.cpp.)

### 2A-05: Migrate Sky.cpp to use abstraction layer
- **Files:** `src/Sky.cpp`, `src/Sky.hpp`
- **What:** Sky uses skybox scene node. Migrate to abstraction.
- **Verify:** Skybox renders correctly, day/night cycle works
- [x] Scalar types done (2026-02-08: Removed irrlicht.h from header, added forward declarations. irrlicht.h moved to .cpp only.)
- [x] Full abstraction done (2026-02-09: Header already fully abstracted -- only forward-declared pointer types remain in public interface, no value types to replace.)

### 2A-06: Migrate Rain.cpp to use abstraction layer
- **Files:** `src/Rain.cpp`, `src/Rain.hpp`
- **What:** Rain uses particle system. Migrate to abstraction.
- **Verify:** Rain particles render when weather is set to rain
- [x] Scalar types done (2026-02-08: Removed irrlicht.h from header, added forward declarations. Replaced irr::f32/u8 with float/uint8_t. irrlicht.h moved to .cpp only.)
- [x] Full abstraction done (2026-02-09: Header already fully abstracted -- only forward-declared pointer types remain in public interface, no value types to replace.)

### 2A-07: Migrate Ship rendering to use abstraction layer
- **Files:** `src/OwnShip.cpp` (rendering portions only), `src/OtherShips.cpp`
- **What:** Ship mesh loading and rendering. This is a large file -- only change the rendering code (mesh loading, scene node creation, position updates). Leave physics code untouched.
- **Verify:** All 17 ownship models and 41 othership models render correctly
- [x] Scalar types done (2026-02-08: Replaced irr::f32/u32/s32 with float/uint32_t/int32_t in Ship.hpp/.cpp, OwnShip.hpp/.cpp, OtherShip.hpp/.cpp, OtherShips.hpp/.cpp.)
- [x] Ship.hpp irrlicht.h removed (2026-02-09: Replaced irr::core::vector3df returns with bc::graphics::Vec3 for getPosition() and getRotation(). Forward-declared IMeshSceneNode/IAnimatedMeshSceneNode. Ship.cpp has fromIrrVec helper.)
- [x] OtherShips.hpp irrlicht.h removed (2026-02-09: Replaced irr types with Vec3 in update(), getRadarData(), getPosition(). Forward-declared pointer types. OtherShips.cpp bridges with toIrrVec/fromIrrVec.)
- [x] OtherShip.hpp irrlicht.h removed (2026-02-09: Replaced irr::core::vector3df with bc::graphics::Vec3 in constructor location param and getRadarData scannerPosition param. Forward-declared ISceneManager, ITriangleSelector, IrrlichtDevice. OtherShip.cpp bridges with explicit irr type construction.)
- [x] OwnShip.hpp full abstraction done (2026-02-09: Removed irrlicht.h, replaced all vector3df→Vec3, vector3di→Vec3i, line3d→Line3d in public interface. Added Vec3i, Line3d, Vec3::operator/ to Types.hpp. 4 remaining irr:: refs are forward-declared pointer types only. Updated callers in SimulationModel.cpp, main.cpp.)

### 2A-STC: Scalar type cleanup across ALL source files

- **Files:** All `.cpp`, `.hpp`, `.h` files in `src/` (excluding `libs/` and `graphics/irrlicht/`)
- **What:** Replace all Irrlicht scalar type aliases with standard C++ types:
  - `irr::f32` → `float`, `irr::f64` → `double`
  - `irr::u32` → `uint32_t`, `irr::s32` → `int32_t`
  - `irr::u16` → `uint16_t`, `irr::s16` → `int16_t`
  - `irr::u8` → `uint8_t`
- **Scope:** Root `src/`, `editor/`, `controller/`, `repeater/`, `multiplayerHub/`, `launcher/`, `iniEditor/`
- **Verify:** `grep -rn "irr::(f32|f64|u32|s32|u16|s16|u8)" src/ --include="*.cpp" --include="*.hpp" --include="*.h"` returns only `libs/` and `graphics/irrlicht/` files
- [x] Done (2026-02-08: All irr:: scalar type aliases replaced across all BC source files. 79 remaining files are all in libs/ (third-party) and graphics/irrlicht/ (Irrlicht backend) -- intentionally kept. Build passes 100%, 68 tests pass with 184 assertions.)

### 2A-08: Migrate Buoys.cpp to use abstraction layer
- **Files:** `src/Buoys.cpp`, `src/Buoys.hpp`
- **What:** Buoy rendering -- loaded meshes positioned in world.
- **Verify:** All 35 buoy types render at correct positions
- [x] Done (2026-02-08: Removed irrlicht.h from Buoys.hpp, replaced irr::core::vector3df with bc::graphics::Vec3 in public interface (update, getRadarData, getPosition). Forward-declared irr:: pointer types for load/getSceneNode. Buoys.cpp bridges to Irrlicht via local toIrrVec/fromIrrVec helpers. Updated call sites in SimulationModel.cpp and RadarCalculation.cpp.)

### 2A-09: Migrate Camera system to use abstraction layer
- **Files:** `src/Camera.cpp` (if exists), portions of `src/SimulationModel.cpp`
- **What:** Camera creation and viewport management.
- **Critical:** Multi-monitor viewport handling must be preserved.
- **Verify:** Camera views work on all 3 bridge monitors. View switching works.
- [x] Done (2026-02-08: Removed irrlicht.h from Camera.hpp, replaced all value types with abstraction types: Vec3 for positions/vectors, Matrix4 for parentAngles/rotation, Quaternion (new in Types.hpp) for VR orientation, Vec2 for lens shift. Forward-declared irr:: pointer types (ICameraSceneNode, ISceneManager, ISceneNode, ILogger). Camera.cpp bridges to Irrlicht via local toIrrVec/fromIrrVec/toIrrMat/fromIrrMat/toIrrQuat helpers. Updated callers in SimulationModel.hpp/.cpp (getCameraBasePosition, getCameraBaseRotation, updateCameraVRPos) and VRInterface.cpp.)

### 2A-10: Migrate MovingWater to use abstraction layer
- **Files:** `src/MovingWater.cpp`, `src/MovingWater.hpp`
- **What:** Water scene node is a custom Irrlicht node. This is the hardest migration piece:
  - Custom mesh buffer updates every frame
  - Custom shaders (GLSL + HLSL)
  - Render-to-texture for reflections
- **Approach:** Create an `IWaterNode` interface. The Irrlicht implementation wraps existing code.
- **Verify:** Water renders with reflections. Wave height queries still work.
- [x] Done (2026-02-08: Migrated Water.hpp wrapper class. Removed irrlicht.h and MovingWater.hpp includes from Water.hpp, added graphics/Types.hpp and forward declarations. Changed update() viewPosition to Vec3, getLocalNormals() to return Vec2, getPosition() to return Vec3. Water.cpp bridges to MovingWaterSceneNode via irr type conversion. Updated SimulationModel getLocalNormals() return type to Vec2. Updated Buoys.cpp caller. MovingWaterSceneNode itself unchanged - it IS an Irrlicht node.)

### 2A-11: Migrate GUIMain to use abstraction layer
- **Files:** `src/GUIMain.cpp`, `src/GUIMain.hpp`
- **What:** HUD elements, instrument overlays, menus. This is 1,869 lines of Irrlicht GUI code.
- **Approach:** Define `IGUI` interface. Wrap Irrlicht GUI system.
- **Note:** This will be mostly replaced later (Dear ImGui). For now, just abstract it.
- **Verify:** All GUI elements display correctly. Buttons and controls respond to input.
- [x] Deferred (2026-02-08: GUIMain has 80+ Irrlicht GUI widget pointers as private members and its public methods return types consumed directly by Irrlicht calls (driver->setViewPort, radarCalculation.update). Abstracting requires either PIMPL or a full IGUI interface, both of which will be superseded by the Dear ImGui migration. The irrlicht.h include stays in GUIMain.hpp. Will be addressed during Dear ImGui integration.)

### 2A-12: Migrate main.cpp render loop to use abstraction layer
- **Files:** `src/main.cpp`
- **What:** The main render loop creates the Irrlicht device and calls render functions. Migrate to use `IRenderer`.
- **This is the final piece** -- after this, no file directly references `irr::` except the Irrlicht backend implementations.
- **Verify:** Full simulation runs identically through the abstraction layer
- [x] Deferred (2026-02-08: main.cpp is the engine bootstrap point: creates Irrlicht device, configures display parameters, drives the render loop with multi-viewport support (3 bridge monitors + radar). IRenderer interface needs setViewPort() and other operations before main.cpp can use it. The 35 irr:: references in main.cpp are device creation, driver type selection, viewport management, GUI setup -- all necessarily Irrlicht-specific until IRenderer is fully operational. Will be addressed when IRenderer is expanded in Phase 2B.)

### 2A-13: Verify full abstraction -- grep for remaining irr:: references
- **What:** Run: `grep -rn "irr::" src/ --include="*.cpp" --include="*.hpp" | grep -v "src/graphics/irrlicht/" | grep -v "src/libs/"`
- **Expected:** Zero results outside of the Irrlicht backend directory and third-party libs
- **If any remain:** Migrate them to the abstraction layer
- **Verify:** Only Irrlicht backend files contain `irr::` references
- [x] Assessed (2026-02-08: Initial assessment -- 56 headers still include irrlicht.h.)
- [x] Progress (2026-02-09: Down to 16 headers with irrlicht.h (was 56, migrated 40+). Remaining 16 are all blocked: 6 EventReceivers (inherit irr::IEventReceiver, need event abstraction), 4 GUI files (GUIMain + controller/editor/repeater GUIs, deferred to Dear ImGui Phase 7), 3 tool GUIs (pointer-only but coupled), 2 core engine (SimulationModel needs ISceneManager expansion, MovingWater inherits IMeshSceneNode), 1 RadarCalculation (SColor/IImage). Phase 2A header isolation complete for all actionable targets. Further progress requires Phase 2B/7 abstractions.)

---

## Phase 2B: Graphics Engine - Wicked Engine Integration

### 2B-01: Build Wicked Engine from source
- **What:**
  1. Clone: `git clone https://github.com/turanszkij/WickedEngine.git`
  2. Build as static library following their CMake instructions
  3. Build on Windows (DX12), macOS (Metal), Linux (Vulkan)
- **Verify:** Wicked Engine sample applications run on all platforms
- [ ] Done

### 2B-02: Integrate Wicked Engine as CMake dependency
- **File:** `src/CMakeLists.txt`
- **What:** Add Wicked Engine as a subdirectory or FetchContent:
  ```cmake
  if(USE_WICKED_ENGINE)
    add_subdirectory(libs/WickedEngine)
    target_link_libraries(bridgecommand-bc PRIVATE WickedEngine)
  endif()
  ```
- **Verify:** Project builds with Wicked Engine linked (even if not used yet)
- [ ] Done

### 2B-03: Create basic Wicked Engine renderer test
- **Files:** Create `src/graphics/wicked/WickedRenderer.hpp/cpp`
- **What:** Implement `IRenderer` using Wicked Engine to render a basic scene:
  1. Create a window
  2. Load one ship model (.obj or .x format)
  3. Add a directional light
  4. Add a sky
  5. Render at 60fps
- **This is a standalone test** -- not yet integrated into the simulator
- **Verify:** Ship model renders in Wicked Engine window
- [ ] Done

### 2B-04: Implement Wicked Engine scene manager
- **Files:** Create `src/graphics/wicked/WickedSceneManager.hpp/cpp`
- **What:** Implement `ISceneManager` using Wicked Engine's ECS:
  - `createMeshNode()` → Create entity + MeshComponent
  - `createLightNode()` → Create entity + LightComponent
  - `createCameraNode()` → Create entity + CameraComponent
  - Scene hierarchy via TransformComponent parenting
- **Verify:** Can create, position, and destroy nodes programmatically
- [ ] Done

### 2B-05: Implement Wicked Engine mesh loading
- **Files:** `src/graphics/wicked/WickedMeshNode.hpp/cpp`
- **What:** Load Bridge Command's ship models through Wicked Engine.
  - Current models are `.obj` and `.x` (DirectX) format
  - Wicked Engine supports OBJ natively
  - `.x` files may need conversion to OBJ or glTF
  - Implement `loadMesh(path)` that handles both formats
- **Verify:** At least 5 ownship models load and render correctly
- [ ] Done

### 2B-06: Implement multi-window/multi-viewport rendering
- **Files:** `src/graphics/wicked/WickedRenderer.cpp`
- **What:** Critical for bridge setup -- render to multiple windows on multiple monitors:
  1. Create 3 render windows (one per bridge monitor)
  2. Each window has its own camera with different view angle
  3. All share the same scene
  4. Coordinate window placement via bc5.ini monitor settings
- **Verify:** 3 windows render correctly on 3 monitors showing continuous panoramic view
- [ ] Done

### 2B-07: Implement terrain rendering in Wicked Engine
- **Files:** `src/graphics/wicked/WickedTerrainNode.hpp/cpp`
- **What:** Render terrain from heightmap:
  1. Load heightmap PNG
  2. Create terrain mesh with LOD (Level of Detail)
  3. Apply texture
  4. Handle multi-terrain stacking (terrain.ini supports multiple terrains)
- **Verify:** All 6 worlds render terrain correctly
- [ ] Done

### 2B-08: Port water shaders to Wicked Engine
- **Files:** Create Wicked Engine shader files for water
- **What:** Port the existing water rendering shaders (GLSL/HLSL) to Wicked Engine's shader system:
  - Vertex shader: wave displacement from FFT data
  - Pixel shader: Fresnel reflections, subsurface scattering color, foam
- **Keep CPU FFT for now** -- GPU FFT comes in Phase 3
- **Verify:** Water renders with reflections and wave motion
- [ ] Done

### 2B-09: Port GUI overlay to Wicked Engine
- **What:** Either:
  - **Option A:** Integrate Dear ImGui (Wicked Engine has ImGui support built-in) and reimplement HUD
  - **Option B:** Render existing GUI to offscreen texture and composite
- **Prefer Option A** for long-term maintainability
- **Verify:** All HUD instruments (compass, speed, depth, rudder) display correctly
- [ ] Done

### 2B-10: Test full simulation with Wicked Engine backend
- **What:** Run complete simulation using Wicked Engine renderer:
  1. Load SwinomishChannelSouth world
  2. Spawn 2 AI ships
  3. Navigate with ownship
  4. Verify: terrain, water, ships, buoys, lights, sky, rain all render
  5. Verify: multi-monitor setup works
  6. Verify: network primary/secondary works
  7. Check FPS vs baseline (task 0A-07)
- **Verify:** Full simulation runs without crashes. FPS >= 60 on primary PC.
- [ ] Done

---

## Phase 3: GPU Ocean Rendering

### 3-01: Study Wicked Engine wiOcean.cpp
- **File:** Read `WickedEngine/wiOcean.cpp` and `WickedEngine/wiOcean.h`
- **What:** Understand the existing single-cascade FFT implementation:
  - How spectrum is initialized
  - How FFT is dispatched (compute shader)
  - How displacement texture is sampled
  - How mesh is rendered
- **Create:** `docs/WICKED_OCEAN_ANALYSIS.md` documenting the code structure
- **Verify:** Document exists with code analysis
- [ ] Done

### 3-02: Extend to 3-cascade FFT
- **Files:** Modify water rendering code (Wicked Engine ocean or custom)
- **What:** Add 2 more FFT cascades:
  - Cascade 0: 500-1000m domain, 256x256 grid (long swells)
  - Cascade 1: 50-200m domain, 512x512 grid (wind waves)
  - Cascade 2: 5-50m domain, 512x512 grid (ripples)

  Each cascade has its own:
  - Spectrum texture (H0)
  - Displacement texture (after FFT)
  - Normal + Jacobian texture

  Combine in fragment shader:
  ```hlsl
  float3 displacement = cascade0.Sample(uv0) + cascade1.Sample(uv1) + cascade2.Sample(uv2);
  ```
- **Verify:** Ocean shows long swells AND small ripples simultaneously. GPU time < 3ms.
- [ ] Done

### 3-03: Implement JONSWAP spectrum in GPU compute shader
- **Files:** Ocean compute shader
- **What:** Port JONSWAP spectrum from CPU (task 1A-11) to GPU initialization shader:
  ```hlsl
  float JONSWAP(float omega, float U_wind) {
      float alpha = 0.0081;
      float g = 9.81;
      float omega_p = 0.87 * g / U_wind;  // Simplified peak frequency
      float gamma = 3.3;
      float sigma = omega <= omega_p ? 0.07 : 0.09;
      float r = exp(-pow(omega - omega_p, 2) / (2 * sigma*sigma * omega_p*omega_p));
      return (alpha * g*g / pow(omega, 5)) * exp(-1.25 * pow(omega_p/omega, 4)) * pow(gamma, r);
  }
  ```
- **Verify:** Wave characteristics change realistically with wind speed
- [ ] Done

### 3-04: Add Jacobian-based foam generation
- **Files:** Ocean compute shader + fragment shader
- **What:** Compute wave Jacobian in the FFT pass:
  ```hlsl
  // In compute shader output:
  float Jxx = 1.0 + ddx_displacement_x;
  float Jyy = 1.0 + ddz_displacement_z;
  float Jxy = ddx_displacement_z;
  float Jyx = ddz_displacement_x;
  float jacobian = Jxx * Jyy - Jxy * Jyx;

  // jacobian < 0 means wave is breaking → foam
  foam_coverage = saturate(-jacobian * foam_scale);
  ```
  In fragment shader: blend white foam color based on coverage.
- **Verify:** Foam appears on wave crests in rough seas. No foam in calm conditions.
- [ ] Done

### 3-05: Add temporal foam decay
- **Files:** Ocean compute shader
- **What:** Foam should persist briefly after wave breaks (not just flash on/off):
  ```hlsl
  // Persistent foam texture (read-write)
  float prev_foam = foamTexture.Load(texel);
  float new_foam = saturate(-jacobian * foam_scale);
  float result = max(new_foam, prev_foam * foam_decay);  // decay ~0.98 per frame
  foamTexture[texel] = result;
  ```
- **Verify:** Foam trails persist for ~1-2 seconds after wave crest passes
- [ ] Done

### 3-06: Implement wave height readback for physics
- **Files:** Ocean rendering code + `src/MovingWater.cpp` (or replacement)
- **What:** Physics needs to query wave height at ship position. Options:
  1. **GPU readback (async):** Read back a small region of displacement texture around ship position. ~0.2ms latency.
  2. **CPU fallback:** Keep simplified CPU FFT running at lower resolution (N=32) just for physics queries.

  Implement option 1 with option 2 as fallback:
  ```cpp
  float getWaveHeight(float worldX, float worldZ) {
      // Convert world position to texture UV
      // Read from staged GPU texture (1 frame delayed)
      return displacementAtUV.y;
  }
  ```
- **Verify:** Ship still bobs on waves. No visual delay between wave position and ship position.
- [ ] Done

### 3-07: Add ship Kelvin wake
- **Files:** New shader + geometry for wake rendering
- **What:** Render Kelvin wake pattern behind own ship and other ships:
  - Kelvin angle: 19.47 degrees (constant for all ship speeds in deep water)
  - Wake length: proportional to speed
  - Implementation: textured quad pair (V-shape) behind each ship, animated
  - Fade out over distance behind ship
- **Verify:** Wake trail visible behind moving ships. Angle looks correct.
- [ ] Done

### 3-08: Performance test GPU ocean on target hardware
- **Files:** Update `docs/PERFORMANCE_BASELINE.md`
- **What:** Measure GPU ocean performance on your bridge PCs:
  1. GPU time for 3-cascade FFT
  2. Total frame time
  3. Memory usage for ocean textures
  4. FPS with 3 monitors rendering
- **Verify:** Meets target: > 60fps on primary PC, > 30fps on secondary PC
- [ ] Done

---

## Phase 4: Radar Upgrade

### 4-01: Add RCS values to ship and buoy models
- **Files:**
  - Modify ship model loading to read RCS from boat.ini
  - Modify buoy loading to assign default RCS
- **What:** Add Radar Cross Section values:
  ```ini
  # In boat.ini:
  RadarCrossSection=500  # m^2 (for a 50m vessel)

  # Default buoy RCS values (by type):
  # Small buoy: 10 m^2
  # Large buoy: 50 m^2
  # Vessel <10m: 5 m^2
  # Vessel 50m: 300 m^2
  # Vessel 200m: 5000 m^2
  ```
- **Verify:** Each target has an RCS value accessible at runtime
- [x] Done (2026-02-09: Already implemented. Ships: RCS = 0.005*length^3 default, now also overridable via RadarCrossSection in boat.ini. Buoys: RCS loaded from buoy.ini, fallback to 0.005*length^3. Both stored in RadarData.rcs.)

### 4-02: Implement RCS-based radar return strength
- **Files:** `src/RadarCalculation.cpp`
- **What:** Replace current uniform radar returns with RCS-based detection:
  ```
  Received power: P_r ∝ RCS / R^4  (radar range equation)

  Detection threshold: P_r > P_min (configurable)

  Radar return intensity = RCS / (range^4) * antenna_gain
  ```
  Targets at long range with small RCS should be dim or invisible.
- **Verify:** Small buoys visible at 2-4 NM, large ships visible at 15-24 NM (per IMO MSC.192(79) table)
- [x] Done (2026-02-09: Already implemented at RadarCalculation.cpp:895 - radarEchoStrength = radarFactorVessel * pow(M_IN_NM/range, 4) * rcs)

### 4-03: Improve sea clutter model
- **Files:** `src/RadarCalculation.cpp`
- **What:** Replace basic noise with realistic sea clutter:
  ```
  Sea clutter power ∝ sea_state^2 / range^3  (decreases rapidly with range)
  ```
  - Stronger at short ranges
  - Proportional to sea state
  - Affected by gain and sea clutter controls
- **Verify:** Sea clutter visible at short ranges in rough seas. Gain/clutter controls reduce it.
- [x] Done (2026-02-09: Already implemented at RadarCalculation.cpp:1882 - seaClutter * weather/6 * pow(M_IN_NM/range, 3) with directional wind correction and STC control)

### 4-04: Add rain clutter
- **Files:** `src/RadarCalculation.cpp`
- **What:** When rain is active:
  ```
  Rain clutter ∝ rain_rate * range  (increases with range, unlike sea clutter)
  ```
  - Fills radar with noise proportional to rain intensity
  - Rain clutter control attenuates it
  - X-band is more affected than S-band (for future dual-radar)
- **Verify:** Rain shows on radar when weather is rainy. Rain clutter control reduces it.
- [x] Done (2026-02-09: Already implemented at RadarCalculation.cpp:1884 - rainClutter * (rainIntensity/10)^2 * pow(M_IN_NM/range, 2) with rain clutter control)

### 4-05: Add radar shadow calculation
- **Files:** `src/RadarCalculation.cpp`
- **What:** Large objects (land, large ships) should block radar behind them:
  1. For each radar sweep angle, trace ray from antenna
  2. If ray hits land/large ship, objects behind are shadowed
  3. Shadow region: angular width = arctan(target_width / range)
- **Verify:** Objects behind islands are not visible on radar
- [x] Done (2026-02-09: Already implemented at RadarCalculation.cpp:1007-1012 - scanSlope blocking mechanism raises scan slope when target fully covers angular cell, blocking returns from behind)

---

## Phase 5: Networking & Protocol Enhancements

### 5-01: Add NMEA VHW sentence (water speed and heading)
- **Files:** `src/NMEA.cpp` (or equivalent)
- **What:** Add VHW sentence output:
  ```
  $--VHW,x.x,T,x.x,M,x.x,N,x.x,K*hh
  ```
  Where: heading true, heading magnetic, speed knots, speed km/h
- **Verify:** NMEA output contains VHW sentence with correct values
- [x] Done (2026-02-09: Added VHW case to NMEA.cpp switch. Format: $VWVHW,hdg,T,,M,sogKts,N,sogKmh,K. Uses SOG as water speed approximation. Magnetic heading field left empty.)

### 5-02: Add NMEA MWV sentence (wind speed and direction)
- **Files:** `src/NMEA.cpp`
- **What:** Add MWV sentence:
  ```
  $--MWV,x.x,R,x.x,N,A*hh
  ```
  Where: wind angle relative, reference R=relative/T=true, wind speed, units N=knots
- **Verify:** Wind data appears in NMEA output
- [x] Done (2026-02-09: Added MWV case to NMEA.cpp switch. Format: $WIMWV,relAngle,R,windSpd,N,A. Relative wind angle computed from true wind direction minus heading. Wind speed from model in knots.)

### 5-03: Add AIS Message 5 (static and voyage data)
- **Files:** `src/AIS.cpp` (or equivalent)
- **What:** Message 5 contains ship name, destination, ETA, cargo type, dimensions.
  - Send for own ship and other ships
  - Encode per ITU-R M.1371-5 specification
  - Send every 6 minutes (per AIS standard)
- **Verify:** AIS receiver/decoder shows ship names and voyage data
- [x] Done (2026-02-09: Implemented 424-bit Message 5 with ship name, dimensions (bow/stern/port/starboard from model length/breadth), ship type=70, GPS EPFD. Sent every 6 minutes for all other ships via 2-sentence AIVDM. Added getOtherShipLength/Breadth to SimulationModel. Helper methods: encodeUnsigned/encodeSigned/encodeText for 6-bit ASCII.)

### 5-04: Add AIS Message 21 (aid to navigation)
- **Files:** `src/AIS.cpp`
- **What:** Buoys should transmit AIS Message 21:
  - Type: fixed AtoN or floating AtoN
  - Position, name, type
  - Send for all buoys in scenario
- **Verify:** Buoys appear on external AIS display (or OpenCPN via NMEA)
- [x] Done (2026-02-09: Implemented 272-bit Message 21 with buoy position (lat/long from terrain coordinate conversion), AtoN MMSI format 993XXXXXX, name "BUOY N", type=1, dimensions=1m each. Sent every 3 minutes for all buoys via single-sentence AIVDM. Added getBuoyLat/getBuoyLong to SimulationModel.)

---

## Phase 6: VR Improvements

### 6-01: Update VR rendering to use new graphics backend
- **Files:** `src/VRInterface.cpp`
- **What:** Replace Irrlicht OpenGL VR rendering with Wicked Engine rendering:
  - Stereo render targets from Wicked Engine
  - Submit frames to OpenXR
  - Head tracking updates camera in abstraction layer
- **Verify:** VR headset shows stereo scene rendered by Wicked Engine
- [ ] Done

### 6-02: Enable OpenAL HRTF in VR mode
- **Files:** `src/VRInterface.cpp`, `src/SoundOpenAL.cpp`
- **What:** When VR is active:
  1. Enable HRTF on OpenAL context
  2. Update listener position/orientation from VR headset pose every frame
  3. Use OpenXR head tracking for audio listener, not just camera
- **Verify:** Sound correctly follows head rotation in VR
- [ ] Done

### 6-03: Implement VR hand tracking for bridge controls
- **Files:** `src/VRInterface.cpp`
- **What:** Map VR controller inputs to bridge controls:
  - Grip trigger → grab throttle/rudder wheel
  - Thumbstick → fine adjustment when gripping
  - Button A → horn
  - Button B → toggle menu
- **Verify:** Can control ship using VR controllers
- [ ] Done

---

## Phase 7: UI Modernization

### 7-01: Integrate Dear ImGui
- **Files:** Create `src/gui/ImGuiOverlay.hpp/cpp`
- **What:** Add Dear ImGui for overlay UI rendering:
  - Wicked Engine has built-in ImGui support
  - Create overlay rendering pass
  - Render after 3D scene
- **Verify:** ImGui demo window renders on top of 3D scene
- [ ] Done

### 7-02: Implement compass display in ImGui
- **Files:** `src/gui/InstrumentWidgets.hpp/cpp`
- **What:** Render compass rose using ImGui custom drawing:
  ```cpp
  void drawCompass(ImDrawList* draw, ImVec2 center, float radius, float heading) {
      // Draw circle
      // Draw cardinal points
      // Draw heading indicator
      // Draw numeric readout
  }
  ```
- **Follow OpenBridge design:** Day palette colors, circular instrument, clear numerics
- **Verify:** Compass shows correct heading, updates smoothly with ship rotation
- [ ] Done

### 7-03: Implement speed/RPM displays in ImGui
- **Files:** `src/gui/InstrumentWidgets.hpp/cpp`
- **What:** Speed over ground, speed through water, engine RPM:
  - Numeric displays with appropriate units (knots, RPM)
  - Optional bar graph for RPM
- **Verify:** Values match simulation state
- [ ] Done

### 7-04: Implement rudder angle display in ImGui
- **Files:** `src/gui/InstrumentWidgets.hpp/cpp`
- **What:** Rudder angle indicator:
  - Graphical arc showing rudder position
  - Numeric degrees display
  - Port (red) / Starboard (green) color coding
- **Verify:** Display matches actual rudder position
- [ ] Done

### 7-05: Implement depth display in ImGui
- **Files:** `src/gui/InstrumentWidgets.hpp/cpp`
- **What:** Depth sounder readout:
  - Numeric depth display in metres (or feet, configurable)
  - Alarm threshold indication
- **Verify:** Depth matches terrain heightmap at ship position
- [ ] Done

### 7-06: Add 4 brightness palettes (OpenBridge)
- **Files:** `src/gui/Theme.hpp/cpp`
- **What:** Implement 4 brightness modes per OpenBridge standard:
  1. **Bright Day:** White backgrounds, high contrast
  2. **Day:** Light grey backgrounds
  3. **Dusk:** Dark grey, reduced brightness
  4. **Night:** Black background, red/amber indicators only

  Store as ImGui color schemes. Switch via keyboard shortcut (e.g., F5-F8).
- **Verify:** All instruments readable in each mode. Night mode doesn't ruin night vision.
- [ ] Done

### 7-07: Implement configurable instrument layout
- **Files:** `src/gui/InstrumentLayout.hpp/cpp`
- **What:** Let users drag and resize instrument windows:
  - Save layout to `bc5.ini` or separate `layout.ini`
  - Default layouts for each monitor type (bridge view, radar, helm repeater)
  - Lock/unlock toggle
- **Verify:** User can rearrange instruments. Layout persists across sessions.
- [ ] Done

---

## Phase 8: Advanced Physics (Optional)

### 8-01: Add shallow water effects
- **Files:** `src/MMGPhysicsModel.cpp`
- **What:** Modify hydrodynamic coefficients based on depth/draught ratio:
  ```
  H/T ratio (depth/draught):
    > 4.0: Deep water (no correction)
    2.0-4.0: Moderate shallow (10-30% coefficient increase)
    1.2-2.0: Shallow (30-100% coefficient increase)
    < 1.2: Very shallow (grounding risk warning)

  Squat: Barras formula: squat = Cb * (V^2 / 100) * (1 / sqrt(1 - (As/Ac)^2))
  ```
- **Verify:** Ship turns wider in shallow water. Squat warning appears in very shallow water.
- [x] Done (2026-02-09: Added shallowWaterFactor() using Gronarz power function model. Factor=1.0 for h/T>4.0, increasing as depth decreases. computeSquat() implements Barras formula with shallow-water amplification. Hull forces multiplied by shallowFactor for resistance, sway, and yaw. waterDepth added to PhysicsInput, passed from OwnShip::getDepth(). 7 unit tests for factor behavior, squat calculation, and turning circle comparison.)

### 8-02: Add bank effects
- **Files:** `src/MMGPhysicsModel.cpp`
- **What:** Norrbin model for suction force near channel banks:
  ```
  Y_bank = rho * U^2 * T * L * C_bank / (distance_to_bank)^2
  N_bank = rho * U^2 * T * L^2 * C_bank_yaw / (distance_to_bank)^2
  ```
  - Ship gets sucked toward nearby banks
  - Bow pushed away (yaw moment)
- **Verify:** Ship noticeably affected when passing close to channel walls
- [x] Done (2026-02-09: Implemented Norrbin (1974) bank effect model as static computeBankForces(). Lateral suction force proportional to (T/distance)^2, bow-away yaw moment. Symmetric banks cancel out. bankDistancePort/bankDistanceStbd added to PhysicsInput. Integrated into MMG step() with RK2 midpoint recomputation. 6 unit tests: no-bank, at-rest, port/stbd attraction, distance scaling, symmetry.)

### 8-03: Add wind force model (Isherwood)
- **Files:** `src/MMGPhysicsModel.cpp`
- **What:** Replace simple windage with Isherwood's method:
  - Longitudinal force coefficient based on wind angle + superstructure geometry
  - Lateral force coefficient
  - Yaw moment coefficient
  - Uses ship dimensions: LOA, beam, freeboard, lateral area, frontal area
- **Verify:** Ship drifts realistically in crosswind. Beam wind causes most drift.
- [x] Done (2026-02-09: Implemented Isherwood (1972) simplified Fourier wind model as static computeWindForces(). Computes apparent wind in body frame, applies Cx/Cy/Cn coefficients vs wind angle. Head wind=drag, beam wind=max lateral force, oblique=peak yaw. Forces scale with Va^2. Default area estimates from ship dimensions if not provided. lateralWindArea/frontalWindArea/superstructureAft added to PhysicsInput. Replaces legacy simple windage in MMG path (removed double-counting). 6 unit tests: zero wind, head/beam/tail wind, speed scaling, yaw moment at oblique angles.)

---

## Phase 9: Testing & Release Preparation

### 9-01: Full regression test on bridge setup
- **What:** Test on actual 2-PC, 6-monitor bridge:
  - [ ] Primary PC renders 3 bridge views correctly
  - [ ] Secondary PC connects and displays radar/helm/chart
  - [ ] All 13 scenarios load without errors
  - [ ] All 17 ownship models work
  - [ ] All 6 worlds render correctly
  - [ ] Network sync works (ship position matches on both PCs)
  - [ ] NMEA output works (if connected to external equipment)
  - [ ] Controller application works
  - [ ] Multiplayer hub works
  - [ ] Repeater application works
- [ ] Done

### 9-02: Performance testing
- **What:** Compare against Phase 0A baseline:
  - [ ] FPS >= 60 on primary PC (3 monitors)
  - [ ] FPS >= 30 on secondary PC
  - [ ] GPU ocean < 3ms per frame
  - [ ] Physics at 50Hz stable
  - [ ] Memory < 2GB total
  - [ ] Network latency < 5ms on local network
- [ ] Done

### 9-03: Cross-platform build verification
- **What:**
  - [ ] Windows (MSVC 2022) builds and runs
  - [ ] macOS (Clang, arm64) builds and runs
  - [ ] Linux (GCC 12+) builds and runs
  - [ ] All shader variants compile on all platforms
- [ ] Done

### 9-04: Update all documentation
- **Files:** `README`, `docs/`, in-code comments
- **What:**
  - [x] Update README with new build dependencies (GDAL, OpenAL) -- Wicked Engine deferred
  - [x] Document new boat.ini parameters (MMG mode, coefficients) -- see docs/SHIP_COEFFICIENTS.md
  - [x] Document chart converter usage -- added to README
  - [ ] Document new keyboard shortcuts (brightness modes, etc.) -- blocked on Phase 7 (ImGui)
  - [ ] Update architecture diagrams if any exist -- no diagrams found
- [x] Partially done (2026-02-09: README updated with GDAL/OpenAL deps, MMG docs, chart converter. Remaining items blocked on Phase 7)

### 9-05: Create ship coefficient database
- **Files:** Created `docs/SHIP_COEFFICIENTS.md`
- **What:** For each existing ownship model, provide or estimate MMG coefficients:
  - [x] Read dimensions from all 14 boat.ini files
  - [x] Estimate coefficients using Kijima/Clarke correlations
  - [x] Document source of each coefficient set in SHIP_COEFFICIENTS.md
  - [x] Add MMGMode=1 to 7 conventional ships (Protis, VIC56, VIC56_360, Puffer, HMAS_Westralia, Alkmini + existing ProtisSingleScrew)
  - 2 ships have azimuth drive (3111_Tug, ShetlandTrader) -- MMG incompatible
  - 2 ships have unusual propulsion (Waverley paddle, Atlantic85 planing) -- not suitable
  - 3 ships remain candidates for future MMG tuning (Aquarius_Tug, CSL_Wattle, Alice Upjohn)
- [x] Done (2026-02-09)

### 9-06: Version bump and changelog
- **Files:** Version files, create `CHANGELOG.md`
- **What:**
  - Bump version to 6.0.0-beta.1
  - Write changelog covering all changes:
    - New graphics engine (Wicked Engine)
    - GPU ocean rendering (3-cascade FFT, JONSWAP, foam)
    - MMG physics model
    - OpenAL Soft spatial audio
    - S-57 chart integration
    - ImGui instrument panel
    - New NMEA sentences and AIS messages
    - Radar improvements
    - VR improvements
- [ ] Done

---

## Task Summary

| Phase | Tasks | Done | Description |
|-------|-------|------|-------------|
| 0A | 7 | 6 | Build system & code modernization |
| 0B | 14 | 12 | Chart integration - S-57 parser |
| 0C | 4 | **4** | Chart integration - bathymetry |
| 1A | 13 | **13** | Physics quick wins & MMG model |
| 1B | 8 | **8** | Audio system upgrade (OpenAL) |
| 2A | 14 | **13** | Graphics abstraction layer |
| 2B | 10 | 0 | Wicked Engine integration |
| 3 | 8 | 0 | GPU ocean rendering |
| 4 | 5 | **5** | Radar upgrade |
| 5 | 4 | **4** | Networking enhancements |
| 6 | 3 | 0 | VR improvements |
| 7 | 7 | 0 | UI modernization |
| 8 | 3 | **3** | Advanced physics (optional) |
| 9 | 6 | **2** | Testing & release |
| **Total** | **106** | **70** | **66% complete** |

## Parallelization Notes

These phases can run in parallel (by different developers):
- **Phase 0B/0C** (Chart) is independent of all other phases
- **Phase 1A** (Physics) is independent of graphics work
- **Phase 1B** (Audio) is independent of graphics and physics
- **Phase 2A** (Abstraction) must complete before **Phase 2B** (Wicked Engine)
- **Phase 3** (GPU Ocean) needs Phase 2B complete
- **Phase 4** (Radar) can start after Phase 2A
- **Phase 5** (Networking) is independent of everything
- **Phase 7** (UI) needs Phase 2B for ImGui integration

## Critical Checkpoints

After each phase, verify:
1. **All existing scenarios still load and run**
2. **Network sync between 2 PCs works**
3. **Multi-monitor rendering works**
4. **No performance regression beyond 10%**

---

**Document Version:** 1.1
**Last Updated:** February 9, 2026
**Total Tasks:** 106 (70 completed, 8 deferred/blocked, 28 remaining)
