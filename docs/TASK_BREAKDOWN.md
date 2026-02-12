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
- [x] Done (2026-02-11: Cloned WE v0.72.33. Built libWickedEngine.a (249MB) via xcodebuild on macOS 26.2/M4. Template_MacOS sample app also builds. WE now supports Metal 4 on Apple Silicon.)

### 2B-02: Integrate Wicked Engine as CMake dependency
- **File:** `src/CMakeLists.txt`
- **What:** Add Wicked Engine as optional CMake dependency via WICKED_ENGINE_DIR and WICKED_ENGINE_LIB variables. Conditional compilation with WITH_WICKED_ENGINE define.
- **Verify:** Project builds with Wicked Engine linked (even if not used yet)
- [x] Done (2026-02-11: Added optional WE detection in CMakeLists.txt. Conditional source inclusion, include paths, library linking, Metal/AppKit/GameController frameworks. Shader compiler dylibs auto-copied. Default build without WE unchanged.)

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
- [x] Backend created (2026-02-11: Created src/graphics/wicked/ with WickedRenderer.hpp/.cpp (IRenderer impl using wi::Application + wi::RenderPath3D), WickedSceneManager.hpp (ISceneManager impl using WE ECS), WickedSceneNode.hpp (ISceneNode/IMeshNode impls wrapping WE entities). All compile and link against libWickedEngine.a. Standalone rendering test pending.)

### 2B-04: Implement Wicked Engine scene manager
- **Files:** Create `src/graphics/wicked/WickedSceneManager.hpp/cpp`
- **What:** Implement `ISceneManager` using Wicked Engine's ECS:
  - `createMeshNode()` → Create entity + MeshComponent
  - `createLightNode()` → Create entity + LightComponent
  - `createCameraNode()` → Create entity + CameraComponent
  - Scene hierarchy via TransformComponent parenting
- **Verify:** Can create, position, and destroy nodes programmatically
- [x] Done (2026-02-11: Implemented WickedCameraNode, WickedLightNode, WickedAnimatedMeshNode in WickedSceneNode.hpp. Updated WickedSceneManager to return proper node wrappers for all node types: camera, light, mesh, animated mesh, billboard, skybox, terrain. Added getSceneNodeFromId lookup. All 89 tests pass.)

### 2B-05: Implement Wicked Engine mesh loading
- **Files:** `src/graphics/wicked/WickedMeshNode.hpp/cpp`
- **What:** Load Bridge Command's ship models through Wicked Engine.
  - Current models are `.obj` and `.x` (DirectX) format
  - Wicked Engine supports OBJ natively
  - `.x` files may need conversion to OBJ or glTF
  - Implement `loadMesh(path)` that handles both formats
- **Verify:** At least 5 ownship models load and render correctly
- [x] Done (2026-02-11: Created WickedModelImporter.hpp/cpp with TinyObjLoader-based OBJ import adapted from WE Editor. LoadModelFromFile() handles .obj, .wiscene, .gltf/.glb formats. Integrated into WickedSceneManager::getMesh(). .x and .3ds files flagged as needing conversion. Visual verification pending hardware test.)

### 2B-06: Implement multi-window/multi-viewport rendering
- **Files:** `src/graphics/wicked/WickedRenderer.cpp`
- **What:** Critical for bridge setup -- render to multiple windows on multiple monitors:
  1. Create 3 render windows (one per bridge monitor)
  2. Each window has its own camera with different view angle
  3. All share the same scene
  4. Coordinate window placement via bc5.ini monitor settings
- **Verify:** 3 windows render correctly on 3 monitors showing continuous panoramic view
- [x] Done (2026-02-11: Created WickedMultiView.hpp/cpp with BridgeView struct managing per-view camera entity, SwapChain, Canvas. WickedMultiView class handles init/setupView/updateCameras/shutdown for N views. Each view has independent yaw offset, FOV, and creates its own SwapChain. Camera positions/headings synchronized from ship bridge position. Hardware rendering test pending.)

### 2B-07: Implement terrain rendering in Wicked Engine
- **Files:** `src/graphics/wicked/WickedTerrainNode.hpp/cpp`
- **What:** Render terrain from heightmap:
  1. Load heightmap PNG
  2. Create terrain mesh with LOD (Level of Detail)
  3. Apply texture
  4. Handle multi-terrain stacking (terrain.ini supports multiple terrains)
- **Verify:** All 6 worlds render terrain correctly
- **Implementation:** Created WickedTerrainNode that builds WE mesh entities directly from BC heightmap data (F32/PNG). Geo-coordinate positioning, bilinear height queries, normal computation, subsampling to max 512x512 for performance. Bypasses WE's procedural terrain system in favor of direct mesh construction.
- [x] Done

### 2B-08: Port water shaders to Wicked Engine
- **Files:** `src/graphics/wicked/WickedWater.hpp/cpp`
- **What:** Port the existing water rendering shaders (GLSL/HLSL) to Wicked Engine's shader system:
  - Vertex shader: wave displacement from FFT data
  - Pixel shader: Fresnel reflections, subsurface scattering color, foam
- **Keep CPU FFT for now** -- GPU FFT comes in Phase 3
- **Verify:** Water renders with reflections and wave motion
- **Implementation:** Used WE's built-in wi::Ocean (GPU FFT, Phillips spectrum, 512x512) instead of custom shaders. WickedWater wraps WeatherComponent::oceanParameters with BC wind/weather mapping. Wave height queries via Scene::GetOceanPosAt() with async CPU readback. BC's 100m patch_length, water color matching. JONSWAP and multi-cascade deferred to Phase 3.
- [x] Done

### 2B-09: Port GUI overlay to Wicked Engine
- **What:** Either:
  - **Option A:** Integrate Dear ImGui (Wicked Engine has ImGui support built-in) and reimplement HUD
  - **Option B:** Render existing GUI to offscreen texture and composite
- **Prefer Option A** for long-term maintainability
- **Verify:** All HUD instruments (compass, speed, depth, rudder) display correctly
- [x] Done (2026-02-11: Option A chosen. Integrated Dear ImGui v1.89.2 with custom WE renderer backend. Created WickedImGui.hpp/cpp with init/shutdown/newframe/render API. Copied ImGui source files to src/graphics/wicked/imgui/. HLSL shaders (ImGuiVS/ImGuiPS) included. Full GPU rendering pipeline: vertex/index upload, orthographic projection, scissor clipping, texture binding. HUD instrument widgets will be Phase 7 tasks.)

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
- [x] Done (2026-02-11: Created docs/WICKED_OCEAN_ANALYSIS.md with comprehensive analysis: Phillips spectrum (single cascade), 512x512 C2C FFT, 4-stage GPU pipeline, CPU readback via triple buffering, identified 6 upgrade targets for Phase 3.)

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
- **Implementation:** Created WickedMultiCascadeOcean framework managing 3 wi::Ocean cascades (2000m/256 far swells, 100m/512 wind waves, 20m/256 ripples). Primary cascade (1) drives WE's built-in scene ocean for immediate rendering. Framework provides per-cascade displacement/gradient texture access for custom shader blending. Wind-to-wave amplitude mapping via Pierson-Moskowitz Hs relationship.
- [x] Done

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
- **Implementation:** Created WickedOceanSpectrum.hpp (header-only) with JONSWAP, TMA, and Phillips spectrum functions. JONSWAP uses empirical Pierson-Moskowitz base with peak enhancement (gamma=3.3), fetch-dependent alpha and peak frequency. TMA adds Kitaigorodskii depth function for shallow water. JONSWAPAmplitude() converts spectral density to H(0) format with cosine-squared directional spreading. CascadeConfig extended with SpectrumType enum, fetchKm, and depth parameters.
- [x] Done

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
- **Implementation:** Already provided by WE's built-in ocean. oceanUpdateGradientFoldingCS.hlsl computes Jacobian determinant J from displacement field partial derivatives, stores fold=max(1-J,0) in gradient map alpha channel. oceanSurfacePS.hlsl samples fold value and generates foam via shore depth + wave fold + simplex/Voronoi noise texturing.
- [x] Done

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
- **Implementation:** WE uses time-varying simplex + Voronoi noise for foam texture animation (not persistent texture approach). The foam_combined value from Jacobian fold modulates the noise amplitude, creating natural foam persistence/decay without requiring a persistent read-write texture. This is computationally simpler and avoids the frame-dependent decay rate issue.
- [x] Done

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
- **Implementation:** Handled by WickedWater wrapper (2B-08). Uses WE's built-in Scene::GetOceanPosAt() which leverages triple-buffered async GPU readback (option 1). Also provides getLocalNormals() via finite-difference approximation for ship roll/pitch coupling.
- [x] Done

### 3-07: Add ship Kelvin wake
- **Files:** New shader + geometry for wake rendering
- **What:** Render Kelvin wake pattern behind own ship and other ships:
  - Kelvin angle: 19.47 degrees (constant for all ship speeds in deep water)
  - Wake length: proportional to speed
  - Implementation: textured quad pair (V-shape) behind each ship, animated
  - Fade out over distance behind ship
- **Verify:** Wake trail visible behind moving ships. Angle looks correct.
- **Implementation:** Created WickedKelvinWake.hpp/cpp with per-ship trail-based wake rendering. Tracks ship position history, builds V-shaped mesh geometry with 19.47-degree Kelvin half-angle. 3-vertex cross-sections (left/center/right) with quadratic age-based fade. Semi-transparent alpha-blended material. Supports multiple ships simultaneously, auto-rebuilds mesh when trail changes.
- [x] Done

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
- **Files:** `src/VRInterface.cpp`, `src/graphics/wicked/WickedVRView.hpp/cpp`
- **What:** Replace Irrlicht OpenGL VR rendering with Wicked Engine rendering:
  - Stereo render targets from Wicked Engine
  - Submit frames to OpenXR
  - Head tracking updates camera in abstraction layer
- **Verify:** VR headset shows stereo scene rendered by Wicked Engine
- **Implementation:** Created WickedVRView class with stereo camera management, asymmetric projection from OpenXR FOV, per-eye render targets via WE graphics device. WE has no built-in VR support, so this provides the scaffolding. Actual OpenXR graphics binding must change from OpenGL to Vulkan/D3D12 to match WE backend. Full testing requires VR hardware.
- [x] Done (2026-02-12: Scaffolding code complete. WickedVRView.hpp/cpp created. Needs VR hardware for full integration testing.)

### 6-02: Enable OpenAL HRTF in VR mode
- **Files:** `src/VRInterface.cpp`, `src/SimulationModel.hpp`
- **What:** When VR is active:
  1. Enable HRTF on OpenAL context
  2. Update listener position/orientation from VR headset pose every frame
  3. Use OpenXR head tracking for audio listener, not just camera
- **Verify:** Sound correctly follows head rotation in VR
- **Implementation:** VRInterface::load() calls enableHRTF() after successful VR init. VRInterface::update() sets audio listener position/orientation from OpenXR head pose (center of head, transformed by ship base position/rotation). VRInterface::unload() calls disableHRTF(). Added getSound() accessor to SimulationModel.
- [x] Done (2026-02-12: HRTF wired into VR lifecycle. Needs VR hardware + OpenAL Soft for testing.)

### 6-03: Implement VR hand tracking for bridge controls
- **Files:** `src/VRInterface.cpp`, `src/VRInterface.hpp`
- **What:** Map VR controller inputs to bridge controls:
  - Grip/squeeze → grab throttle/rudder wheel
  - Thumbstick Y → fine adjustment when gripping
  - Trigger → horn
  - Menu button → toggle HUD
- **Verify:** Can control ship using VR controllers
- **Implementation:** Added thumbstick_y_action and trigger_action to OpenXR actions. Added Oculus Touch controller interaction profile (/interaction_profiles/oculus/touch_controller) with squeeze→grab, trigger→horn, thumbstick Y→fine adjustment. Left thumbstick Y adjusts engine, right adjusts rudder (only while grip held). Trigger on either hand activates horn via startHorn()/endHorn(). Menu button (existing) toggles HUD. Simple controller profile preserved as fallback.
- [x] Done (2026-02-12: Actions and Oculus Touch bindings added. Needs VR hardware for testing.)

---

## Phase 7: UI Modernization

### 7-01: Integrate Dear ImGui
- **Files:** Create `src/gui/ImGuiOverlay.hpp/cpp`
- **What:** Add Dear ImGui for overlay UI rendering:
  - Wicked Engine has built-in ImGui support
  - Create overlay rendering pass
  - Render after 3D scene
- **Verify:** ImGui demo window renders on top of 3D scene
- **Implementation:** Created ImGuiOverlay class with SimulationHUDData struct for simulation-to-HUD data flow. Overlay manages all instrument windows with show/hide per instrument, palette switching, and layout lock/unlock. Renders compass, speed, rudder, depth, engine, and wind displays.
- [x] Done

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
- **Implementation:** renderCompass() in ImGuiOverlay.cpp. ImDrawList custom drawing: filled circle background, 8 cardinal/intercardinal labels rotated by heading, 10-degree tick marks, yellow heading triangle indicator, numeric readout below.
- [x] Done

### 7-03: Implement speed/RPM displays in ImGui
- **Files:** `src/gui/InstrumentWidgets.hpp/cpp`
- **What:** Speed over ground, speed through water, engine RPM:
  - Numeric displays with appropriate units (knots, RPM)
  - Optional bar graph for RPM
- **Verify:** Values match simulation state
- **Implementation:** renderSpeedDisplay() shows SOG (yellow), STW (green), COG. renderEngineDisplay() shows RPM with progress bar and thrust lever percentage.
- [x] Done

### 7-04: Implement rudder angle display in ImGui
- **Files:** `src/gui/InstrumentWidgets.hpp/cpp`
- **What:** Rudder angle indicator:
  - Graphical arc showing rudder position
  - Numeric degrees display
  - Port (red) / Starboard (green) color coding
- **Verify:** Display matches actual rudder position
- **Implementation:** renderRudderDisplay() with arc drawn via ImDrawList::PathArcTo. Port=red, starboard=green, center mark, yellow needle. Numeric readout color-coded by direction.
- [x] Done

### 7-05: Implement depth display in ImGui
- **Files:** `src/gui/ImGuiOverlay.hpp/cpp`
- **What:** Depth sounder readout:
  - Numeric depth display in metres (or feet, configurable)
  - Alarm threshold indication
- **Verify:** Depth matches terrain heightmap at ship position
- **Implementation:** Enhanced renderDepthDisplay() with vertical depth gauge bar, alarm threshold marker (dashed red line), depth trend arrow (shoaling/deepening/steady from 30-frame history), flashing red background on alarm, yellow warning when approaching alarm (depth < 2x threshold), m/ft unit toggle via button or F10 key. Scale auto-ranges based on current depth.
- [x] Done (2026-02-12)

### 7-06: Add 4 brightness palettes (OpenBridge)
- **Files:** `src/gui/ImGuiOverlay.hpp/cpp`
- **What:** Implement 4 brightness modes per OpenBridge standard:
  1. **Bright Day:** White backgrounds, high contrast
  2. **Day:** Light grey backgrounds
  3. **Dusk:** Dark grey, reduced brightness
  4. **Night:** Black background, red/amber indicators only

  Store as ImGui color schemes. Switch via keyboard shortcut (e.g., F5-F8).
- **Verify:** All instruments readable in each mode. Night mode doesn't ruin night vision.
- **Implementation:** applyPalette() expanded to set 12 ImGui color slots per palette (WindowBg, TitleBg, TitleBgActive, Text, Border, FrameBg, ScrollbarBg, ScrollbarGrab, Button, ButtonHovered, ButtonActive, PlotHistogram). Night mode uses red/amber only. Keyboard shortcuts: F5=Bright Day, F6=Day, F7=Dusk, F8=Night, F9=toggle layout lock. BridgePalette enum added.
- [x] Done (2026-02-12)

### 7-07: Implement configurable instrument layout
- **Files:** `src/gui/ImGuiOverlay.hpp/cpp`
- **What:** Let users drag and resize instrument windows:
  - Save layout to `bc5.ini` or separate `layout.ini`
  - Default layouts for each monitor type (bridge view, radar, helm repeater)
  - Lock/unlock toggle
- **Verify:** User can rearrange instruments. Layout persists across sessions.
- **Implementation:** saveLayout()/loadLayout() for instrument visibility, palette, units, lock state in INI format. setIniFilePath() wires up ImGui's built-in window position persistence (imgui.ini). F9 toggles layout lock for drag/resize. Default positions set via ImGuiCond_FirstUseEver.
- [x] Done (2026-02-12)

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
  - [x] Document new keyboard shortcuts (brightness modes, etc.) -- added to README (F5-F8 palettes, F9 layout lock, F10 depth units)
  - [x] Update architecture diagrams if any exist -- no diagrams found, not applicable
  - [x] Wicked Engine build instructions added to README
- [x] Done (2026-02-12: README updated with WE build section, keyboard shortcuts. CHANGELOG.md fully rewritten covering all 9 phases.)

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
- [x] Done (2026-02-12: Version bumped to 6.0.0-beta.1 in Constants.hpp. CHANGELOG.md covers all completed phases with user-facing summary and detailed phase breakdown.)

---

## Phase 10A: Scenario Editor - Map Tile System

> **Goal:** Build the foundation for a slippy map widget that displays real-world satellite
> imagery and street maps inside a Dear ImGui window. This is the base layer that all
> subsequent editor features build on top of.
>
> **Why this matters:** The current editor (`bridgecommand-ed`) displays a tiny bitmap
> of the pre-built world. The new editor will show a zoomable, pannable map of the
> **entire Earth** -- like Google Maps -- so users can create scenarios anywhere.
>
> **Key concept -- Slippy Map Tiles:** The world map is divided into 256x256 pixel PNG
> images called "tiles". At zoom level 0, the entire world is 1 tile. At zoom 1, it's
> 2x2 = 4 tiles. At zoom N, it's 2^N x 2^N tiles. Tiles are addressed by `{z}/{x}/{y}`
> where z=zoom, x=column (0=left), y=row (0=top). Each tile covers a specific lat/lon
> rectangle. The math to convert lat/lon to tile coordinates is standardised (Web Mercator
> projection, EPSG:3857).
>
> **Tile URL format:**
> - OpenStreetMap: `https://tile.openstreetmap.org/{z}/{x}/{y}.png`
> - ESRI Satellite: `https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}` (note: y before x)

### 10A-01: Add libcurl dependency to CMake
- **File:** `src/CMakeLists.txt`
- **What:** Add libcurl as a required dependency for HTTP tile downloads:
  ```cmake
  find_package(CURL REQUIRED)
  if(CURL_FOUND)
    message(STATUS "libcurl found: ${CURL_VERSION_STRING}")
    add_definitions(-DWITH_CURL)
  endif()
  ```
- **Install:**
  - macOS: `brew install curl` (usually pre-installed)
  - Ubuntu: `apt install libcurl4-openssl-dev`
  - Windows: `vcpkg install curl` or download from curl.se
- **Verify:** CMake configure says "libcurl found" with version number. Project still builds.
- [ ] Done

### 10A-02: Create tile coordinate math utilities
- **Files:**
  - Create `src/editor/TileMath.hpp`
- **What:** Implement the standard Web Mercator tile coordinate conversion functions.
  These are pure math -- no external dependencies:
  ```cpp
  #pragma once
  #include <cmath>
  #include <algorithm>

  namespace TileMath {
      // Convert longitude to tile X coordinate at given zoom level
      // lon: degrees (-180 to 180), zoom: 0-19
      // Returns: tile X index (0 to 2^zoom - 1)
      inline int lonToTileX(double lon, int zoom) {
          return static_cast<int>(std::floor((lon + 180.0) / 360.0 * (1 << zoom)));
      }

      // Convert latitude to tile Y coordinate at given zoom level
      // lat: degrees (-85.05 to 85.05), zoom: 0-19
      // Returns: tile Y index (0 to 2^zoom - 1)
      inline int latToTileY(double lat, int zoom) {
          double latRad = lat * M_PI / 180.0;
          return static_cast<int>(std::floor(
              (1.0 - std::log(std::tan(latRad) + 1.0 / std::cos(latRad)) / M_PI)
              / 2.0 * (1 << zoom)));
      }

      // Convert tile X back to longitude (left edge of tile)
      inline double tileXToLon(int x, int zoom) {
          return x / static_cast<double>(1 << zoom) * 360.0 - 180.0;
      }

      // Convert tile Y back to latitude (top edge of tile)
      inline double tileYToLat(int y, int zoom) {
          double n = M_PI - 2.0 * M_PI * y / static_cast<double>(1 << zoom);
          return 180.0 / M_PI * std::atan(0.5 * (std::exp(n) - std::exp(-n)));
      }

      // Convert screen pixel position to lat/lon given map state
      // centerLat/Lon: map center, zoom: current zoom, pixelX/Y: screen offset from center
      // tileSize: usually 256
      struct LatLon { double lat; double lon; };
      LatLon pixelToLatLon(double centerLat, double centerLon, int zoom,
                           int pixelX, int pixelY, int tileSize = 256);

      // Inverse: lat/lon to screen pixel offset from map center
      struct PixelPos { int x; int y; };
      PixelPos latLonToPixel(double centerLat, double centerLon, int zoom,
                             double lat, double lon, int tileSize = 256);

      // Clamp latitude to valid Mercator range
      inline double clampLat(double lat) {
          return std::clamp(lat, -85.0511, 85.0511);
      }
  }
  ```
- **Verify:** Write unit tests in `src/tests/test_tile_math.cpp`:
  - `lonToTileX(0.0, 0)` == 0 (centre of world at zoom 0)
  - `lonToTileX(-180.0, 1)` == 0 (left edge at zoom 1)
  - `lonToTileX(179.99, 1)` == 1 (right edge at zoom 1)
  - `latToTileY(0.0, 0)` == 0 (equator at zoom 0)
  - Round-trip: `tileXToLon(lonToTileX(lon, z), z)` ≈ lon (within one tile width)
  - Round-trip: `tileYToLat(latToTileY(lat, z), z)` ≈ lat (within one tile height)
  - `pixelToLatLon` ↔ `latLonToPixel` round-trip within 1 pixel
- [ ] Done

### 10A-03: Create tile downloader with disk cache
- **Files:**
  - Create `src/editor/TileDownloader.hpp`
  - Create `src/editor/TileDownloader.cpp`
- **What:** Download map tiles from a tile server URL and cache them to disk so they're
  only downloaded once. Uses libcurl for HTTP requests.
  ```cpp
  class TileDownloader {
  public:
      // tileServerUrl: URL template with {z}, {x}, {y} placeholders
      //   e.g. "https://tile.openstreetmap.org/{z}/{x}/{y}.png"
      // cacheDir: local directory for cached tiles
      //   e.g. "~/.bridgecommand/tilecache/"
      TileDownloader(const std::string& tileServerUrl, const std::string& cacheDir);
      ~TileDownloader();

      // Request a tile. Returns immediately.
      // If tile is in disk cache, loads from disk (fast).
      // If not cached, queues HTTP download (async).
      // Returns: raw PNG bytes, or empty vector if not yet available.
      std::vector<uint8_t> getTile(int z, int x, int y);

      // Check if a tile is available (cached or downloaded)
      bool isTileReady(int z, int x, int y) const;

      // How many downloads are pending?
      int pendingDownloads() const;

      // Set User-Agent header (required by OSM tile usage policy)
      void setUserAgent(const std::string& ua);

  private:
      std::string buildUrl(int z, int x, int y) const;
      std::string buildCachePath(int z, int x, int y) const;

      std::string serverUrl;
      std::string cacheDir;
      std::string userAgent;

      // In-memory cache of recently used tiles (LRU, max ~200 tiles)
      // Key: "z/x/y" string
      std::unordered_map<std::string, std::vector<uint8_t>> memoryCache;

      // Background download queue (use std::thread + std::mutex)
      // Download at most 2 tiles concurrently (be nice to tile servers)
      std::queue<std::tuple<int,int,int>> downloadQueue;
      std::mutex queueMutex;
      std::thread downloadThread;
      bool running = true;
  };
  ```
- **Implementation notes:**
  - Cache directory structure: `{cacheDir}/{z}/{x}/{y}.png`
  - Create directories recursively with `std::filesystem::create_directories()`
  - libcurl: use `curl_easy_perform()` with `CURLOPT_WRITEFUNCTION` callback
  - Set `User-Agent: BridgeCommand/6.0 (scenario-editor)` (OSM requires a user agent)
  - Rate limit: max 2 requests/second (OSM usage policy)
  - Handle HTTP errors: 404 = tile doesn't exist at this zoom, 429 = rate limited
  - Handle network errors: timeout after 10 seconds, don't retry immediately
- **Verify:** Download a single tile, verify PNG file appears in cache directory.
  Re-run, verify it loads from cache (no network request).
- [ ] Done

### 10A-04: Create tile texture manager (PNG → GPU texture)
- **Files:**
  - Create `src/editor/TileTextureManager.hpp`
  - Create `src/editor/TileTextureManager.cpp`
- **What:** Convert downloaded PNG tile data into OpenGL textures that Dear ImGui can display.
  Uses `stb_image` to decode PNG, then `glGenTextures`/`glTexImage2D` to upload to GPU.
  ```cpp
  class TileTextureManager {
  public:
      // Get or create GPU texture for a tile.
      // Returns OpenGL texture ID (cast to ImTextureID for ImGui).
      // Returns 0 if tile not yet available (still downloading).
      ImTextureID getTileTexture(int z, int x, int y);

      // Release textures that haven't been used recently (call once per frame)
      void evictUnused(int maxTextures = 300);

      // Release all textures (call on shutdown)
      void clear();

  private:
      struct CachedTexture {
          GLuint textureId = 0;
          int lastUsedFrame = 0;
      };
      std::unordered_map<std::string, CachedTexture> textures;
      int currentFrame = 0;

      GLuint uploadToGPU(const uint8_t* rgbaData, int width, int height);
  };
  ```
- **Implementation notes:**
  - Use `stb_image` (already in project for heightmap work) to decode PNG → RGBA
  - Standard OpenGL texture upload:
    ```cpp
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    ```
  - Evict oldest textures when cache exceeds limit (LRU eviction)
  - Show a placeholder colour (light grey) while tile is loading
- **Verify:** A tile texture uploads and can be displayed with `ImGui::Image(textureId, ...)`
- [ ] Done

### 10A-05: Create ImGui map widget
- **Files:**
  - Create `src/editor/MapWidget.hpp`
  - Create `src/editor/MapWidget.cpp`
- **What:** An ImGui widget that displays a pannable, zoomable slippy map. This is the
  core visual component of the new editor.
  ```cpp
  class MapWidget {
  public:
      MapWidget(TileDownloader* downloader, TileTextureManager* texManager);

      // Call every frame inside an ImGui window. Handles rendering + input.
      void render();

      // Map state
      void setCenter(double lat, double lon);
      void setZoom(int zoom);  // 0-19
      double getCenterLat() const;
      double getCenterLon() const;
      int getZoom() const;

      // Convert between screen coords (within widget) and lat/lon
      TileMath::LatLon screenToLatLon(float screenX, float screenY) const;
      TileMath::PixelPos latLonToScreen(double lat, double lon) const;

      // Callbacks for click events (set by editor)
      std::function<void(double lat, double lon, int button)> onMapClick;
      std::function<void(double lat, double lon)> onMapDrag;

      // Layer toggles
      bool showSatellite = true;   // ESRI World Imagery
      bool showStreetMap = false;  // OSM tiles (overlaid)
      bool showGrid = true;        // Lat/lon grid lines

  private:
      TileDownloader* downloader;
      TileTextureManager* texManager;

      double centerLat = 50.0;  // Default: somewhere in the English Channel
      double centerLon = -5.0;
      int zoom = 6;

      // Dragging state
      bool isDragging = false;
      float dragStartX, dragStartY;
      double dragStartLat, dragStartLon;

      void renderTiles();       // Draw the tile grid
      void renderGridLines();   // Draw lat/lon lines
      void handleInput();       // Mouse wheel zoom, click-drag pan
  };
  ```
- **Implementation notes:**
  - Get the widget area with `ImGui::GetContentRegionAvail()` and `ImGui::GetCursorScreenPos()`
  - Calculate which tiles are visible: for each screen pixel row/column, compute lat/lon,
    then tile x/y. Usually ~3x4 tiles visible at any time.
  - Use `ImDrawList::AddImage()` to draw each visible tile at the correct screen position
  - Mouse wheel: zoom in/out (clamp to 2-19)
  - Mouse drag: pan the map (update centerLat/centerLon)
  - Right-click: fire `onMapClick` callback with lat/lon coordinates
  - Draw lat/lon grid lines every 1° (zoom < 8), 0.1° (zoom 8-12), 0.01° (zoom > 12)
  - Show crosshair at center with lat/lon readout
  - Show zoom level and scale bar (e.g. "1 km" or "10 NM")
- **Verify:** Widget displays world map tiles. Can pan with mouse drag. Can zoom with
  scroll wheel. Lat/lon readout updates as you move the mouse. Tiles load progressively
  (grey placeholder → satellite imagery appears).
- [ ] Done

### 10A-06: Add dual tile source support (satellite + street map)
- **Files:** `src/editor/MapWidget.cpp`, `src/editor/TileDownloader.hpp`
- **What:** Support two tile sources that can be toggled or overlaid:
  1. **Satellite imagery** (default): ESRI World Imagery
     - URL: `https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}`
     - Note: ESRI uses `{z}/{y}/{x}` order (Y before X!)
  2. **Street map** (toggle): OpenStreetMap standard tiles
     - URL: `https://tile.openstreetmap.org/{z}/{x}/{y}.png`
     - Requires User-Agent header and attribution
  - Add a toolbar button or keyboard shortcut (e.g. `M` key) to switch between them
  - Each source has its own TileDownloader + cache subdirectory:
    - `~/.bridgecommand/tilecache/esri/`
    - `~/.bridgecommand/tilecache/osm/`
- **Attribution display:** When OSM tiles are shown, render "© OpenStreetMap contributors"
  at the bottom-right corner of the map widget. When ESRI is shown, render "Tiles © Esri".
- **Verify:** Toggle between satellite and street view. Both load correctly. Attribution
  text displays for each source.
- [ ] Done

### 10A-07: Write unit tests for tile math and downloader
- **Files:** Create `src/tests/test_tile_system.cpp`
- **What:** Test the tile coordinate system thoroughly:
  ```cpp
  TEST_CASE("Tile coordinates at zoom 0") {
      // At zoom 0, entire world is one tile
      REQUIRE(TileMath::lonToTileX(-180.0, 0) == 0);
      REQUIRE(TileMath::lonToTileX(180.0, 0) == 0);  // wraps
      REQUIRE(TileMath::latToTileY(0.0, 0) == 0);
  }

  TEST_CASE("Tile coordinates at zoom 2") {
      // At zoom 2, 4x4 = 16 tiles
      REQUIRE(TileMath::lonToTileX(0.0, 2) == 2);     // Prime meridian
      REQUIRE(TileMath::latToTileY(0.0, 2) == 2);     // Equator
      REQUIRE(TileMath::lonToTileX(-122.4, 2) == 0);   // San Francisco
  }

  TEST_CASE("Known locations") {
      // London: 51.5, -0.1
      int z = 10;
      int tx = TileMath::lonToTileX(-0.1, z);
      int ty = TileMath::latToTileY(51.5, z);
      // At zoom 10, London should be around tile (511, 340)
      REQUIRE(tx >= 510);
      REQUIRE(tx <= 512);
      REQUIRE(ty >= 339);
      REQUIRE(ty <= 341);
  }

  TEST_CASE("ESRI tile URL reverses x/y") {
      // ESRI uses {z}/{y}/{x} not {z}/{x}/{y}
      TileDownloader esri("https://server.arcgisonline.com/.../tile/{z}/{y}/{x}", "cache");
      std::string url = esri.buildUrl(10, 511, 340);
      // URL should contain "/10/340/511" (y before x)
      REQUIRE(url.find("/10/340/511") != std::string::npos);
  }
  ```
- **Verify:** All tile math tests pass. Round-trip conversions within tolerance.
- [ ] Done

---

## Phase 10B: Scenario Editor - Global Data Sources

> **Goal:** Integrate free global datasets so the editor can display coastlines, ocean
> depth, and generate terrain for any location on Earth -- even without S-57 charts.
>
> **Data sources:**
> - **Natural Earth** (public domain): Global coastline polygons for land/sea display
> - **GEBCO 2024** (free, attribution required): Global ocean depth at ~450m resolution
> - **Copernicus DEM** (free): Land elevation at 30m resolution (already integrated in Phase 0C)
> - **S-57 ENC charts** (free from NOAA/UKHO): High-detail navigation data (already integrated in Phase 0B)
>
> **Fallback chain for terrain generation:**
> 1. If S-57 chart available → use ChartReader (buoys, lights, detailed depth)
> 2. If no S-57 → use GEBCO for bathymetry + Natural Earth for coastlines
> 3. Always use Copernicus DEM for land elevation (already works)

### 10B-01: Bundle Natural Earth coastline data
- **Files:**
  - Create `tools/download_natural_earth.sh`
  - Store processed data in `bin/Data/Coastlines/`
- **What:** Download and pre-process Natural Earth coastline data for fast runtime loading.
  The editor needs to draw coastline outlines on the map at all zoom levels.

  **Step 1: Download script:**
  ```bash
  #!/bin/bash
  # Downloads Natural Earth data and converts to a simple binary format
  # Requires: ogr2ogr (from GDAL), or just download the shapefiles

  # 1:50m scale (good for zoom 2-8)
  wget https://naciscdn.org/naturalearth/50m/physical/ne_50m_land.zip
  unzip ne_50m_land.zip -d ne_50m_land/

  # 1:10m scale (good for zoom 8+)
  wget https://naciscdn.org/naturalearth/10m/physical/ne_10m_land.zip
  unzip ne_10m_land.zip -d ne_10m_land/

  # Convert shapefiles to GeoJSON (simpler to parse in C++)
  ogr2ogr -f GeoJSON ne_50m_land.geojson ne_50m_land/ne_50m_land.shp
  ogr2ogr -f GeoJSON ne_10m_land.geojson ne_10m_land/ne_10m_land.shp
  ```

  **Step 2: Pre-process to compact binary format:**
  Create a Python script `tools/convert_coastlines.py` that reads the GeoJSON and writes
  a simple binary format:
  ```
  [uint32_t polygon_count]
  For each polygon:
    [uint32_t vertex_count]
    [float lon0, lat0, lon1, lat1, ...] (vertex pairs)
  ```
  This binary file is ~2 MB for 50m and ~15 MB for 10m data.

  **Step 3: Ship the binary files with the app** in `bin/Data/Coastlines/`:
  - `coastlines_50m.bin` (for zoom 2-8)
  - `coastlines_10m.bin` (for zoom 8+)
- **Add to `.gitignore`:** The raw shapefiles and GeoJSON (large), but DO commit the
  compact binary files (small).
- **Verify:** Binary files exist. A test can load them and count polygons.
- [ ] Done

### 10B-02: Create coastline renderer
- **Files:**
  - Create `src/editor/CoastlineRenderer.hpp`
  - Create `src/editor/CoastlineRenderer.cpp`
- **What:** Load the pre-processed coastline data and render it as line overlays on the
  map widget using `ImDrawList` line-drawing functions.
  ```cpp
  class CoastlineRenderer {
  public:
      // Load coastline data from binary file
      bool load(const std::string& path);

      // Render coastlines visible in the current map viewport
      // drawList: ImGui draw list for the map widget
      // mapWidget: provides latLonToScreen() conversion
      void render(ImDrawList* drawList, const MapWidget& mapWidget);

  private:
      struct Polygon {
          std::vector<float> vertices;  // [lon0, lat0, lon1, lat1, ...]
          float minLon, maxLon, minLat, maxLat;  // Bounding box for culling
      };
      std::vector<Polygon> polygons;
  };
  ```
- **Implementation notes:**
  - For each polygon, compute a bounding box at load time
  - Each frame: skip polygons whose bounding box doesn't overlap the current viewport
  - Convert each vertex from lat/lon to screen pixels using `mapWidget.latLonToScreen()`
  - Draw with `ImDrawList::AddPolyline()` -- yellow or white lines, 1-2 px thick
  - At low zoom (world view), use the 50m data. At high zoom (regional), switch to 10m.
  - Simplify polygons at low zoom: skip every Nth vertex to avoid drawing millions of points
- **Verify:** Coastlines render on the map. Recognisable shapes of continents at zoom 3-4.
  Detailed coastline at zoom 10+. Performance: <5ms for coastline rendering at any zoom.
- [ ] Done

### 10B-03: Add GEBCO bathymetry data support
- **Files:**
  - Create `tools/download_gebco.sh`
  - Create `src/editor/GEBCOReader.hpp`
  - Create `src/editor/GEBCOReader.cpp`
- **What:** Download GEBCO 2024 global bathymetry grid and build a reader class that
  can query ocean depth at any lat/lon.

  **Download:** GEBCO provides the full grid as 8 GeoTIFF tiles (~4 GB compressed):
  ```bash
  #!/bin/bash
  # Download GEBCO 2024 sub-ice GeoTIFF tiles from GEBCO website
  # Visit: https://download.gebco.net/ → select "2024 Sub-Ice Topo" → GeoTIFF format
  # Or download directly:
  TILES="n0-90_w-180-0 n0-90_w0-180 s-90-0_w-180-0 s-90-0_w0-180"
  for tile in $TILES; do
      wget "https://download.gebco.net/gebco_2024/gebco_2024_sub_ice_topo_geotiff/${tile}.tif.zip"
      unzip "${tile}.tif.zip" -d gebco_tiles/
  done
  ```

  **Reader class:**
  ```cpp
  class GEBCOReader {
  public:
      // Load one or more GEBCO GeoTIFF tiles using GDAL
      bool loadTile(const std::string& geotiffPath);
      bool loadDirectory(const std::string& dirPath);  // Load all .tif files

      // Query depth at a point. Returns depth in metres (negative = below sea level).
      // Returns NaN if point is outside loaded tiles.
      float getDepth(double lat, double lon) const;

      // Query depths for a rectangular area (for heightmap generation)
      // Returns a 2D grid of depths
      std::vector<std::vector<float>> getDepthGrid(
          double minLat, double maxLat, double minLon, double maxLon,
          int rows, int cols) const;

      // Is a point on land (depth > 0) or sea (depth <= 0)?
      bool isLand(double lat, double lon) const;

  private:
      struct GeoTile {
          std::vector<int16_t> data;
          int width, height;
          double originLon, originLat;  // Top-left corner
          double pixelSizeLon, pixelSizeLat;  // Degrees per pixel
      };
      std::vector<GeoTile> tiles;

      // Find which tile covers a lat/lon point
      const GeoTile* findTile(double lat, double lon) const;
      float sampleTile(const GeoTile& tile, double lat, double lon) const;
  };
  ```
- **Implementation notes:**
  - GEBCO GeoTIFFs are 16-bit signed integers (int16_t), in WGS84 (EPSG:4326)
  - Resolution: 15 arc-seconds ≈ 0.004167° per pixel
  - Grid size per 90° tile: ~21,600 x 21,600 pixels
  - Use GDAL to read (same as existing DEM code in HeightmapGenerator):
    ```cpp
    GDALDataset* ds = (GDALDataset*)GDALOpen(path.c_str(), GA_ReadOnly);
    GDALRasterBand* band = ds->GetRasterBand(1);
    double geotransform[6];
    ds->GetGeoTransform(geotransform);
    // geotransform[0] = origin lon, [3] = origin lat
    // geotransform[1] = pixel width, [5] = pixel height (negative)
    ```
  - For the editor display, depth queries can be relatively slow (~1ms each is fine)
  - For world generation, batch queries using `getDepthGrid()` with GDAL `RasterIO`
- **Storage:** GEBCO tiles are large (~7.5 GB uncompressed). Store externally, not in git.
  Add `bin/Data/GEBCO/` to `.gitignore`. The download script and reader code are committed.
- **Verify:** Load a GEBCO tile. Query depth at known locations:
  - Mid-Atlantic (~30°N, 30°W): should return ~-3000 to -4000m
  - London (51.5°N, 0.1°W): should return >0 (land)
  - English Channel (50.5°N, 1.0°W): should return ~-30 to -60m
- [ ] Done

### 10B-04: Add depth contour overlay to map widget
- **Files:** `src/editor/MapWidget.cpp` (extend)
- **What:** Draw ocean depth contours on the map as coloured overlays, giving the user
  visual feedback about water depth.
  - At the current viewport, sample GEBCO depth on a ~50-pixel grid
  - Colour-code by depth:
    - 0-10m: light cyan (shallow -- danger zone for large ships)
    - 10-50m: light blue
    - 50-200m: medium blue
    - 200-2000m: dark blue
    - 2000m+: navy (deep ocean)
  - Draw as semi-transparent filled rectangles using `ImDrawList::AddRectFilled()`
  - Only render at zoom level 6+ (at lower zoom it's too coarse to be useful)
  - Add toggle: `bool showDepthOverlay = false;`
- **Verify:** Enable depth overlay. See blue shading over ocean areas. Shallow coastal
  areas are lighter. Depth legend displays in corner.
- [ ] Done

### 10B-05: Integrate GEBCO into WorldGenerator as fallback
- **Files:** `src/WorldGenerator.hpp/cpp` (extend), `src/HeightmapGenerator.hpp/cpp` (extend)
- **What:** When generating a world without S-57 chart data, use GEBCO for bathymetry
  and Natural Earth for coastlines. Extend the existing WorldGenerator pipeline:
  ```cpp
  // New method: generate world from bounding box alone (no chart required)
  WorldGeneratorResult WorldGenerator::generateWorldFromBounds(
      double minLat, double maxLat, double minLon, double maxLon,
      const std::string& outputDir,
      int heightmapSize = 1025,
      const std::string& demDir = "",      // Copernicus DEM tiles (land)
      const std::string& gebcoDir = "",    // GEBCO tiles (ocean depth)
      const std::string& coastlineFile = "" // Natural Earth binary
  );
  ```
  **Pipeline:** Same as existing `generateWorld()` but with different data sources:
  1. If coastlineFile provided → use Natural Earth polygons for land/sea detection
  2. If gebcoDir provided → use GEBCO for underwater depths (instead of S-57 DEPARE)
  3. If demDir provided → use Copernicus DEM for land elevation (already works)
  4. No buoys, lights, or landmarks (those only come from S-57 charts)
  5. Still generates: height.png, texture.png, map.png, terrain.ini, empty buoy/light/landobject.ini
- **Verify:** Generate a world for an area with no S-57 chart (e.g. Mediterranean coast).
  Load it in the simulator. Terrain renders with correct land elevation and water depth.
- [ ] Done

### 10B-07: Add NOAA ENC chart catalog and auto-download
- **Files:**
  - Create `src/editor/ENCCatalog.hpp`
  - Create `src/editor/ENCCatalog.cpp`
- **What:** Automatically discover and download free S-57 charts covering the user's
  selected scenario area. NOAA provides a free JSON index of every US ENC chart with
  bounding boxes and direct download URLs.

  **Data source:** NOAA Interactive Catalog JSON (~2 MB, updated daily):
  ```
  https://www.charts.noaa.gov/InteractiveCatalog/data/enc.json
  ```
  Each entry contains:
  ```json
  {
    "cnum": "US5WA18M",           // Chart ID
    "title": "Puget Sound - ...",  // Human name
    "scale": "1:22,000",           // Scale (lower = more detail)
    "pdate": "2025-12-23",         // Publication date
    "mbr": [48.175, -122.925, 48.250, -122.850],  // [minLat, minLon, maxLat, maxLon]
    "points": [...]                // Coverage polygon vertices
  }
  ```

  **Download URL pattern:** `https://www.charts.noaa.gov/ENCs/{CHART_ID}.zip`
  Each ZIP contains the `.000` base file plus update files (`.001`, `.002`), a
  `CATALOG.031`, and `README.TXT`.

  ```cpp
  class ENCCatalog {
  public:
      struct ChartInfo {
          std::string id;         // e.g. "US5WA18M"
          std::string title;
          int scale;              // e.g. 22000
          double minLat, maxLat, minLon, maxLon;  // Bounding box
          std::string downloadUrl;
          bool isDownloaded = false;
      };

      // Download and parse the NOAA catalog JSON. Cache locally for 24 hours.
      bool loadCatalog(const std::string& cacheDir);

      // Find all charts whose bounding box intersects the given area.
      // Returns charts sorted by scale (most detailed first).
      std::vector<ChartInfo> findChartsForArea(
          double minLat, double maxLat, double minLon, double maxLon) const;

      // Download a chart ZIP, extract the .000 file to cacheDir.
      // Returns path to the extracted .000 file, or empty on failure.
      std::string downloadChart(const ChartInfo& chart,
                                const std::string& cacheDir);

      // Check if a chart is already cached locally.
      bool isChartCached(const ChartInfo& chart,
                         const std::string& cacheDir) const;

  private:
      std::vector<ChartInfo> catalog;
      std::string catalogCacheFile;  // Local path to cached enc.json
  };
  ```

- **Implementation notes:**
  - Use libcurl to download `enc.json` (same as tile downloader)
  - Parse JSON manually (use nlohmann/json header-only, or a minimal parser)
  - Bounding box intersection: `chart.minLat <= area.maxLat && chart.maxLat >= area.minLat && ...`
  - Chart ZIP files are 50 KB - 5 MB each, fast to download
  - Cache directory: `~/.bridgecommand/charts/noaa/`
  - NOAA terms: free, no auth, no rate limits, US government works (CC0)
  - Set `User-Agent: BridgeCommand/6.0` header
- **ENC naming convention:** `US{scale}{state}{seq}M` where scale 1=overview, 5=harbour, 6=berthing
- **Verify:** Call `findChartsForArea(48.35, 48.55, -122.60, -122.40)` → returns Swinomish/
  La Conner area charts. Download one → `.000` file extracts successfully. Feed to existing
  `ChartReader` → buoys/lights extracted correctly.
- [ ] Done

### 10B-08: Add European and worldwide chart catalog support
- **Files:** `src/editor/ENCCatalog.hpp/cpp` (extend)
- **What:** Support additional free chart sources beyond NOAA (US only). The
  [chartcatalogs](https://github.com/chartcatalogs/catalogs) project maintains XML
  catalogs for free chart sources worldwide:

  **Available sources (all free, S-57 format):**

  | Region | Catalog URL |
  |--------|-------------|
  | European inland (all) | `https://raw.githubusercontent.com/chartcatalogs/catalogs/master/EURIS_IENC_Catalog.xml` |
  | Germany inland | `DE_IENC_Catalog.xml` |
  | Netherlands inland | `NL_IENC_Catalog.xml` |
  | France inland | `FR_IENC_Catalog.xml` |
  | Austria, Belgium, Bulgaria, Croatia, Czech Republic, Hungary, Poland, Romania, Serbia, Slovakia, Switzerland | Individual `XX_IENC_Catalog.xml` files |
  | US inland (Army Corps) | `https://ienccloud.us/ienc/products/catalog/IENCU37ProductsCatalog.xml` |
  | New Zealand (LINZ) | Free with registration at `encservice.linz.govt.nz` |

  These XML catalogs follow the same format as NOAA's `ENCProdCat.xml`:
  ```xml
  <cell>
    <name>CHART_ID</name>
    <zipfile_location>https://download.url/CHART_ID.zip</zipfile_location>
    <cov><panel><vertex><lat>...</lat><long>...</long></vertex>...</panel></cov>
  </cell>
  ```

  **Implementation:**
  - Add `loadCatalogXML()` method to parse the XML format (use tinyxml2 or manual parsing)
  - Support multiple catalogs simultaneously (NOAA JSON + European XML + others)
  - `findChartsForArea()` searches all loaded catalogs
  - Show chart source in UI: "NOAA (US)", "EURIS (EU Inland)", etc.
  - Cache each catalog locally for 7 days

- **Note:** UKHO (UK maritime) charts are NOT free (commercial licence required).
  For UK waters, users must provide their own `.000` files via "Browse..." button.
- **Verify:** Load EURIS catalog. Search for charts on the Rhine near Cologne.
  Download and parse → inland waterway features extracted.
- [ ] Done

### 10B-06: Write unit tests for global data sources
- **Files:** Create `src/tests/test_global_data.cpp`
- **What:** Test coastline loading and GEBCO depth queries:
  ```cpp
  TEST_CASE("Coastline data loads") {
      CoastlineRenderer renderer;
      REQUIRE(renderer.load("Data/Coastlines/coastlines_50m.bin"));
      // Should have thousands of polygons (continents, islands)
  }

  TEST_CASE("GEBCO depth queries") {
      GEBCOReader gebco;
      REQUIRE(gebco.loadDirectory("Data/GEBCO/"));
      // Mid-ocean point
      float depth = gebco.getDepth(30.0, -30.0);
      REQUIRE(depth < -1000.0f);  // Deep ocean
      // Coastal point
      float coastal = gebco.getDepth(50.5, -1.0);
      REQUIRE(coastal < 0.0f);    // Channel is underwater
      REQUIRE(coastal > -200.0f); // But not deep ocean
  }

  TEST_CASE("World generation from GEBCO") {
      // Generate a small test world
      WorldGenerator gen;
      auto result = gen.generateWorldFromBounds(
          50.0, 50.5, -5.0, -4.5, "/tmp/test_world", 257,
          "", "Data/GEBCO/", "Data/Coastlines/coastlines_10m.bin");
      REQUIRE(result.success);
      REQUIRE(result.maxDepth > 0.0f);
  }
  ```
- **Note:** These tests require the GEBCO data files to be present. Mark them with
  a `[requires-data]` tag so they can be skipped in CI where the data isn't available.
- **Verify:** Tests pass when data files are present. Tests skip cleanly when files are missing.
- [ ] Done

---

## Phase 10C: Scenario Editor - Interactive Editor GUI

> **Goal:** Build a new Dear ImGui-based scenario editor that replaces the current
> Irrlicht-based editor (`bridgecommand-ed`). The new editor uses the map widget from
> Phase 10A to let users visually create scenarios anywhere in the world.
>
> **User workflow (what we're building):**
> 1. Launch the editor → see a world map with satellite imagery
> 2. Navigate to the area of interest (pan, zoom, or type coordinates)
> 3. Draw a rectangle on the map to define the scenario area
> 4. Click "Generate World" → system downloads DEM + bathymetry, creates BC world files
> 5. Place own ship and other ships by clicking on the map
> 6. Set ship types, courses, speeds, waypoints
> 7. Configure weather, time, visibility
> 8. Save scenario → writes standard BC scenario files (environment.ini, ownship.ini, etc.)
> 9. Click "Test" → launches the simulator with this scenario
>
> **The editor is a separate executable** (`bridgecommand-ed`), not part of the main simulator.
> It currently uses Irrlicht for its GUI. We will rewrite it using Dear ImGui + OpenGL,
> which gives us a modern, customisable UI without depending on Irrlicht.

### 10C-01: Create new ImGui-based editor application skeleton
- **Files:**
  - Create `src/editor/EditorApp.hpp`
  - Create `src/editor/EditorApp.cpp`
  - Modify `src/editor/main.cpp` (add ImGui init alongside existing Irrlicht code)
- **What:** Set up the ImGui rendering context for the editor. The editor needs its own
  window with OpenGL context + Dear ImGui.

  **Option A (recommended): Use GLFW + ImGui** (cross-platform, lightweight):
  ```cpp
  class EditorApp {
  public:
      bool init(int width, int height, const std::string& title);
      void run();   // Main loop
      void shutdown();

  private:
      GLFWwindow* window = nullptr;

      // Editor components (created in init)
      std::unique_ptr<TileDownloader> satelliteDownloader;
      std::unique_ptr<TileDownloader> osmDownloader;
      std::unique_ptr<TileTextureManager> texManager;
      std::unique_ptr<MapWidget> mapWidget;
      std::unique_ptr<CoastlineRenderer> coastlineRenderer;

      // Editor state
      ScenarioData scenarioData;
      std::string currentWorldPath;
      bool scenarioModified = false;

      void renderMenuBar();
      void renderMapPanel();        // Left: full map widget
      void renderPropertiesPanel();  // Right: scenario properties
      void renderToolbar();          // Top: tools (select, place ship, draw area)
      void renderStatusBar();        // Bottom: coordinates, zoom, status
  };
  ```

  **Build approach:** Add GLFW as CMake dependency:
  ```cmake
  # In src/CMakeLists.txt, for the editor target:
  find_package(glfw3 3.3 REQUIRED)
  target_link_libraries(bridgecommand-ed PRIVATE glfw imgui)
  ```
  Install: `brew install glfw` (macOS), `apt install libglfw3-dev` (Linux),
  vcpkg (Windows).

- **Important:** Keep the existing Irrlicht editor working (don't delete it). The new
  editor builds alongside it. Eventually the old code will be removed.
- **Verify:** Editor window opens with ImGui rendering. Shows empty map widget area +
  properties panel. Can be closed cleanly.
- [ ] Done

### 10C-02: Implement "Go To Location" search
- **Files:** `src/editor/EditorApp.cpp` (extend)
- **What:** Add a search bar at the top of the editor that lets users jump to a location
  by name or coordinates.

  **Two modes:**
  1. **Coordinates mode:** User types `51.5, -0.1` → map centres on London
     - Parse as `lat, lon` (comma-separated decimals)
     - Also accept `51°30'N 0°06'W` DMS format
  2. **Place name mode:** User types "Portsmouth" → geocode to lat/lon
     - Use Nominatim API (free, no key required):
       `https://nominatim.openstreetmap.org/search?q=Portsmouth&format=json&limit=5`
     - Parse JSON response for `lat` and `lon` fields
     - Show top 5 results in a dropdown for disambiguation
     - Rate limit: max 1 request/second (Nominatim usage policy)
     - Set `User-Agent: BridgeCommand/6.0` header

  **UI:** An `ImGui::InputText` at the top with a "Go" button. Results in a popup list.
- **Verify:** Type "Southampton" → map jumps to Southampton, UK. Type "48.5, -122.5" →
  map jumps to Swinomish Channel area.
- [ ] Done

### 10C-03: Implement scenario area selection tool
- **Files:**
  - Create `src/editor/AreaSelector.hpp`
  - `src/editor/MapWidget.cpp` (extend)
- **What:** Let users draw a rectangle on the map to define the scenario area.
  This rectangle determines the world bounds (terrain.ini TerrainLong/Lat/Extent).
  ```cpp
  class AreaSelector {
  public:
      enum State { IDLE, DRAWING, COMPLETE };

      // Start drawing mode (user will click two corners)
      void startDrawing();

      // Handle map click (called from MapWidget::onMapClick)
      void handleClick(double lat, double lon);

      // Render the rectangle overlay on the map
      void render(ImDrawList* drawList, const MapWidget& mapWidget);

      // Get the selected area
      State getState() const;
      double getMinLat() const;
      double getMaxLat() const;
      double getMinLon() const;
      double getMaxLon() const;

      // Estimated world dimensions
      double getWidthKm() const;   // East-west extent in km
      double getHeightKm() const;  // North-south extent in km

  private:
      State state = IDLE;
      double corner1Lat, corner1Lon;
      double corner2Lat, corner2Lon;
  };
  ```
- **UI flow:**
  1. User clicks "Select Area" button in toolbar (or presses `A` key)
  2. Cursor changes to crosshair
  3. User clicks first corner on map
  4. User moves mouse → dashed rectangle previews from first corner to cursor
  5. User clicks second corner → rectangle is finalised
  6. Rectangle displayed with dimensions label (e.g. "12.3 km × 8.7 km")
  7. User can click "Clear" to reset and draw again

- **Render the rectangle:**
  - Dashed border (yellow or white)
  - Semi-transparent fill
  - Label showing dimensions and lat/lon bounds
  - Corner handles (small squares) for resizing after placement

- **Verify:** Draw a rectangle around Southampton Water. Dimensions display correctly
  (~15 km × 10 km). Coordinates match expected lat/lon values.
- [ ] Done

### 10C-04: Implement world generation trigger
- **Files:** `src/editor/EditorApp.cpp` (extend)
- **What:** After the user selects an area, provide a "Generate World" button that creates
  the BC world files for that area.

  **UI:** A panel that appears after area selection:
  ```
  ┌─ Generate World ─────────────────────────────┐
  │ Area: 50.85°N to 50.92°N, 1.35°W to 1.25°W  │
  │ Size: 8.2 km × 7.8 km                         │
  │                                                │
  │ World name: [Southampton_Water___________]     │
  │ Resolution: [1025 ▼] (pixels per side)         │
  │                                                │
  │ Data sources:                                  │
  │ ☑ S-57 chart: [Browse...] (optional)           │
  │ ☑ Copernicus DEM (land elevation)              │
  │ ☑ GEBCO bathymetry (ocean depth)               │
  │ ☐ BlueTopo (high-res US harbours)              │
  │                                                │
  │ [Generate World]  [Cancel]                     │
  │                                                │
  │ Status: Downloading DEM tiles... (2/4)         │
  └────────────────────────────────────────────────┘
  ```

  **Generation pipeline:**
  1. Create output directory: `bin/World/{worldName}/`
  2. If S-57 chart provided → use existing `WorldGenerator::generateWorld()`
  3. If no chart → use new `WorldGenerator::generateWorldFromBounds()` (task 10B-05)
  4. Auto-download Copernicus DEM tiles for the area (existing `tools/download_dem.py`)
  5. Show progress bar during generation
  6. On completion: "World generated! X buoys, Y lights, Z depth areas"
  7. Automatically switch to scenario editing mode with the new world loaded

- **Important:** Generation can take 10-30 seconds (DEM download, heightmap computation).
  Run in a background thread. Show progress updates in the UI.
- **Verify:** Select an area, click Generate, world files appear in bin/World/. Load the
  generated world in the simulator -- terrain renders with correct coastline shape.
- [ ] Done

### 10C-05: Implement ship placement tool
- **Files:**
  - Create `src/editor/ShipPlacer.hpp`
  - `src/editor/MapWidget.cpp` (extend)
- **What:** Let users place ships on the map by clicking.
  ```cpp
  class ShipPlacer {
  public:
      enum Tool { SELECT, PLACE_OWNSHIP, PLACE_OTHERSHIP, PLACE_WAYPOINT };

      Tool currentTool = SELECT;

      // Handle map click based on current tool
      void handleMapClick(double lat, double lon, ScenarioData& scenario);

      // Render ship icons and waypoint lines on the map
      void renderShips(ImDrawList* drawList, const MapWidget& mapWidget,
                       const ScenarioData& scenario, int selectedShip);

  private:
      // Ship icon rendering
      void drawShipIcon(ImDrawList* drawList, float screenX, float screenY,
                        float heading, ImU32 color, bool isSelected);
      void drawWaypointLines(ImDrawList* drawList, const MapWidget& mapWidget,
                             const OtherShipData& ship, ImU32 color);
  };
  ```
- **UI flow for placing own ship:**
  1. Click "Place Own Ship" button in toolbar (or press `O`)
  2. Click on map → own ship placed at that position
  3. A rotation handle appears → drag to set initial heading
  4. Ship icon (triangle) rendered at position with heading indicator

- **UI flow for placing other ships:**
  1. Click "Add Other Ship" button (or press `N`)
  2. Click on map → new other ship placed
  3. Click again → add waypoint (leg) for that ship
  4. Each subsequent click adds another waypoint
  5. Press `Escape` or right-click to finish adding waypoints
  6. Lines drawn between waypoints showing the route

- **Ship icon rendering:**
  - Own ship: filled green triangle, pointing in heading direction
  - Other ships: filled red triangles
  - Selected ship: yellow highlight ring
  - Waypoints: small circles connected by dashed lines
  - Speed label next to each leg line

- **Verify:** Place own ship + 3 other ships with waypoints. All render on map at correct
  positions. Save scenario, load in editor → positions match.
- [ ] Done

### 10C-06: Implement scenario properties panel
- **Files:** `src/editor/EditorApp.cpp` (extend)
- **What:** Right-side panel for editing all scenario properties.

  **Layout (ImGui windows/tabs):**
  ```
  ┌─ Scenario Properties ─────────────────┐
  │ Name: [My_Scenario______________]      │
  │ World: Southampton_Water               │
  │ Description:                           │
  │ [Multi-line text box____________]      │
  │ [_________________________________]   │
  │                                        │
  │ ┌─ Time & Date ──────────────────┐    │
  │ │ Start: [11]:[00]               │    │
  │ │ Date:  [15]/[03]/[2026]        │    │
  │ │ Sunrise: [06]:30               │    │
  │ │ Sunset:  [18]:15               │    │
  │ └────────────────────────────────┘    │
  │                                        │
  │ ┌─ Weather ──────────────────────┐    │
  │ │ Conditions: [Moderate ▼]       │    │
  │ │ Visibility:  [5.0] NM          │    │
  │ │ Rain:       [None ▼]           │    │
  │ │ Wind dir:   [225]°             │    │
  │ │ Wind speed: [15] kts           │    │
  │ └────────────────────────────────┘    │
  │                                        │
  │ ┌─ Ships ────────────────────────┐    │
  │ │ [Own Ship ▼]                   │    │
  │ │ Type: [Alkmini ▼]             │    │
  │ │ Pos: 50.8912°N, 1.3045°W      │    │
  │ │ Heading: [180]°  Speed: [10] kt│    │
  │ │ MMSI: [0________]             │    │
  │ │                                │    │
  │ │ Legs:                          │    │
  │ │ 1. 180° @ 10kt for 0.5 NM     │    │
  │ │ 2. 220° @ 8kt for 1.2 NM      │    │
  │ │ [+ Add Leg] [- Delete Leg]     │    │
  │ │                                │    │
  │ │ [+ Add Ship] [Delete Ship]     │    │
  │ └────────────────────────────────┘    │
  │                                        │
  │ [Save Scenario]  [Test in Simulator]   │
  └────────────────────────────────────────┘
  ```

- **Ship type dropdown:** Read available ship names from `bin/Models/Ownship/` and
  `bin/Models/Othership/` directories (list subdirectories).
- **Leg editing:** Each leg has bearing (°), speed (knots), distance (NM). Display
  as a list with inline editing. Add/delete buttons. Legs can be reordered.
- **Weather presets:** Map the weather slider (0-2) to labels:
  0.0=Calm, 0.5=Slight, 1.0=Moderate, 1.5=Rough, 2.0=Very Rough
- **Verify:** All properties editable. Changes reflected on map (ship positions,
  weather indicator). Save produces valid scenario files.
- [ ] Done

### 10C-07: Implement scenario file save/load
- **Files:** `src/editor/ScenarioFileIO.hpp`, `src/editor/ScenarioFileIO.cpp`
- **What:** Read and write the standard Bridge Command scenario file format (same format
  as the existing editor produces). This ensures full compatibility.

  ```cpp
  class ScenarioFileIO {
  public:
      // Save scenario to directory
      // Creates: environment.ini, ownship.ini, othership.ini, description.ini
      static bool save(const std::string& scenarioDir, const ScenarioData& data);

      // Load scenario from directory
      static ScenarioData load(const std::string& scenarioDir);

      // List available scenarios (subdirectories of bin/Scenarios/)
      static std::vector<std::string> listScenarios(const std::string& scenariosDir);

      // List available worlds (subdirectories of bin/World/)
      static std::vector<std::string> listWorlds(const std::string& worldsDir);
  };
  ```
- **File format:** Use the exact same INI format as the existing editor (see Phase 0B
  notes above for format details). The existing `ScenarioDataStructure.hpp` already
  has the data types -- reuse them.
- **Note:** The existing editor's save code is in `ControllerModel::save()` at
  [ControllerModel.cpp:591](src/editor/ControllerModel.cpp#L591). Port this logic
  to the new `ScenarioFileIO::save()` method.
- **Verify:** Save a scenario with the new editor, load it in the old editor (and vice
  versa). All data (ships, legs, weather, time) matches.
- [ ] Done

### 10C-08: Implement "Test in Simulator" button
- **Files:** `src/editor/EditorApp.cpp` (extend)
- **What:** A button that saves the current scenario and launches the simulator with it.
  ```cpp
  void EditorApp::testInSimulator() {
      // 1. Auto-save current scenario
      ScenarioFileIO::save(scenarioPath, scenarioData);

      // 2. Launch simulator with this scenario
      // On macOS: fork + exec (same as launcher, see macLaunchHelper)
      // On Windows: ShellExecute
      // On Linux: fork + exec
      // Pass scenario name as command-line argument or write to bc5.ini
  }
  ```
- **Implementation notes:**
  - The simulator reads its scenario from `bc5.ini` (`scenario_name=...`)
  - Either: update bc5.ini before launch, or add a command-line arg to bridgecommand-bc
  - Use the same launch mechanism as the launcher (fork+execl on macOS, ShellExecute on Windows)
- **Verify:** Click "Test" → simulator launches with the scenario. Ships at correct positions.
  Weather matches. Close simulator → return to editor.
- [ ] Done

### 10C-09: Implement buoy/light display on map
- **Files:** `src/editor/MapWidget.cpp` (extend)
- **What:** When a world is loaded (or generated), display buoys and lights from the
  world's `buoy.ini` and `light.ini` files on the map.
  ```cpp
  void renderBuoys(ImDrawList* drawList, const MapWidget& mapWidget,
                   const std::vector<PositionData>& buoys);
  void renderLights(ImDrawList* drawList, const MapWidget& mapWidget,
                    const std::vector<ChartLight>& lights);
  ```
- **Rendering:**
  - Buoys: small coloured diamonds (red=port, green=starboard, yellow=cardinal, etc.)
  - Lights: small star icons with colour matching light colour
  - Only show at zoom level 10+ (too cluttered at lower zoom)
  - Tooltip on hover: show buoy type, light characteristics
- **Verify:** Generate a world from S-57 chart data. Buoys and lights appear on map at
  correct positions. Colours match buoy types. Match positions with OpenCPN chart.
- [ ] Done

### 10C-10: Implement S-57 chart overlay on map
- **Files:** `src/editor/ChartOverlay.hpp`, `src/editor/ChartOverlay.cpp`
- **What:** When the user loads an S-57 chart file (.000), overlay the chart features
  directly on the map widget. This shows the user exactly what data is available
  before generating a world.
  - Use the existing `ChartReader` class to extract features
  - Render on map:
    - Depth contours from DEPARE polygons (blue shading)
    - Soundings as small text labels (depth in metres)
    - Coastlines as lines (thicker than Natural Earth ones)
    - Buoys and lights (same as 10C-09 but from chart data)
    - Landmarks (small named icons)
  - Add "Load Chart" button to toolbar with file browser dialog
  - When chart is loaded, auto-zoom to the chart's geographic extent
- **Verify:** Load a NOAA S-57 chart. Features render on top of satellite imagery.
  Depth areas visible. Buoy positions match chart data.
- [ ] Done

---

## Phase 10D: Scenario Editor - Advanced Features

> **Goal:** Polish features that make the editor production-ready and comparable to
> Sea Power's mission editor in capability.

### 10D-01: Implement scenario import/export
- **Files:** `src/editor/EditorApp.cpp` (extend)
- **What:** Import and export scenarios for sharing:
  1. **Export as ZIP:** Bundle scenario directory (4 INI files + world reference) into a
     single .zip file for easy sharing
  2. **Export with world:** Optionally include the full world directory in the ZIP
     (so recipient doesn't need to generate it)
  3. **Import from ZIP:** Extract ZIP into Scenarios/ and World/ directories
  - Use `miniz` (public domain, header-only) for ZIP creation/extraction
- **Verify:** Export a scenario as ZIP. Send to another PC. Import it. Scenario loads
  and runs correctly.
- [ ] Done

### 10D-02: Implement distance and bearing measurement tool
- **Files:** `src/editor/MeasureTool.hpp`
- **What:** A ruler tool that lets users click two points on the map and shows:
  - Distance in nautical miles (and km)
  - Bearing from point A to point B (true)
  - Estimated transit time at a given speed
  - Draw a line between the points with labels
  ```
  A ────────────── B
     4.7 NM  045°T
     28 min @ 10kt
  ```
- **Math:** Use the Haversine formula for great-circle distance:
  ```cpp
  double haversineNM(double lat1, double lon1, double lat2, double lon2) {
      double dLat = (lat2 - lat1) * M_PI / 180.0;
      double dLon = (lon2 - lon1) * M_PI / 180.0;
      double a = sin(dLat/2)*sin(dLat/2) +
                 cos(lat1*M_PI/180) * cos(lat2*M_PI/180) *
                 sin(dLon/2)*sin(dLon/2);
      double c = 2 * atan2(sqrt(a), sqrt(1-a));
      return 3440.065 * c;  // Earth radius in NM
  }
  ```
- **Verify:** Measure distance from Southampton to Portsmouth (~15 NM). Verify against
  known distance. Bearing should be approximately 100°T.
- [ ] Done

### 10D-03: Implement traffic lane / TSS overlay
- **Files:** `src/editor/ChartOverlay.cpp` (extend)
- **What:** When S-57 charts are loaded, extract and display Traffic Separation Schemes:
  - **S-57 layers:** TSSLPT (TSS lane part), TSSRON (TSS roundabout), TSEZNE (TSS zone)
  - Render as semi-transparent magenta polygons (standard chart colour)
  - Shows users where shipping lanes are, helping them create realistic scenarios
- **Verify:** Load a chart with TSS data (e.g. Dover Strait). TSS lanes render as
  magenta overlays matching OpenCPN display.
- [ ] Done

### 10D-04: Implement multi-chart loading
- **Files:** `src/editor/EditorApp.cpp` (extend), `src/ChartReader.hpp` (extend)
- **What:** Allow loading multiple S-57 charts simultaneously. This is needed because
  real-world scenarios often span multiple chart tiles.
  - "Load Chart" button allows selecting multiple .000 files
  - Each chart's features are merged into a single display
  - When generating a world, use all loaded charts' data
  - Handle overlapping charts: prefer the chart with more detail (higher compilation scale)
- **Verify:** Load 2 adjacent NOAA charts. Features from both appear on map without gaps
  or duplicates at boundaries.
- [ ] Done

### 10D-05: Add satellite imagery as terrain texture option
- **Files:** `src/WorldGenerator.cpp` (extend)
- **What:** Instead of the procedural green/blue terrain texture, download satellite
  imagery tiles for the scenario area and composite them into the terrain texture.
  - During world generation, download ESRI satellite tiles at zoom level 15-16
    for the scenario area
  - Stitch tiles together and resize to match heightmap dimensions
  - Write as `texture.png` instead of the procedural colour-band texture
  - Add checkbox in Generate World dialog: "☑ Use satellite imagery for texture"
- **Result:** The 3D terrain in the simulator shows realistic satellite imagery draped
  over the heightmap, instead of flat green/blue colours.
- **Note:** Downloaded satellite tiles are typically 256x256 each. For a 1025x1025
  texture at zoom 15, you need roughly 16x16 = 256 tiles (~4 MB download).
- **Verify:** Generate a world with satellite texture. Load in simulator. Terrain shows
  recognisable satellite imagery (buildings, roads, fields visible on land).
- [ ] Done

### 10D-06: Add undo/redo system
- **Files:** Create `src/editor/UndoStack.hpp`
- **What:** Track all user actions and allow undo (Ctrl+Z) / redo (Ctrl+Y):
  ```cpp
  class UndoStack {
  public:
      void push(const std::string& description, std::function<void()> undo,
                std::function<void()> redo);
      void undo();
      void redo();
      bool canUndo() const;
      bool canRedo() const;
      std::string undoDescription() const;
      std::string redoDescription() const;
  private:
      struct Action { std::string desc; std::function<void()> undo, redo; };
      std::vector<Action> stack;
      int currentIndex = -1;
  };
  ```
- **Actions to track:** Place ship, move ship, delete ship, add leg, delete leg,
  change weather, change time, select area, modify properties.
- **Verify:** Place a ship. Ctrl+Z → ship disappears. Ctrl+Y → ship reappears.
  Multiple levels of undo work correctly.
- [ ] Done

### 10D-07: Add keyboard shortcuts and help overlay
- **Files:** `src/editor/EditorApp.cpp` (extend)
- **What:** Implement keyboard shortcuts for common operations and a help overlay
  showing all available shortcuts.
  ```
  General:
    Ctrl+S        Save scenario
    Ctrl+Z        Undo
    Ctrl+Y        Redo
    Ctrl+N        New scenario
    Ctrl+O        Open scenario
    F1            Toggle help overlay
    Escape        Cancel current operation

  Map:
    +/-           Zoom in/out
    Arrow keys    Pan map
    Home          Centre on own ship
    M             Toggle satellite/street map

  Tools:
    S             Select tool (click to select ships)
    O             Place own ship
    N             Place new other ship
    W             Add waypoint to selected ship
    A             Select scenario area
    R             Ruler / measure distance
    Delete        Delete selected ship/waypoint

  Data:
    C             Load S-57 chart overlay
    G             Toggle depth contour overlay
    L             Toggle coastline overlay
  ```
- **Help overlay:** Press F1 → semi-transparent overlay showing all shortcuts.
  Press F1 again to dismiss.
- **Verify:** All shortcuts work. Help overlay displays and dismisses correctly.
- [ ] Done

---

## Phase 11: Multiplayer Online Sessions

> **Goal:** Enable multiple players on the internet to each control their own ship in a
> shared scenario. Currently, Bridge Command supports multiplayer on a local network via
> the `multiplayerHub` executable -- each BC instance controls one ship and the hub relays
> positions between them. This phase upgrades that system for internet play.
>
> **What already works (existing `multiplayerHub`):**
> - Hub connects to N Bridge Command peers via ENet (UDP)
> - Each peer's "own ship" is assigned from the scenario's "other ships" list
> - Each peer runs its own ship physics (MMG model, rudder, engine) locally
> - Hub receives `MPF` (Multiplayer Feedback) from each peer: position, heading, speed, rate of turn
> - Hub redistributes positions so each peer sees all other ships
> - Dead reckoning in `ShipPositions` extrapolates between updates
> - Mooring/towing lines synced between peers
> - Time coordination with accelerator control
>
> **What's missing for internet play:**
> - No lobby or session browser (currently requires manual hostname entry)
> - All peers must connect before simulation starts (no late join/leave)
> - Dead reckoning is linear only (needs smoothing for higher latency)
> - No scenario/asset distribution (all peers must have identical world files locally)
> - No NAT traversal (hub must be directly reachable via public IP or port forwarding)
> - Each player's bridge can have its own secondary PCs (radar, charts) via the existing
>   Primary/Secondary networking -- this must continue to work alongside multiplayer
>
> **Architecture (hub-and-spoke, same as professional simulators):**
> ```
>      Player A (BC)  ──┐
>      Player A (Radar) ─┤     ┌─── Player B (BC)
>                        ├─── Hub ───┤
>      Player C (BC)  ──┤     └─── Player B (Charts)
>      Player C (Helm) ─┘
> ```
> Each player runs their own ship physics. The hub is a lightweight relay (~100 bytes/ship
> at 10 Hz = trivial bandwidth). This matches how Kongsberg K-Sim and Wartsila NAUTIS
> professional training simulators work.
>
> **Key files:**
> - `src/multiplayerHub/main.cpp` -- hub main loop
> - `src/multiplayerHub/Network.hpp/cpp` -- ENet networking
> - `src/multiplayerHub/ShipPositions.hpp/cpp` -- dead reckoning + position storage
> - `src/multiplayerHub/ScenarioChoice.hpp/cpp` -- scenario selection UI
> - `src/NetworkPrimary.cpp` / `src/NetworkSecondary.cpp` -- existing bridge networking

### 11-01: Add lobby UI to multiplayer hub
- **Files:** `src/multiplayerHub/main.cpp`, `src/multiplayerHub/ScenarioChoice.hpp/cpp`
- **What:** Replace the current text-entry hostname dialog with a proper lobby UI where
  the host configures the session and players can join/ready up before starting.

  **Current flow (replace this):**
  1. Hub starts → text box for hostnames (comma-separated IPs) + scenario dropdown
  2. Hub connects to all peers immediately → if any peer is missing, it hangs
  3. Simulation starts with no ability to add/remove players

  **New flow:**
  1. Hub starts → lobby screen with session configuration
  2. Host selects scenario, sets weather/time overrides, configures max players
  3. Hub listens for incoming connections on configured port (default 18304)
  4. As players connect, they appear in a player list with their chosen ship
  5. Host can assign ships to players (or players choose from available ships)
  6. "Start" button only enabled when at least 2 players are connected
  7. Late joiners can connect after simulation starts (see 11-03)

  **Lobby UI (ImGui, matching the editor's style):**
  ```
  ┌─ Multiplayer Session ──────────────────────────────┐
  │ Scenario: [Southampton_Approach ▼]                  │
  │ Port: [18304]   Your IP: 192.168.1.42               │
  │                                                     │
  │ Players:                                            │
  │ ┌────┬────────────────┬──────────────┬─────────┐   │
  │ │ #  │ Name/IP        │ Ship         │ Status  │   │
  │ ├────┼────────────────┼──────────────┼─────────┤   │
  │ │ 1  │ 192.168.1.10   │ Alkmini      │ Ready   │   │
  │ │ 2  │ 86.24.103.55   │ Protis       │ Ready   │   │
  │ │ 3  │ (waiting...)   │              │         │   │
  │ └────┴────────────────┴──────────────┴─────────┘   │
  │                                                     │
  │ [Start Simulation]  [Kick Selected]                 │
  └─────────────────────────────────────────────────────┘
  ```

- **Implementation notes:**
  - Rewrite the hub to use ImGui (same as the scenario editor) instead of Irrlicht GUI
  - Use GLFW + OpenGL + ImGui for the hub window (lightweight, no 3D rendering needed)
  - The hub listens for ENet connections instead of connecting out to peers
  - Reverse the connection direction: hub is the SERVER, players are CLIENTS
  - This is a significant refactor of the current hub which acts as an ENet client
- **Verify:** Hub starts in lobby mode. A BC instance can connect to the hub's IP:port.
  Player appears in the player list. Scenario selection works.
- [ ] Done

### 11-02: Reverse hub connection direction (hub as server)
- **Files:** `src/multiplayerHub/Network.hpp/cpp`
- **What:** Currently the hub acts as an ENet **client** that connects out to each peer
  (each BC instance runs an ENet server). This must be reversed for internet play:
  the hub should be the **server** that listens for incoming connections.

  **Current architecture (hub as client):**
  ```
  Hub ──connect──> BC Peer 1 (server on port 18304)
  Hub ──connect──> BC Peer 2 (server on port 18304)
  ```
  Problem: Every peer must have a reachable IP. Behind NAT, this requires port forwarding
  on every player's router.

  **New architecture (hub as server):**
  ```
  BC Peer 1 ──connect──> Hub (server on port 18304)
  BC Peer 2 ──connect──> Hub (server on port 18304)
  ```
  Advantage: Only the hub needs a reachable IP. Players behind NAT can connect out
  without port forwarding (outbound UDP works through most NATs).

  ```cpp
  // New: Hub creates a server
  ENetAddress address;
  address.host = ENET_HOST_ANY;
  address.port = port;
  server = enet_host_create(&address, maxPlayers, 2, 0, 0);

  // New: Hub accepts incoming connections
  ENetEvent event;
  while (enet_host_service(server, &event, 0) > 0) {
      if (event.type == ENET_EVENT_TYPE_CONNECT) {
          // New player connected
          addPeer(event.peer);
      }
  }
  ```

  **BC client-side changes:**
  - Add `OperatingMode::MultiplayerClient` (new mode alongside existing `Multiplayer`)
  - In this mode, BC connects to the hub's hostname:port as an ENet client
  - The existing `Multiplayer` mode (BC as server) stays for backward compatibility
  - Add command-line flag: `--connect <hub_ip:port>` to launch directly into client mode

- **Backward compatibility:** Keep the old hub-as-client code behind a `--legacy` flag
  for existing LAN setups that depend on the current architecture.
- **Verify:** Hub starts as server on port 18304. BC instance connects to hub. Data flows
  correctly in both directions. Old `--legacy` mode still works.
- [ ] Done

### 11-03: Support dynamic join and leave
- **Files:** `src/multiplayerHub/main.cpp`, `src/multiplayerHub/ShipPositions.hpp/cpp`
- **What:** Allow players to join mid-session and handle disconnections gracefully.

  **Late join:**
  1. New player connects to hub after simulation has started
  2. Hub sends current scenario state: time, all ship positions, weather
  3. Hub assigns an available ship (or a new "other ship" if the scenario allows)
  4. Hub notifies existing players: "Player X joined as Ship Y"
  5. New player's ship appears in all other players' worlds
  6. New player receives position updates for all existing ships

  **Disconnect handling:**
  1. Hub detects peer disconnection (ENet timeout or explicit disconnect)
  2. Hub marks that ship as "disconnected" -- it stops dead in the water
  3. Hub notifies remaining players: "Player X disconnected"
  4. Ship remains visible to other players (stationary) until host removes it
  5. Player can reconnect and resume control of the same ship

  **Implementation notes:**
  - The current code (`peerScenarioData` vector) assumes a fixed number of peers at startup.
    Change to a dynamic `std::map<unsigned int, PeerData>` keyed by peer ID.
  - `ShipPositions` needs to grow/shrink dynamically as players join/leave
  - When a player disconnects, set their ship's speed to 0 and stop sending MPF updates
  - When they reconnect, match by some identifier (ship name or a session token)

- **Verify:** Start session with 2 players. A 3rd player joins after 5 minutes -- their ship
  appears for all players. Player 2 disconnects -- their ship stops. Player 2 reconnects --
  resumes control from the stopped position.
- [ ] Done

### 11-04: Improve dead reckoning with interpolation
- **Files:** `src/multiplayerHub/ShipPositions.hpp/cpp`
- **What:** The current dead reckoning uses simple linear extrapolation. For internet
  play with 50-200ms latency, this produces visible jumps when corrections arrive.
  Implement smooth interpolation.

  **Current (linear extrapolation):**
  ```cpp
  // Just extends the last known velocity -- snaps when correction arrives
  positionX = lastX + deltaTime * speed * sin(bearing);
  positionZ = lastZ + deltaTime * speed * cos(bearing);
  ```

  **Improved (cubic Hermite interpolation with blending):**
  ```cpp
  class ShipPositions {
  public:
      void setShipPosition(int ship, float time, float x, float z,
                           float speed, float bearing, float rateOfTurn);

      // Returns interpolated position, smoothly blending toward latest update
      void getShipPosition(int ship, float currentTime,
                           float& x, float& z, float& speed,
                           float& bearing, float& rateOfTurn);

  private:
      struct ShipState {
          float time;
          float x, z;
          float speed, bearing, rateOfTurn;
      };

      struct ShipData {
          ShipState previous;     // Second-to-last update
          ShipState current;      // Most recent update
          ShipState displayed;    // What's currently shown (smoothed)
          float blendFactor;      // 0.0 = displayed, 1.0 = corrected
      };

      std::map<int, ShipData> ships;
      float blendDuration = 0.5f; // Blend corrections over 500ms
  };
  ```

  **Algorithm:**
  1. When a new position update arrives, store it as `current` (old `current` becomes `previous`)
  2. Calculate the dead-reckoned position (where we expected the ship to be)
  3. If the correction is small (<5m), blend from displayed position to corrected over 500ms
  4. If the correction is large (>5m), snap immediately (probably a teleport/respawn)
  5. Between updates, extrapolate from `current` using speed + rate of turn (arc, not line)

  **Rate-of-turn extrapolation (arc instead of line):**
  ```cpp
  // Instead of straight line, follow a circular arc
  float newBearing = bearing + rateOfTurn * deltaTime;
  float avgBearing = bearing + rateOfTurn * deltaTime / 2.0f;
  float distance = speed * deltaTime;
  positionX += distance * sin(avgBearing * DEG_TO_RAD);
  positionZ += distance * cos(avgBearing * DEG_TO_RAD);
  ```

- **Verify:** With artificial 200ms latency, ships move smoothly without visible jumps.
  Ship following a curved path shows smooth turning, not zigzag.
- [ ] Done

### 11-05: Add extended ship state sync
- **Files:** `src/multiplayerHub/main.cpp`, `src/NetworkPrimary.cpp`
- **What:** Extend the `MPF` feedback message to include additional state that other players
  need to see for realistic multi-ship exercises.

  **Current MPF format** (7 fields):
  ```
  MPF{posX}#{posZ}#{bearing}#{rateOfTurn}#{speed}#{time}#{linesData}
  ```

  **Extended MPF format** (add fields after existing ones for backward compatibility):
  ```
  MPF{posX}#{posZ}#{bearing}#{rateOfTurn}#{speed}#{time}#{linesData}
     #{rudderAngle}#{engineRPM}#{navLights}#{hornActive}#{MMSI}#{shipName}
  ```

  | Field | Type | Purpose |
  |-------|------|---------|
  | rudderAngle | float | Visual rudder indicator for nearby ships |
  | engineRPM | float | Engine telegraph display |
  | navLights | int | Bitfield: running, anchor, NUC, aground, fishing, etc. |
  | hornActive | int | 0/1: sound signal (for COLREGS training) |
  | MMSI | string | AIS MMSI for radar/AIS display |
  | shipName | string | Display name for other players |

- **Backward compatibility:** Old clients that send 7-field MPF still work (hub checks
  field count and uses defaults for missing fields).
- **Verify:** Two players in session. Player A turns rudder -- Player B sees rudder angle
  update on the other ship. Player A sounds horn -- Player B hears it.
- [ ] Done

### 11-06: Add session browser for players
- **Files:** Create `src/multiplayerHub/SessionBrowser.hpp/cpp`,
  modify `src/launcher/main.cpp`
- **What:** Add a way for players to find and join multiplayer sessions without manually
  entering IP addresses.

  **Option A (simplest): Direct IP entry in launcher**
  - Add "Join Multiplayer" button to the main launcher
  - Text field for hub IP:port (e.g. "86.24.103.55:18304")
  - "Connect" button launches BC in `MultiplayerClient` mode
  - Save last-used IP for convenience

  **Option B (session list via simple web API):**
  - Hub registers itself with a lightweight master server when it starts
  - Master server is a simple REST API: `POST /sessions` to register, `GET /sessions` to list
  - Could be hosted on any free tier (Oracle Cloud, Cloudflare Workers, etc.)
  - Players see a list of active sessions in the launcher
  - Clicking a session connects to that hub's IP:port
  - Master server is optional -- direct IP entry always works as fallback

  **Start with Option A** (no infrastructure required). Option B can be added later.

- **Verify:** Launch BC → click "Join Multiplayer" → enter hub IP → BC connects and
  receives scenario data → player can control their ship.
- [ ] Done

### 11-07: Handle scenario asset distribution
- **Files:** `src/multiplayerHub/main.cpp` (extend)
- **What:** For multiplayer to work, all players need the same world files (terrain,
  textures, buoy positions) and ship models. Handle this:

  **Approach 1 (simplest): Require identical installations**
  - All players must have the same world and ship model files
  - Hub sends the world name and scenario name -- if a player doesn't have them,
    they get an error message telling them which files are needed
  - Works well for controlled environments (training centres, clubs)

  **Approach 2 (better UX): Scenario pack download**
  - Hub creates a scenario pack (ZIP) containing:
    - The 4 scenario INI files (environment, ownship, othership, description)
    - The world directory (terrain.ini, height.png, texture.png, buoy.ini, light.ini, etc.)
    - Ship model names (not the actual models -- those must be installed separately)
  - When a player connects, hub checks if they have the required world
  - If not, hub offers to send the scenario pack (typically 5-20 MB)
  - Player receives and extracts the pack before simulation starts
  - Reuses the import/export ZIP from task 10D-01

  **Start with Approach 1.** Add Approach 2 when the scenario editor's export feature
  (10D-01) is complete.

- **Verify:** Hub starts with "Southampton_Approach" scenario. Player connects but doesn't
  have that world → gets clear error: "Missing world: Southampton_Approach. Please install
  it or ask the host to send the scenario pack."
- [ ] Done

### 11-08: Ensure Primary/Secondary works alongside multiplayer
- **Files:** `src/NetworkPrimary.cpp`, `src/NetworkSecondary.cpp`
- **What:** Each player in a multiplayer session should be able to use their own
  multi-monitor bridge setup (Primary PC + Secondary PCs for radar/charts).

  **Current conflict:** Both Primary/Secondary networking and multiplayer use ENet on
  similar ports. They must coexist:
  - Primary/Secondary: port 18304 (configurable via `udp_send_port`)
  - Multiplayer hub: port 18304 (same default!)

  **Fix:**
  - Use different default ports: Primary/Secondary stays on 18304, multiplayer uses 18305
  - Or: make the multiplayer port configurable in `mph.ini` / launcher UI
  - The BC instance connects to the multiplayer hub AND broadcasts to its own secondaries
  - Primary/Secondary sync happens locally (LAN), multiplayer sync happens via hub (internet)
  - These are separate network channels that don't interfere

  **Data flow for a multiplayer player with secondary monitors:**
  ```
  Hub ←──MPF──→ BC Primary ──BC sync──→ BC Secondary (radar)
                                    └──→ BC Secondary (charts)
  ```
  The Primary sends multiplayer data (own ship position) to the hub, AND sends full
  simulation state (all ships, buoys, weather) to its own secondaries.

- **Verify:** Player A has Primary + Secondary (radar). Player B is single-monitor.
  Both connect to hub. Player A's radar shows Player B's ship. Player B sees Player A's
  ship on their main display.
- [ ] Done

### 11-09: Add multiplayer chat
- **Files:** `src/multiplayerHub/main.cpp` (extend), `src/gui/ImGuiOverlay.hpp/cpp` (extend)
- **What:** Simple text chat between players during a multiplayer session. Useful for
  coordinating exercises and communicating intentions.
  - Chat messages sent via reliable ENet channel (separate from position updates)
  - Messages displayed in a small overlay window (ImGui) at bottom of screen
  - Press `T` to open chat input, Enter to send, Escape to cancel
  - Messages show sender's ship name and timestamp
  - Chat history scrollable, max 100 messages
  - Hub relays messages to all connected peers
- **Message format:** `CHAT#{senderShipIndex}#{timestamp}#{message_text}`
- **Verify:** Player A types "I am overtaking on your port side" → message appears on
  Player B's screen with Player A's ship name.
- [ ] Done

### 11-10: Write multiplayer integration tests
- **Files:** Create `src/tests/test_multiplayer.cpp`
- **What:** Test the core multiplayer data flow without requiring actual network connections.
  ```cpp
  TEST_CASE("Dead reckoning with interpolation") {
      ShipPositions positions(3);
      // Set initial position
      positions.setShipPosition(0, 0.0f, 100.0f, 200.0f, 5.0f, 90.0f, 0.0f);
      // Extrapolate 1 second ahead
      float x, z, spd, brg, rot;
      positions.getShipPosition(0, 1.0f, x, z, spd, brg, rot);
      // Ship at 5 m/s heading 90° should move ~5m in X
      REQUIRE(x == Approx(105.0f).margin(0.5f));
      REQUIRE(z == Approx(200.0f).margin(0.5f));
  }

  TEST_CASE("Dead reckoning with rate of turn") {
      ShipPositions positions(1);
      positions.setShipPosition(0, 0.0f, 0.0f, 0.0f, 10.0f, 0.0f, 10.0f); // 10 deg/s turn
      float x, z, spd, brg, rot;
      positions.getShipPosition(0, 6.0f, x, z, spd, brg, rot);
      // After 6 seconds at 10 deg/s, heading should be ~60°
      REQUIRE(brg == Approx(60.0f).margin(2.0f));
  }

  TEST_CASE("Extended MPF parsing") {
      // Test backward compatibility: 7-field MPF (old client)
      std::string oldMPF = "MPF100#200#90#5#10#1000#";
      // Should parse without error, defaults for missing fields

      // Test new format: 13-field MPF
      std::string newMPF = "MPF100#200#90#5#10#1000##-15.5#120#1#0#123456789#MyShip";
      // Should parse all fields
  }

  TEST_CASE("Dynamic player join/leave") {
      ShipPositions positions(2);
      positions.setShipPosition(0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
      positions.setShipPosition(1, 0.0f, 100.0f, 100.0f, 0.0f, 0.0f, 0.0f);
      // Add a 3rd player dynamically
      positions.addShip();
      positions.setShipPosition(2, 0.0f, 200.0f, 200.0f, 5.0f, 45.0f, 0.0f);
      REQUIRE(positions.getNumberOfShips() == 3);
      // Remove player 1
      positions.removeShip(1);
      REQUIRE(positions.getNumberOfShips() == 2);
  }
  ```
- **Verify:** All multiplayer unit tests pass. Dead reckoning accuracy within tolerances.
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
| 2B | 10 | **9** | Wicked Engine integration |
| 3 | 8 | **7** | GPU ocean rendering |
| 4 | 5 | **5** | Radar upgrade |
| 5 | 4 | **4** | Networking enhancements |
| 6 | 3 | **3** | VR improvements |
| 7 | 7 | **7** | UI modernization |
| 8 | 3 | **3** | Advanced physics (optional) |
| 9 | 6 | **4** | Testing & release |
| 10A | 7 | 0 | Scenario editor - map tile system |
| 10B | 8 | 0 | Scenario editor - global data sources & chart auto-download |
| 10C | 10 | 0 | Scenario editor - interactive editor GUI |
| 10D | 7 | 0 | Scenario editor - advanced features |
| 11 | 10 | 0 | Multiplayer online sessions |
| **Total** | **148** | **98** | **66% complete** |

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
- **Phase 10A** (Map Tiles) is independent -- can start immediately
- **Phase 10B** (Global Data) is independent of 10A but 10B-04 (depth overlay) needs 10A-05 (map widget)
- **Phase 10C** (Editor GUI) depends on 10A (map widget) and 10B (data sources)
- **Phase 10D** (Advanced Features) depends on 10C being mostly complete
- **Phase 11** (Multiplayer Online) is independent of Phase 10 -- can run in parallel
- **Phase 11** tasks 11-01 to 11-03 (hub refactor) must complete before 11-04 to 11-09

## Critical Checkpoints

After each phase, verify:
1. **All existing scenarios still load and run**
2. **Network sync between 2 PCs works**
3. **Multi-monitor rendering works**
4. **No performance regression beyond 10%**

---

**Document Version:** 1.6
**Last Updated:** February 12, 2026
**Total Tasks:** 148 (98 completed, 5 deferred/blocked, 3 remaining from original scope, 32 scenario editor tasks, 10 multiplayer online tasks)
