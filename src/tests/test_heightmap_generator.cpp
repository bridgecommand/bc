#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#ifdef WITH_GDAL
#include "HeightmapGenerator.hpp"

using Catch::Approx;

// ── encodeRGB ───────────────────────────────────────────────────────────────

TEST_CASE("encodeRGB sea level encodes to 32768", "[heightmap]") {
    // Height = 0 → encoded = 32768 → R=128, G=0, B=0
    std::vector<std::vector<float>> grid = {{0.0f}};
    auto rgb = HeightmapGenerator::encodeRGB(grid);
    REQUIRE(rgb.size() == 3);
    REQUIRE(rgb[0] == 128); // R = 32768 / 256
    REQUIRE(rgb[1] == 0);   // G = 32768 % 256
    REQUIRE(rgb[2] == 0);   // B = 0 (no fractional)
}

TEST_CASE("encodeRGB positive height", "[heightmap]") {
    // Height = 10.0 → encoded = 32778 → R=128, G=10, B=0
    std::vector<std::vector<float>> grid = {{10.0f}};
    auto rgb = HeightmapGenerator::encodeRGB(grid);
    REQUIRE(rgb[0] == 128); // R = 32778 / 256
    REQUIRE(rgb[1] == 10);  // G = 32778 % 256
    REQUIRE(rgb[2] == 0);
}

TEST_CASE("encodeRGB negative depth", "[heightmap]") {
    // Height = -20.0 → encoded = 32748 → R=127, G=220, B=0
    std::vector<std::vector<float>> grid = {{-20.0f}};
    auto rgb = HeightmapGenerator::encodeRGB(grid);
    REQUIRE(rgb[0] == 127); // R = 32748 / 256
    REQUIRE(rgb[1] == 220); // G = 32748 % 256
    REQUIRE(rgb[2] == 0);
}

TEST_CASE("encodeRGB round-trip accuracy", "[heightmap]") {
    // Verify that encoding then decoding gets back close to original
    std::vector<float> testHeights = {-100.0f, -50.5f, -10.0f, 0.0f, 5.0f, 25.0f, 100.0f};

    for (float h : testHeights) {
        std::vector<std::vector<float>> grid = {{h}};
        auto rgb = HeightmapGenerator::encodeRGB(grid);

        // Decode: Height = R*256 + G + B/256.0 - 32768
        float decoded = static_cast<float>(rgb[0]) * 256.0f +
                        static_cast<float>(rgb[1]) +
                        static_cast<float>(rgb[2]) / 256.0f - 32768.0f;

        // Should be within 1 metre of original (B channel gives ~0.004m precision)
        REQUIRE(decoded == Approx(h).margin(1.0));
    }
}

TEST_CASE("encodeRGB multi-pixel grid", "[heightmap]") {
    std::vector<std::vector<float>> grid = {
        {0.0f, 10.0f},
        {-5.0f, 50.0f}
    };
    auto rgb = HeightmapGenerator::encodeRGB(grid);
    REQUIRE(rgb.size() == 12); // 2x2 * 3 channels
}

TEST_CASE("encodeRGB empty grid returns empty", "[heightmap]") {
    std::vector<std::vector<float>> grid;
    auto rgb = HeightmapGenerator::encodeRGB(grid);
    REQUIRE(rgb.empty());
}

// ── computeBoundsFromData ───────────────────────────────────────────────────

TEST_CASE("computeBoundsFromData with soundings", "[heightmap]") {
    HeightmapGenerator gen;
    std::vector<Sounding> soundings = {
        {-122.60, 48.30, 10.0},
        {-122.50, 48.40, 20.0},
    };
    gen.setSoundings(soundings);

    auto bounds = gen.computeBoundsFromData();
    REQUIRE(bounds.minLon < -122.60);
    REQUIRE(bounds.maxLon > -122.50);
    REQUIRE(bounds.minLat < 48.30);
    REQUIRE(bounds.maxLat > 48.40);
}

TEST_CASE("computeBoundsFromData with depth areas", "[heightmap]") {
    HeightmapGenerator gen;
    std::vector<DepthArea> areas;
    DepthArea a;
    a.minDepth = 5.0;
    a.maxDepth = 10.0;
    a.boundary = {{-122.55, 48.35}, {-122.50, 48.35}, {-122.50, 48.40}, {-122.55, 48.40}, {-122.55, 48.35}};
    areas.push_back(a);
    gen.setDepthAreas(areas);

    auto bounds = gen.computeBoundsFromData();
    REQUIRE(bounds.minLon < -122.55);
    REQUIRE(bounds.maxLon > -122.50);
}

// ── generate basic tests ────────────────────────────────────────────────────

TEST_CASE("generate with simple depth area", "[heightmap]") {
    HeightmapGenerator gen;

    // Create a depth area covering the entire bounds
    DepthArea area;
    area.minDepth = 8.0;
    area.maxDepth = 12.0;
    area.boundary = {
        {-122.60, 48.30}, {-122.40, 48.30},
        {-122.40, 48.50}, {-122.60, 48.50},
        {-122.60, 48.30} // closed
    };
    gen.setDepthAreas({area});

    HeightmapBounds bounds{-122.60, -122.40, 48.30, 48.50};
    gen.setBounds(bounds);

    HeightmapParams params;
    params.resolution = 5; // Small for testing
    auto grid = gen.generate(params);

    REQUIRE(grid.size() == 5);
    REQUIRE(grid[0].size() == 5);

    // Interior pixels should have depth ≈ -10.0 (avg of 8 and 12)
    // Check center pixel
    REQUIRE(grid[2][2] == Approx(-10.0f).margin(1.0));
}

TEST_CASE("generate land areas have positive height", "[heightmap]") {
    HeightmapGenerator gen;

    // Create a land polygon covering the area
    CoastlineSegment land;
    land.points = {
        {-122.60, 48.30}, {-122.40, 48.30},
        {-122.40, 48.50}, {-122.60, 48.50},
        {-122.60, 48.30} // closed
    };
    gen.setCoastlines({land});

    HeightmapBounds bounds{-122.60, -122.40, 48.30, 48.50};
    gen.setBounds(bounds);

    HeightmapParams params;
    params.resolution = 5;
    params.defaultLandHeight = 5.0;
    auto grid = gen.generate(params);

    // Center pixel should be land
    REQUIRE(grid[2][2] == Approx(5.0f));
}

TEST_CASE("generate empty data returns zero grid", "[heightmap]") {
    HeightmapGenerator gen;

    HeightmapBounds bounds{-122.60, -122.40, 48.30, 48.50};
    gen.setBounds(bounds);

    HeightmapParams params;
    params.resolution = 3;
    params.defaultSeaDepth = 0.0;
    auto grid = gen.generate(params);

    REQUIRE(grid.size() == 3);
    for (const auto& row : grid) {
        for (float v : row) {
            REQUIRE(v == Approx(0.0f));
        }
    }
}

// ── DEMTile / sampleDEM tests ───────────────────────────────────────────────

TEST_CASE("sampleDEM returns NaN when no tiles loaded", "[heightmap]") {
    HeightmapGenerator gen;
    float val = gen.sampleDEM(-122.5, 48.4);
    REQUIRE(std::isnan(val));
}

TEST_CASE("generate with land uses DEM when available", "[heightmap]") {
    HeightmapGenerator gen;

    // Create a land polygon covering the area
    CoastlineSegment land;
    land.points = {
        {-122.60, 48.30}, {-122.40, 48.30},
        {-122.40, 48.50}, {-122.60, 48.50},
        {-122.60, 48.30} // closed
    };
    gen.setCoastlines({land});

    // Without DEM, land should get defaultLandHeight
    HeightmapBounds bounds{-122.60, -122.40, 48.30, 48.50};
    gen.setBounds(bounds);

    HeightmapParams params;
    params.resolution = 5;
    params.defaultLandHeight = 5.0;
    auto grid = gen.generate(params);
    REQUIRE(grid[2][2] == Approx(5.0f));
}

#endif // WITH_GDAL
