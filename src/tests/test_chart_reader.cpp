#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

// Test ChartReader's pure functions (mapping, sequence gen, colour)
// These don't need GDAL since they're pure logic

#ifdef WITH_GDAL
#include "ChartReader.hpp"

using Catch::Approx;

// ── mapBuoyType ─────────────────────────────────────────────────────────────

TEST_CASE("mapBuoyType lateral port buoy", "[chartreader]") {
    ChartBuoy buoy{};
    buoy.layerName = "BOYLAT";
    buoy.categoryLateral = 1;
    buoy.shape = 2; // can
    REQUIRE(ChartReader::mapBuoyType(buoy) == "port_med");
}

TEST_CASE("mapBuoyType lateral port spar", "[chartreader]") {
    ChartBuoy buoy{};
    buoy.layerName = "BOYLAT";
    buoy.categoryLateral = 1;
    buoy.shape = 5; // spar
    REQUIRE(ChartReader::mapBuoyType(buoy) == "port_post");
}

TEST_CASE("mapBuoyType lateral starboard buoy", "[chartreader]") {
    ChartBuoy buoy{};
    buoy.layerName = "BOYLAT";
    buoy.categoryLateral = 2;
    buoy.shape = 1; // conical
    REQUIRE(ChartReader::mapBuoyType(buoy) == "stbd_med");
}

TEST_CASE("mapBuoyType lateral starboard spar", "[chartreader]") {
    ChartBuoy buoy{};
    buoy.layerName = "BOYLAT";
    buoy.categoryLateral = 2;
    buoy.shape = 5; // spar
    REQUIRE(ChartReader::mapBuoyType(buoy) == "stbd_post");
}

TEST_CASE("mapBuoyType preferred channel buoys", "[chartreader]") {
    ChartBuoy buoy{};
    buoy.layerName = "BOYLAT";

    buoy.categoryLateral = 3; // pref channel stbd
    REQUIRE(ChartReader::mapBuoyType(buoy) == "pref_stbd_small");

    buoy.categoryLateral = 4; // pref channel port
    REQUIRE(ChartReader::mapBuoyType(buoy) == "pref_port_small");
}

TEST_CASE("mapBuoyType cardinal buoys", "[chartreader]") {
    ChartBuoy buoy{};
    buoy.layerName = "BOYCAR";

    buoy.categoryCardinal = 1;
    REQUIRE(ChartReader::mapBuoyType(buoy) == "north_small");

    buoy.categoryCardinal = 2;
    REQUIRE(ChartReader::mapBuoyType(buoy) == "east_small");

    buoy.categoryCardinal = 3;
    REQUIRE(ChartReader::mapBuoyType(buoy) == "south_small");

    buoy.categoryCardinal = 4;
    REQUIRE(ChartReader::mapBuoyType(buoy) == "west_small");
}

TEST_CASE("mapBuoyType special buoys", "[chartreader]") {
    ChartBuoy buoy{};
    buoy.layerName = "BOYSPP";

    buoy.shape = 2; // can
    REQUIRE(ChartReader::mapBuoyType(buoy) == "special_1");

    buoy.shape = 5; // spar
    REQUIRE(ChartReader::mapBuoyType(buoy) == "special_post");
}

TEST_CASE("mapBuoyType safe water", "[chartreader]") {
    ChartBuoy buoy{};
    buoy.layerName = "BOYSAW";
    REQUIRE(ChartReader::mapBuoyType(buoy) == "safe");
}

TEST_CASE("mapBuoyType isolated danger", "[chartreader]") {
    ChartBuoy buoy{};
    buoy.layerName = "BOYISD";
    REQUIRE(ChartReader::mapBuoyType(buoy) == "black");
}

// ── colourToRGB ─────────────────────────────────────────────────────────────

TEST_CASE("colourToRGB standard colours", "[chartreader]") {
    int r, g, b;

    ChartReader::colourToRGB(1, r, g, b); // White
    REQUIRE(r == 255); REQUIRE(g == 255); REQUIRE(b == 255);

    ChartReader::colourToRGB(3, r, g, b); // Red
    REQUIRE(r == 255); REQUIRE(g == 0); REQUIRE(b == 0);

    ChartReader::colourToRGB(4, r, g, b); // Green
    REQUIRE(r == 0); REQUIRE(g == 255); REQUIRE(b == 0);

    ChartReader::colourToRGB(6, r, g, b); // Yellow
    REQUIRE(r == 255); REQUIRE(g == 255); REQUIRE(b == 0);
}

TEST_CASE("colourToRGB unknown defaults to white", "[chartreader]") {
    int r, g, b;
    ChartReader::colourToRGB(99, r, g, b);
    REQUIRE(r == 255); REQUIRE(g == 255); REQUIRE(b == 255);
}

// ── lightCharacteristicToSequence ───────────────────────────────────────────

TEST_CASE("light sequence Fixed", "[chartreader]") {
    std::string seq = ChartReader::lightCharacteristicToSequence(1, 4.0, "");
    REQUIRE(seq.length() == 16); // 4s * 4 slots/s
    REQUIRE(seq.find('D') == std::string::npos); // All light
}

TEST_CASE("light sequence Flashing", "[chartreader]") {
    std::string seq = ChartReader::lightCharacteristicToSequence(2, 4.0, "");
    REQUIRE(seq.length() == 16);
    // First 4 chars should be L (1 second flash)
    REQUIRE(seq.substr(0, 4) == "LLLL");
    // Rest should be D
    REQUIRE(seq.substr(4) == "DDDDDDDDDDDD");
}

TEST_CASE("light sequence Group Flash Fl(2)", "[chartreader]") {
    std::string seq = ChartReader::lightCharacteristicToSequence(2, 5.0, "(2)");
    REQUIRE(seq.length() == 20); // 5s * 4 slots/s
    // Should start with two flashes separated by gap
    REQUIRE(seq.substr(0, 2) == "LL"); // first flash
    REQUIRE(seq.substr(2, 2) == "DD"); // gap
    REQUIRE(seq.substr(4, 2) == "LL"); // second flash
    // Rest dark
    for (size_t i = 6; i < seq.length(); i++) {
        REQUIRE(seq[i] == 'D');
    }
}

TEST_CASE("light sequence Isophase", "[chartreader]") {
    std::string seq = ChartReader::lightCharacteristicToSequence(7, 6.0, "");
    REQUIRE(seq.length() == 24); // 6s * 4 slots/s
    // First half light, second half dark
    for (int i = 0; i < 12; i++) REQUIRE(seq[i] == 'L');
    for (int i = 12; i < 24; i++) REQUIRE(seq[i] == 'D');
}

TEST_CASE("light sequence Quick", "[chartreader]") {
    std::string seq = ChartReader::lightCharacteristicToSequence(4, 4.0, "");
    REQUIRE(seq.length() == 16);
    // Pattern: L at every 4th position
    for (size_t i = 0; i < seq.length(); i++) {
        if (i % 4 == 0) REQUIRE(seq[i] == 'L');
        else REQUIRE(seq[i] == 'D');
    }
}

TEST_CASE("light sequence Occulting", "[chartreader]") {
    std::string seq = ChartReader::lightCharacteristicToSequence(8, 4.0, "");
    REQUIRE(seq.length() == 16);
    // Mostly light, last 1 second dark
    for (int i = 0; i < 12; i++) REQUIRE(seq[i] == 'L');
    for (int i = 12; i < 16; i++) REQUIRE(seq[i] == 'D');
}

// ── generateBuoyIni ─────────────────────────────────────────────────────────

TEST_CASE("generateBuoyIni formats correctly", "[chartreader]") {
    std::vector<ChartBuoy> buoys;

    ChartBuoy b1{};
    b1.longitude = -122.5516920;
    b1.latitude = 48.3618069;
    b1.layerName = "BOYLAT";
    b1.categoryLateral = 1;
    b1.shape = 2;
    buoys.push_back(b1);

    ChartBuoy b2{};
    b2.longitude = -122.5551163;
    b2.latitude = 48.3620939;
    b2.layerName = "BOYLAT";
    b2.categoryLateral = 2;
    b2.shape = 5; // spar -> grounded
    buoys.push_back(b2);

    std::string ini = ChartReader::generateBuoyIni(buoys);
    REQUIRE(ini.find("Number=2") != std::string::npos);
    REQUIRE(ini.find("Type(1)=port_med") != std::string::npos);
    REQUIRE(ini.find("Type(2)=stbd_post") != std::string::npos);
    REQUIRE(ini.find("Grounded(2)=1") != std::string::npos);
    // Buoy 1 (can) should NOT have Grounded
    REQUIRE(ini.find("Grounded(1)") == std::string::npos);
}

#endif // WITH_GDAL
