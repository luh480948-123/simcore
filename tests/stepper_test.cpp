#include <simcore/simcore.h>

#include <gtest/gtest.h>

#include <stdexcept>

namespace {

constexpr double kEpsilon = 1e-9;

/// @brief Entity that records the time steps it receives.
struct Probe {
    void update(double dt) {
        ++updates;
        last_dt = dt;
        total += dt;
    }

    int updates{0};
    double last_dt{0.0};
    double total{0.0};
};

}  // namespace

// T1: a fixed-step run advances the total simulated time by exactly `duration`
// (within 1e-9) and delivers that same total to the entities.
TEST(StepperTest, FixedStepRunMatchesElapsedTime) {
    simcore::Registry registry;
    simcore::Stepper stepper(registry);
    auto probe = registry.spawn<Probe>();

    stepper.run(1.0, 0.1);

    EXPECT_EQ(probe->updates, 10);
    EXPECT_NEAR(stepper.elapsed(), 1.0, kEpsilon);
    EXPECT_NEAR(probe->total, 1.0, kEpsilon);
}

// T1 (continued): when duration is not a multiple of the fixed step, the final
// frame receives the remainder instead of a full step.
TEST(StepperTest, FinalFrameReceivesRemainder) {
    simcore::Registry registry;
    simcore::Stepper stepper(registry);
    auto probe = registry.spawn<Probe>();

    stepper.run(0.25, 0.1);

    EXPECT_EQ(probe->updates, 3);
    EXPECT_NEAR(stepper.elapsed(), 0.25, kEpsilon);
    EXPECT_NEAR(probe->total, 0.25, kEpsilon);
    EXPECT_NEAR(probe->last_dt, 0.05, 1e-12);
}

// T1 (continued): explicit steps accumulate into elapsed() as well.
TEST(StepperTest, ExplicitStepsAccumulate) {
    simcore::Registry registry;
    simcore::Stepper stepper(registry);
    auto probe = registry.spawn<Probe>();

    stepper.step(0.5);
    stepper.step(0.25);

    EXPECT_EQ(probe->updates, 2);
    EXPECT_NEAR(stepper.elapsed(), 0.75, kEpsilon);
}

// Accumulator: advance() runs floor(acc / fixed_step) steps and preserves the
// remainder for subsequent frames and interpolation_alpha().
TEST(StepperTest, AccumulatorAdvancesFloorStepsAndReportsAlpha) {
    simcore::Registry registry;
    simcore::Stepper stepper(registry, 0.1);
    auto probe = registry.spawn<Probe>();

    // Sub-step frame: 0.04 < 0.1 -> 0 steps, alpha = 0.4.
    EXPECT_EQ(stepper.advance(0.04), 0U);
    EXPECT_EQ(probe->updates, 0);
    EXPECT_NEAR(stepper.elapsed(), 0.0, kEpsilon);
    EXPECT_NEAR(stepper.accumulator(), 0.04, kEpsilon);
    EXPECT_NEAR(stepper.interpolation_alpha(), 0.4, kEpsilon);

    // Accumulated 0.12 -> 1 step (0.1), remainder 0.02, alpha = 0.2.
    EXPECT_EQ(stepper.advance(0.08), 1U);
    EXPECT_EQ(probe->updates, 1);
    EXPECT_NEAR(probe->last_dt, 0.1, kEpsilon);
    EXPECT_NEAR(stepper.elapsed(), 0.1, kEpsilon);
    EXPECT_NEAR(stepper.accumulator(), 0.02, kEpsilon);
    EXPECT_NEAR(stepper.interpolation_alpha(), 0.2, kEpsilon);

    // Accumulated 0.25 -> 2 steps (0.2), remainder 0.05, alpha = 0.5.
    EXPECT_EQ(stepper.advance(0.23), 2U);
    EXPECT_EQ(probe->updates, 3);
    EXPECT_NEAR(stepper.elapsed(), 0.3, kEpsilon);
    EXPECT_NEAR(probe->total, 0.3, kEpsilon);
    EXPECT_NEAR(stepper.accumulator(), 0.05, kEpsilon);
    EXPECT_NEAR(stepper.interpolation_alpha(), 0.5, kEpsilon);
}

// Accumulator: exact multiples like 0.3 with step 0.1 execute all 3 steps
// despite binary floating-point rounding.
TEST(StepperTest, AccumulatorHandlesExactMultiplesWithoutDrift) {
    simcore::Registry registry;
    simcore::Stepper stepper(registry, 0.1);
    auto probe = registry.spawn<Probe>();

    EXPECT_EQ(stepper.advance(0.3), 3U);
    EXPECT_EQ(probe->updates, 3);
    EXPECT_NEAR(stepper.elapsed(), 0.3, kEpsilon);
    EXPECT_NEAR(stepper.accumulator(), 0.0, kEpsilon);
    EXPECT_NEAR(stepper.interpolation_alpha(), 0.0, kEpsilon);
}

// Accumulator: invalid fixed_step, unconfigured advance(), and negative
// frame_dt throw the expected exceptions.
TEST(StepperTest, AccumulatorRejectsInvalidInputAndUnconfiguredStep) {
    simcore::Registry registry;

    EXPECT_THROW((void)simcore::Stepper(registry, 0.0), std::invalid_argument);
    EXPECT_THROW((void)simcore::Stepper(registry, -0.1), std::invalid_argument);

    simcore::Stepper stepper(registry);
    EXPECT_DOUBLE_EQ(stepper.fixed_step(), 0.0);
    EXPECT_DOUBLE_EQ(stepper.interpolation_alpha(), 0.0);
    EXPECT_THROW(stepper.advance(0.1), std::logic_error);

    EXPECT_THROW(stepper.set_fixed_step(0.0), std::invalid_argument);
    stepper.set_fixed_step(0.1);
    EXPECT_THROW(stepper.advance(-0.01), std::invalid_argument);
}
