#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <cmath>
#include <limits>

#include "Angles.hpp"

using Catch::Approx;

// ── normaliseAngle ──────────────────────────────────────────────────────────

TEST_CASE("normaliseAngle puts angle in [0,360)", "[angles]") {
    REQUIRE(Angles::normaliseAngle(0.0f) == Approx(0.0f));
    REQUIRE(Angles::normaliseAngle(180.0f) == Approx(180.0f));
    REQUIRE(Angles::normaliseAngle(359.9f) == Approx(359.9f));
}

TEST_CASE("normaliseAngle wraps negative angles", "[angles]") {
    REQUIRE(Angles::normaliseAngle(-1.0f) == Approx(359.0f));
    REQUIRE(Angles::normaliseAngle(-90.0f) == Approx(270.0f));
    REQUIRE(Angles::normaliseAngle(-360.0f) == Approx(0.0f));
    REQUIRE(Angles::normaliseAngle(-720.0f) == Approx(0.0f));
}

TEST_CASE("normaliseAngle wraps large positive angles", "[angles]") {
    REQUIRE(Angles::normaliseAngle(360.0f) == Approx(0.0f));
    REQUIRE(Angles::normaliseAngle(361.0f) == Approx(1.0f));
    REQUIRE(Angles::normaliseAngle(720.0f) == Approx(0.0f));
    REQUIRE(Angles::normaliseAngle(450.0f) == Approx(90.0f));
}

TEST_CASE("normaliseAngle returns NaN/Inf unchanged", "[angles]") {
    float nan = std::numeric_limits<float>::quiet_NaN();
    float inf = std::numeric_limits<float>::infinity();
    REQUIRE(std::isnan(Angles::normaliseAngle(nan)));
    REQUIRE(std::isinf(Angles::normaliseAngle(inf)));
    REQUIRE(std::isinf(Angles::normaliseAngle(-inf)));
}

// ── isAngleBetween (scalar) ────────────────────────────────────────────────

TEST_CASE("isAngleBetween simple range", "[angles]") {
    REQUIRE(Angles::isAngleBetween(45.0f, 0.0f, 90.0f) == true);
    REQUIRE(Angles::isAngleBetween(0.0f, 0.0f, 90.0f) == true);
    REQUIRE(Angles::isAngleBetween(90.0f, 0.0f, 90.0f) == true);
    REQUIRE(Angles::isAngleBetween(91.0f, 0.0f, 90.0f) == false);
    REQUIRE(Angles::isAngleBetween(359.0f, 0.0f, 90.0f) == false);
}

TEST_CASE("isAngleBetween wrapping range (crossing 360/0)", "[angles]") {
    // Range from 350 to 10 degrees (wraps through north)
    REQUIRE(Angles::isAngleBetween(355.0f, 350.0f, 370.0f) == true);
    REQUIRE(Angles::isAngleBetween(5.0f, 350.0f, 370.0f) == true);
    REQUIRE(Angles::isAngleBetween(345.0f, 350.0f, 370.0f) == false);
    REQUIRE(Angles::isAngleBetween(15.0f, 350.0f, 370.0f) == false);
}

TEST_CASE("isAngleBetween returns false for NaN input", "[angles]") {
    float nan = std::numeric_limits<float>::quiet_NaN();
    REQUIRE(Angles::isAngleBetween(nan, 0.0f, 90.0f) == false);
    REQUIRE(Angles::isAngleBetween(45.0f, nan, 90.0f) == false);
    REQUIRE(Angles::isAngleBetween(45.0f, 0.0f, nan) == false);
}

// ── sign ────────────────────────────────────────────────────────────────────

TEST_CASE("sign returns correct values", "[angles]") {
    REQUIRE(Angles::sign(1.0f) == 1);
    REQUIRE(Angles::sign(100.0f) == 1);
    REQUIRE(Angles::sign(0.001f) == 1);
    REQUIRE(Angles::sign(-1.0f) == -1);
    REQUIRE(Angles::sign(-100.0f) == -1);
    REQUIRE(Angles::sign(-0.001f) == -1);
    REQUIRE(Angles::sign(0.0f) == 0);
}

// ── localisnan / localisinf ─────────────────────────────────────────────────

TEST_CASE("localisnan detects NaN", "[angles]") {
    REQUIRE(Angles::localisnan(std::numeric_limits<double>::quiet_NaN()) != 0);
    REQUIRE(Angles::localisnan(0.0) == 0);
    REQUIRE(Angles::localisnan(1.0) == 0);
    REQUIRE(Angles::localisnan(-1.0) == 0);
    REQUIRE(Angles::localisnan(std::numeric_limits<double>::infinity()) == 0);
}

TEST_CASE("localisinf detects infinity", "[angles]") {
    REQUIRE(Angles::localisinf(std::numeric_limits<double>::infinity()) != 0);
    REQUIRE(Angles::localisinf(-std::numeric_limits<double>::infinity()) != 0);
    REQUIRE(Angles::localisinf(0.0) == 0);
    REQUIRE(Angles::localisinf(1.0) == 0);
    REQUIRE(Angles::localisinf(std::numeric_limits<double>::quiet_NaN()) == 0);
}
