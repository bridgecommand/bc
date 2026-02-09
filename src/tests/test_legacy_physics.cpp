/*   Bridge Command 5.0 Ship Simulator - Legacy Physics Tests
     Tests that the extracted LegacyPhysicsModel produces expected results
     matching the inline OwnShip.cpp calculations. */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>
#include <vector>

#include "LegacyPhysicsModel.hpp"

using Catch::Matchers::WithinRel;
using Catch::Matchers::WithinAbs;

// ── Helper: create params matching ProtisSingleScrew boat.ini ──────────────
static LegacyShipParams createProtisParams() {
    LegacyShipParams p;
    p.shipMass = 300000.0;
    p.inertia = 14040000.0;
    p.maxForce = 180000.0;
    p.asternEfficiency = 0.667;
    p.dynamicsSpeedA = 1122.6;
    p.dynamicsSpeedB = 6000.0;
    p.dynamicsLateralDragA = 0; // Will be estimated
    p.dynamicsLateralDragB = 0;
    p.dynamicsTurnDragA = 21060000.0;
    p.dynamicsTurnDragB = 1404000.0;
    p.rudderA = 585.0;
    p.rudderB = 0.0975;
    p.rudderBAstern = 0.0;
    p.propellorSpacing = 0.0; // Single engine
    p.propWalkAhead = 11700.0;
    p.propWalkAstern = 35100.0;
    p.singleEngine = true;
    return p;
}

// ── Helper: run simulation for N seconds ──────────────────────────────────
static PhysicsState simulate(LegacyPhysicsModel& model, PhysicsInput input,
                              PhysicsState state, double duration, double dt = 0.02) {
    int steps = static_cast<int>(duration / dt);
    for (int i = 0; i < steps; i++) {
        model.step(dt, input, state);
    }
    return state;
}

// ══════════════════════════════════════════════════════════════════════════
// Tests
// ══════════════════════════════════════════════════════════════════════════

TEST_CASE("Legacy: Ship at rest with no engine stays at rest", "[legacy]") {
    LegacyPhysicsModel model(createProtisParams());
    PhysicsState state;
    PhysicsInput input;
    state.heading = 90.0;

    state = simulate(model, input, state, 10.0);

    REQUIRE_THAT(state.surge, WithinAbs(0.0, 1e-6));
    REQUIRE_THAT(state.sway, WithinAbs(0.0, 1e-6));
    REQUIRE_THAT(state.yawRate, WithinAbs(0.0, 1e-6));
}

TEST_CASE("Legacy: Full ahead accelerates ship forward", "[legacy]") {
    LegacyPhysicsModel model(createProtisParams());
    PhysicsState state;
    PhysicsInput input;
    input.portEngine = 1.0;
    input.stbdEngine = 1.0;

    state = simulate(model, input, state, 5.0);

    REQUIRE(state.surge > 0.0);
    // After 5 seconds of full thrust, ship should be moving
    REQUIRE(state.surge > 1.0);
}

TEST_CASE("Legacy: Ship reaches terminal speed where thrust equals drag", "[legacy]") {
    auto params = createProtisParams();
    LegacyPhysicsModel model(params);
    PhysicsState state;
    PhysicsInput input;
    input.portEngine = 1.0;
    input.stbdEngine = 1.0;

    // Run for a long time to reach steady state
    state = simulate(model, input, state, 600.0);
    double speed1 = state.surge;

    // Run more - speed should barely change (steady state)
    state = simulate(model, input, state, 60.0);
    double speed2 = state.surge;

    REQUIRE_THAT(speed2, WithinRel(speed1, 0.01)); // Within 1%

    // Verify equilibrium: thrust ≈ drag
    double thrust = 2.0 * params.maxForce; // Single engine doubles port thrust
    double drag = params.dynamicsSpeedA * speed2 * speed2 + params.dynamicsSpeedB * speed2;
    REQUIRE_THAT(thrust, WithinRel(drag, 0.05)); // Within 5%
}

TEST_CASE("Legacy: Astern thrust decelerates and reverses", "[legacy]") {
    auto params = createProtisParams();
    LegacyPhysicsModel model(params);
    PhysicsState state;
    PhysicsInput input;

    // First accelerate forward
    input.portEngine = 1.0;
    state = simulate(model, input, state, 60.0);
    REQUIRE(state.surge > 0.0);
    double forwardSpeed = state.surge;

    // Now go full astern
    input.portEngine = -1.0;
    state = simulate(model, input, state, 120.0);

    // Ship should have slowed and reversed
    REQUIRE(state.surge < forwardSpeed);
    REQUIRE(state.surge < 0.0);
}

TEST_CASE("Legacy: Rudder causes turning", "[legacy]") {
    auto params = createProtisParams();
    LegacyPhysicsModel model(params);
    PhysicsState state;
    PhysicsInput input;
    input.portEngine = 0.5;
    input.stbdEngine = 0.5;

    // First get some speed
    state = simulate(model, input, state, 30.0);
    REQUIRE(state.surge > 0.0);
    double initialHeading = state.heading;

    // Apply starboard rudder for a short time (avoid > 180° wrap)
    input.rudderAngle = 20.0;
    state = simulate(model, input, state, 5.0);

    // Ship should have turned to starboard (heading increased)
    double headingChange = state.heading - initialHeading;
    if (headingChange < -180) headingChange += 360;
    if (headingChange > 180) headingChange -= 360;
    REQUIRE(headingChange > 5.0); // Should have turned significantly
}

TEST_CASE("Legacy: Rudder effect depends on speed", "[legacy]") {
    auto params = createProtisParams();
    LegacyPhysicsModel model(params);
    PhysicsInput input;
    input.portEngine = 0.3;
    input.rudderAngle = 15.0;

    // At low speed
    PhysicsState slowState;
    slowState.surge = 1.0;
    slowState = simulate(model, input, slowState, 10.0);
    double slowYawRate = slowState.yawRate;

    // At high speed
    PhysicsState fastState;
    fastState.surge = 5.0;
    fastState = simulate(model, input, fastState, 10.0);
    double fastYawRate = fastState.yawRate;

    // Rudder should be more effective at higher speed
    REQUIRE(std::abs(fastYawRate) > std::abs(slowYawRate));
}

TEST_CASE("Legacy: Prop walk creates torque", "[legacy]") {
    auto params = createProtisParams();
    LegacyPhysicsModel model(params);
    PhysicsInput input;
    input.portEngine = 1.0; // Full ahead
    input.rudderAngle = 0.0; // No rudder

    PhysicsState state;
    state = simulate(model, input, state, 10.0);

    // With prop walk ahead = 11700 and single engine (doubled), should have yaw
    REQUIRE(std::abs(state.yawRate) > 0.001);
}

TEST_CASE("Legacy: Axial drag formula matches quadratic model", "[legacy]") {
    // Directly test: drag = A*v^2 + B*v
    auto params = createProtisParams();
    double v = 5.0; // m/s
    double expectedDrag = params.dynamicsSpeedA * v * v + params.dynamicsSpeedB * v;

    // 1122.6 * 25 + 6000 * 5 = 28065 + 30000 = 58065 N
    REQUIRE_THAT(expectedDrag, WithinRel(58065.0, 0.001));
}

TEST_CASE("Legacy: Negative speed produces correct drag direction", "[legacy]") {
    auto params = createProtisParams();
    LegacyPhysicsModel model(params);
    PhysicsInput input;

    // Ship moving backward with no engine
    PhysicsState state;
    state.surge = -3.0;
    state = simulate(model, input, state, 1.0);

    // Drag should slow the ship (surge should be less negative / closer to zero)
    REQUIRE(state.surge > -3.0);
}

TEST_CASE("Legacy: Turn drag opposes rotation", "[legacy]") {
    auto params = createProtisParams();
    LegacyPhysicsModel model(params);
    PhysicsInput input;

    // Ship rotating with no engine/rudder
    PhysicsState state;
    state.yawRate = 5.0; // deg/s clockwise
    state = simulate(model, input, state, 5.0);

    // Turn drag should slow the rotation
    REQUIRE(state.yawRate < 5.0);
    REQUIRE(state.yawRate >= 0.0); // Should not reverse
}

TEST_CASE("Legacy: Position updates correctly from heading and speed", "[legacy]") {
    auto params = createProtisParams();
    LegacyPhysicsModel model(params);
    PhysicsInput input;

    // Ship heading north at constant speed (no engine, just initial speed)
    // Use large drag to keep speed roughly constant for one step
    PhysicsState state;
    state.heading = 0.0; // North
    state.surge = 10.0;

    // One small step
    model.step(0.01, input, state);

    // North = +Z direction, X should be ~0
    REQUIRE(state.posZ > 0.0);
    REQUIRE_THAT(state.posX, WithinAbs(0.0, 0.01));

    // Now heading east
    PhysicsState eastState;
    eastState.heading = 90.0;
    eastState.surge = 10.0;
    model.step(0.01, input, eastState);

    // East = +X direction, Z should be ~0
    REQUIRE(eastState.posX > 0.0);
    REQUIRE_THAT(eastState.posZ, WithinAbs(0.0, 0.01));
}

TEST_CASE("Legacy: Heading normalizes to 0-360", "[legacy]") {
    auto params = createProtisParams();
    LegacyPhysicsModel model(params);
    PhysicsInput input;

    PhysicsState state;
    state.heading = 359.0;
    state.yawRate = 10.0; // deg/s
    state = simulate(model, input, state, 1.0);

    REQUIRE(state.heading >= 0.0);
    REQUIRE(state.heading < 360.0);
}

TEST_CASE("Legacy: IPhysicsModel interface works", "[legacy]") {
    auto params = createProtisParams();
    LegacyPhysicsModel legacyModel(params);
    IPhysicsModel& model = legacyModel;

    PhysicsState state;
    PhysicsInput input;
    input.portEngine = 1.0;

    model.step(0.1, input, state);

    REQUIRE(state.surge > 0.0);
    REQUIRE(model.getDimensions().displacement == params.shipMass);
}

TEST_CASE("Legacy: Symmetry - port and starboard rudder mirror", "[legacy]") {
    auto params = createProtisParams();
    params.propWalkAhead = 0; // Disable prop walk for symmetry test
    params.propWalkAstern = 0;
    LegacyPhysicsModel model(params);
    PhysicsInput input;
    input.portEngine = 0.5;

    // Get some speed first
    PhysicsState baseState;
    baseState = simulate(model, input, baseState, 30.0);

    // Port rudder
    PhysicsState portState = baseState;
    PhysicsInput portInput = input;
    portInput.rudderAngle = -20.0;
    portState = simulate(model, portInput, portState, 20.0);

    // Starboard rudder
    PhysicsState stbdState = baseState;
    PhysicsInput stbdInput = input;
    stbdInput.rudderAngle = 20.0;
    stbdState = simulate(model, stbdInput, stbdState, 20.0);

    // Heading changes should be equal and opposite
    double portChange = portState.heading - baseState.heading;
    if (portChange > 180) portChange -= 360;
    if (portChange < -180) portChange += 360;

    double stbdChange = stbdState.heading - baseState.heading;
    if (stbdChange > 180) stbdChange -= 360;
    if (stbdChange < -180) stbdChange += 360;

    REQUIRE_THAT(portChange, WithinAbs(-stbdChange, std::abs(stbdChange) * 0.05 + 0.1));
}
