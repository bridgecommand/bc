#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <cmath>
#include <limits>
#include <string>
#include <vector>

#include "Utilities.hpp"

using Catch::Approx;

// ── trim ────────────────────────────────────────────────────────────────────

TEST_CASE("trim removes leading and trailing whitespace", "[utilities]") {
    REQUIRE(Utilities::trim("  hello  ") == "hello");
    REQUIRE(Utilities::trim("\thello\t") == "hello");
    REQUIRE(Utilities::trim("\nhello\n") == "hello");
    REQUIRE(Utilities::trim("  \t hello \n ") == "hello");
}

TEST_CASE("trim preserves interior whitespace", "[utilities]") {
    REQUIRE(Utilities::trim("  hello world  ") == "hello world");
}

TEST_CASE("trim handles empty string", "[utilities]") {
    REQUIRE(Utilities::trim("") == "");
}

TEST_CASE("trim handles all-whitespace string", "[utilities]") {
    REQUIRE(Utilities::trim("   ") == "");
}

TEST_CASE("trim with custom characters", "[utilities]") {
    REQUIRE(Utilities::trim("\"hello\"", "\"") == "hello");
    REQUIRE(Utilities::trim("xxhelloxx", "x") == "hello");
}

// ── split ───────────────────────────────────────────────────────────────────

TEST_CASE("split by comma", "[utilities]") {
    auto result = Utilities::split("a,b,c", ',');
    REQUIRE(result.size() == 3);
    REQUIRE(result[0] == "a");
    REQUIRE(result[1] == "b");
    REQUIRE(result[2] == "c");
}

TEST_CASE("split trims parts", "[utilities]") {
    auto result = Utilities::split(" a , b , c ", ',');
    REQUIRE(result.size() == 3);
    REQUIRE(result[0] == "a");
    REQUIRE(result[1] == "b");
    REQUIRE(result[2] == "c");
}

TEST_CASE("split with trailing delimiter adds empty string", "[utilities]") {
    auto result = Utilities::split("a,b,", ',');
    REQUIRE(result.size() == 3);
    REQUIRE(result[0] == "a");
    REQUIRE(result[1] == "b");
    REQUIRE(result[2] == "");
}

TEST_CASE("split empty string", "[utilities]") {
    auto result = Utilities::split("", ',');
    // std::getline on empty string produces no tokens
    REQUIRE(result.size() == 0);
}

TEST_CASE("split single element", "[utilities]") {
    auto result = Utilities::split("hello", ',');
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == "hello");
}

// ── to_lower ────────────────────────────────────────────────────────────────

TEST_CASE("to_lower converts uppercase", "[utilities]") {
    std::string s = "HELLO";
    Utilities::to_lower(s);
    REQUIRE(s == "hello");
}

TEST_CASE("to_lower mixed case", "[utilities]") {
    std::string s = "HeLLo WoRLd";
    Utilities::to_lower(s);
    REQUIRE(s == "hello world");
}

TEST_CASE("to_lower already lowercase", "[utilities]") {
    std::string s = "hello";
    Utilities::to_lower(s);
    REQUIRE(s == "hello");
}

TEST_CASE("to_lower empty string", "[utilities]") {
    std::string s = "";
    Utilities::to_lower(s);
    REQUIRE(s == "");
}

// ── round ───────────────────────────────────────────────────────────────────

TEST_CASE("round positive values", "[utilities]") {
    REQUIRE(Utilities::round(1.5f) == 2);
    REQUIRE(Utilities::round(1.4f) == 1);
    REQUIRE(Utilities::round(1.6f) == 2);
    REQUIRE(Utilities::round(0.5f) == 1);
}

TEST_CASE("round negative values", "[utilities]") {
    REQUIRE(Utilities::round(-1.5f) == -2);
    REQUIRE(Utilities::round(-1.4f) == -1);
    REQUIRE(Utilities::round(-1.6f) == -2);
}

TEST_CASE("round zero", "[utilities]") {
    REQUIRE(Utilities::round(0.0f) == 0);
}

// ── hasEnding ───────────────────────────────────────────────────────────────

TEST_CASE("hasEnding matches suffix", "[utilities]") {
    REQUIRE(Utilities::hasEnding("hello.txt", ".txt") == true);
    REQUIRE(Utilities::hasEnding("hello.txt", ".csv") == false);
    REQUIRE(Utilities::hasEnding("a", "a") == true);
}

TEST_CASE("hasEnding with longer ending than string", "[utilities]") {
    REQUIRE(Utilities::hasEnding("hi", "hello") == false);
}

TEST_CASE("hasEnding with empty strings", "[utilities]") {
    REQUIRE(Utilities::hasEnding("hello", "") == true);
    REQUIRE(Utilities::hasEnding("", "") == true);
    REQUIRE(Utilities::hasEnding("", "a") == false);
}

// ── lexical_cast ────────────────────────────────────────────────────────────

TEST_CASE("lexical_cast string to float", "[utilities]") {
    REQUIRE(Utilities::lexical_cast<float>(std::string("3.14")) == Approx(3.14f));
    REQUIRE(Utilities::lexical_cast<float>(std::string("0")) == Approx(0.0f));
    REQUIRE(Utilities::lexical_cast<float>(std::string("-1.5")) == Approx(-1.5f));
}

TEST_CASE("lexical_cast string to int", "[utilities]") {
    REQUIRE(Utilities::lexical_cast<int>(std::string("42")) == 42);
    REQUIRE(Utilities::lexical_cast<int>(std::string("0")) == 0);
    REQUIRE(Utilities::lexical_cast<int>(std::string("-7")) == -7);
}

TEST_CASE("lexical_cast handles inf strings for float", "[utilities]") {
    float posInf = Utilities::lexical_cast<float>(std::string("inf"));
    REQUIRE(std::isinf(posInf));
    REQUIRE(posInf > 0);

    float negInf = Utilities::lexical_cast<float>(std::string("-inf"));
    REQUIRE(std::isinf(negInf));
    REQUIRE(negInf < 0);
}

TEST_CASE("lexical_cast handles inf strings for int", "[utilities]") {
    int posInf = Utilities::lexical_cast<int>(std::string("inf"));
    REQUIRE(posInf == std::numeric_limits<int>::max());

    int negInf = Utilities::lexical_cast<int>(std::string("-inf"));
    REQUIRE(negInf == std::numeric_limits<int>::lowest());
}

TEST_CASE("lexical_cast between numeric types", "[utilities]") {
    REQUIRE(Utilities::lexical_cast<std::string>(42) == "42");
    REQUIRE(Utilities::lexical_cast<int>(3.14) == 3);
}

// ── dmyToTimestamp ──────────────────────────────────────────────────────────

TEST_CASE("dmyToTimestamp returns valid timestamp", "[utilities]") {
    // 1 Jan 2000 00:00:00 UTC
    time_t t = Utilities::dmyToTimestamp(1, 1, 2000);
    REQUIRE(t > 0);

    // Verify round-trip through gmtime
    struct tm* tm_result = gmtime(&t);
    REQUIRE(tm_result->tm_mday == 1);
    REQUIRE(tm_result->tm_mon == 0); // January = 0
    REQUIRE(tm_result->tm_year == 100); // 2000 - 1900
}

TEST_CASE("dmyToTimestamp ordering is correct", "[utilities]") {
    time_t jan1 = Utilities::dmyToTimestamp(1, 1, 2000);
    time_t jan2 = Utilities::dmyToTimestamp(2, 1, 2000);
    time_t feb1 = Utilities::dmyToTimestamp(1, 2, 2000);
    REQUIRE(jan2 > jan1);
    REQUIRE(feb1 > jan2);
}
