#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#ifdef WITH_GDAL
#include "WorldGenerator.hpp"

using Catch::Approx;

// ── generateTerrainIni ──────────────────────────────────────────────────────

TEST_CASE("generateTerrainIni produces valid format", "[worldgen]") {
    HeightmapBounds bounds{-122.60, -122.40, 48.30, 48.50};
    std::string ini = WorldGenerator::generateTerrainIni(bounds, 50.0f, 100.0f, 1025);

    REQUIRE(ini.find("Number=1") != std::string::npos);
    REQUIRE(ini.find("HeightMap(1)=height.png") != std::string::npos);
    REQUIRE(ini.find("Texture(1)=texture.png") != std::string::npos);
    REQUIRE(ini.find("MapImage=map.png") != std::string::npos);
    REQUIRE(ini.find("TerrainLong(1)=") != std::string::npos);
    REQUIRE(ini.find("TerrainLat(1)=") != std::string::npos);
    REQUIRE(ini.find("TerrainLongExtent(1)=") != std::string::npos);
    REQUIRE(ini.find("TerrainLatExtent(1)=") != std::string::npos);
    REQUIRE(ini.find("TerrainMaxHeight(1)=50") != std::string::npos);
    REQUIRE(ini.find("SeaMaxDepth(1)=100") != std::string::npos);
    REQUIRE(ini.find("UsesRGB(1)=1") != std::string::npos);
    REQUIRE(ini.find("TerrainHeightMapRows(1)=1025") != std::string::npos);
    REQUIRE(ini.find("TerrainHeightMapColumns(1)=1025") != std::string::npos);
}

TEST_CASE("generateTerrainIni custom filenames", "[worldgen]") {
    HeightmapBounds bounds{-1.0, 1.0, 50.0, 51.0};
    std::string ini = WorldGenerator::generateTerrainIni(bounds, 10.0f, 20.0f, 513,
                                                          "custom_height.png",
                                                          "custom_texture.png",
                                                          "custom_map.png");

    REQUIRE(ini.find("HeightMap(1)=custom_height.png") != std::string::npos);
    REQUIRE(ini.find("Texture(1)=custom_texture.png") != std::string::npos);
    REQUIRE(ini.find("MapImage=custom_map.png") != std::string::npos);
    REQUIRE(ini.find("TerrainHeightMapRows(1)=513") != std::string::npos);
}

TEST_CASE("generateTerrainIni clamps small maxHeight", "[worldgen]") {
    HeightmapBounds bounds{0, 1, 0, 1};
    std::string ini = WorldGenerator::generateTerrainIni(bounds, 0.0f, 0.0f, 257);
    // Should use minimum values when 0 is passed
    REQUIRE(ini.find("TerrainMaxHeight(1)=5") != std::string::npos);
    REQUIRE(ini.find("SeaMaxDepth(1)=10") != std::string::npos);
}

// ── generateTideIni / generateTidalStreamIni ────────────────────────────────

TEST_CASE("generateTideIni empty default", "[worldgen]") {
    std::string ini = WorldGenerator::generateTideIni();
    REQUIRE(ini.find("Number=0") != std::string::npos);
}

TEST_CASE("generateTidalStreamIni empty default", "[worldgen]") {
    std::string ini = WorldGenerator::generateTidalStreamIni();
    REQUIRE(ini.find("Number=0") != std::string::npos);
}

// ── generateTexture ─────────────────────────────────────────────────────────

TEST_CASE("generateTexture all sea produces blue", "[worldgen]") {
    // No coastlines = all sea
    std::vector<CoastlineSegment> coastlines;
    HeightmapBounds bounds{-1, 1, 50, 51};
    auto rgb = WorldGenerator::generateTexture(coastlines, bounds, 4);

    REQUIRE(rgb.size() == 4 * 4 * 3);
    // Every pixel should be sea blue (40, 80, 140)
    for (int i = 0; i < 4 * 4; i++) {
        REQUIRE(rgb[i * 3 + 0] == 40);
        REQUIRE(rgb[i * 3 + 1] == 80);
        REQUIRE(rgb[i * 3 + 2] == 140);
    }
}

TEST_CASE("generateTexture with land polygon", "[worldgen]") {
    // Create a land polygon covering the whole area
    CoastlineSegment land;
    land.points = {{-2, 49}, {2, 49}, {2, 52}, {-2, 52}, {-2, 49}};
    HeightmapBounds bounds{-1, 1, 50, 51};
    auto rgb = WorldGenerator::generateTexture({land}, bounds, 4);

    // Interior pixels should be green (60, 120, 40)
    // Check center-ish pixel
    int centerIdx = (1 * 4 + 1) * 3; // row 1, col 1
    REQUIRE(rgb[centerIdx + 0] == 60);
    REQUIRE(rgb[centerIdx + 1] == 120);
    REQUIRE(rgb[centerIdx + 2] == 40);
}

// ── Height-aware generateTexture ────────────────────────────────────────────

TEST_CASE("generateTexture height-aware deep water is dark blue", "[worldgen]") {
    // 4x4 grid, all deep water (-30m)
    std::vector<std::vector<float>> grid(4, std::vector<float>(4, -30.0f));
    std::vector<UrbanArea> noUrban;
    HeightmapBounds bounds{-1, 1, 50, 51};
    auto rgb = WorldGenerator::generateTexture(grid, noUrban, bounds);

    REQUIRE(rgb.size() == 4 * 4 * 3);
    for (int i = 0; i < 16; i++) {
        REQUIRE(rgb[i * 3 + 0] == 30);
        REQUIRE(rgb[i * 3 + 1] == 60);
        REQUIRE(rgb[i * 3 + 2] == 120);
    }
}

TEST_CASE("generateTexture height-aware shallow water is light blue", "[worldgen]") {
    std::vector<std::vector<float>> grid(4, std::vector<float>(4, -3.0f));
    std::vector<UrbanArea> noUrban;
    HeightmapBounds bounds{-1, 1, 50, 51};
    auto rgb = WorldGenerator::generateTexture(grid, noUrban, bounds);

    for (int i = 0; i < 16; i++) {
        REQUIRE(rgb[i * 3 + 0] == 80);
        REQUIRE(rgb[i * 3 + 1] == 140);
        REQUIRE(rgb[i * 3 + 2] == 200);
    }
}

TEST_CASE("generateTexture height-aware land gradient", "[worldgen]") {
    // 3x1 grid: low land (5m), mid land (25m), high land (100m)
    std::vector<std::vector<float>> grid = {{5.0f, 25.0f, 100.0f}};
    std::vector<UrbanArea> noUrban;
    HeightmapBounds bounds{-1, 1, 50, 51};
    auto rgb = WorldGenerator::generateTexture(grid, noUrban, bounds);

    REQUIRE(rgb.size() == 3 * 3);
    // Low land: dark green (60, 120, 40)
    REQUIRE(rgb[0] == 60);
    REQUIRE(rgb[1] == 120);
    REQUIRE(rgb[2] == 40);
    // Mid land: medium green (80, 140, 60)
    REQUIRE(rgb[3] == 80);
    REQUIRE(rgb[4] == 140);
    REQUIRE(rgb[5] == 60);
    // High land: brown/grey (140, 130, 100)
    REQUIRE(rgb[6] == 140);
    REQUIRE(rgb[7] == 130);
    REQUIRE(rgb[8] == 100);
}

TEST_CASE("generateTexture height-aware intertidal is sand", "[worldgen]") {
    std::vector<std::vector<float>> grid(2, std::vector<float>(2, 0.0f));
    std::vector<UrbanArea> noUrban;
    HeightmapBounds bounds{-1, 1, 50, 51};
    auto rgb = WorldGenerator::generateTexture(grid, noUrban, bounds);

    for (int i = 0; i < 4; i++) {
        REQUIRE(rgb[i * 3 + 0] == 194);
        REQUIRE(rgb[i * 3 + 1] == 178);
        REQUIRE(rgb[i * 3 + 2] == 128);
    }
}

TEST_CASE("generateTexture height-aware urban overlay on land", "[worldgen]") {
    // 4x4 grid, all low land (5m)
    std::vector<std::vector<float>> grid(4, std::vector<float>(4, 5.0f));
    HeightmapBounds bounds{0, 1, 0, 1};

    // Urban area covering the whole grid
    UrbanArea ua;
    ua.boundary = {{-0.5, -0.5}, {1.5, -0.5}, {1.5, 1.5}, {-0.5, 1.5}, {-0.5, -0.5}};
    std::vector<UrbanArea> urbanAreas = {ua};

    auto rgb = WorldGenerator::generateTexture(grid, urbanAreas, bounds);

    // All pixels should be urban grey (160, 160, 160)
    for (int i = 0; i < 16; i++) {
        REQUIRE(rgb[i * 3 + 0] == 160);
        REQUIRE(rgb[i * 3 + 1] == 160);
        REQUIRE(rgb[i * 3 + 2] == 160);
    }
}

TEST_CASE("generateTexture height-aware urban does not colour water", "[worldgen]") {
    // 2x2 grid: water (-10m)
    std::vector<std::vector<float>> grid(2, std::vector<float>(2, -10.0f));
    HeightmapBounds bounds{0, 1, 0, 1};

    // Urban area covering everything - but water should stay blue
    UrbanArea ua;
    ua.boundary = {{-0.5, -0.5}, {1.5, -0.5}, {1.5, 1.5}, {-0.5, 1.5}, {-0.5, -0.5}};

    auto rgb = WorldGenerator::generateTexture(grid, {ua}, bounds);

    // Should remain medium water blue (40, 80, 160), not grey
    for (int i = 0; i < 4; i++) {
        REQUIRE(rgb[i * 3 + 0] == 40);
        REQUIRE(rgb[i * 3 + 1] == 80);
        REQUIRE(rgb[i * 3 + 2] == 160);
    }
}

TEST_CASE("generateTexture height-aware empty grid returns empty", "[worldgen]") {
    std::vector<std::vector<float>> grid;
    std::vector<UrbanArea> noUrban;
    HeightmapBounds bounds{0, 1, 0, 1};
    auto rgb = WorldGenerator::generateTexture(grid, noUrban, bounds);
    REQUIRE(rgb.empty());
}

// ── UrbanArea struct ───────────────────────────────────────────────────────

TEST_CASE("UrbanArea struct construction", "[chartreader]") {
    UrbanArea ua;
    ua.boundary = {{-122.5, 48.4}, {-122.4, 48.4}, {-122.4, 48.5}, {-122.5, 48.5}, {-122.5, 48.4}};
    REQUIRE(ua.boundary.size() == 5);
    REQUIRE(ua.boundary[0].longitude == -122.5);
}

#endif // WITH_GDAL
