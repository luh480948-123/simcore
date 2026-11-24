#include <simcore/simcore.h>

#include <gtest/gtest.h>

#include <stdexcept>
#include <functional>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

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

/// @brief Records update order and optionally changes the registry during update.
struct OrderedEntity {
    OrderedEntity(int id, std::vector<int>& log, std::function<void()> action = {})
        : id(id), log(log), action(std::move(action)) {}

    void update(double) {
        log.push_back(id);
        if (action) {
            action();
        }
    }

    int id;
    std::vector<int>& log;
    std::function<void()> action;
};

/// @brief A second concrete type, sharing the same scheduling rules.
struct OtherOrderedEntity : OrderedEntity {
    using OrderedEntity::OrderedEntity;
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
TEST(StepperTest, PrioritiesApplyAcrossEntityTypesAndEveryRunStep) {
    simcore::Registry registry;
    simcore::Stepper stepper(registry);
    std::vector<int> log;

    registry.spawn_with_priority<OrderedEntity>(5, 1, log);
    registry.spawn<OtherOrderedEntity>(2, log);
    registry.spawn_with_priority<OtherOrderedEntity>(-2, 3, log);
    registry.spawn<OrderedEntity>(4, log);
    registry.spawn_with_priority<OrderedEntity>(std::numeric_limits<int>::min(), 5, log);
    registry.spawn_with_priority<OtherOrderedEntity>(std::numeric_limits<int>::max(), 6, log);

    stepper.run(0.25, 0.1);

    EXPECT_EQ(log, (std::vector<int>{5, 3, 2, 4, 1, 6,
                                   5, 3, 2, 4, 1, 6,
                                   5, 3, 2, 4, 1, 6}));
    EXPECT_NEAR(stepper.elapsed(), 0.25, kEpsilon);
}

TEST(StepperTest, EqualPrioritiesKeepRegistrationOrderAfterSlotReuse) {
    simcore::Registry registry;
    simcore::Stepper stepper(registry);
    std::vector<int> log;
    auto first = registry.spawn_with_priority<OrderedEntity>(4, 1, log);
    registry.spawn_with_priority<OrderedEntity>(4, 2, log);
    registry.despawn(first);
    registry.spawn_with_priority<OrderedEntity>(4, 3, log);

    stepper.step(0.1);
    stepper.step(0.1);

    EXPECT_EQ(log, (std::vector<int>{2, 3, 2, 3}));
}

TEST(StepperTest, OrdinarySpawnResetsRecycledSlotToDefaultPriority) {
    simcore::Registry registry;
    simcore::Stepper stepper(registry);
    std::vector<int> log;
    auto first = registry.spawn_with_priority<OrderedEntity>(-10, 1, log);
    registry.spawn<OrderedEntity>(2, log);
    registry.despawn(first);
    registry.spawn<OrderedEntity>(3, log);
    registry.spawn_with_priority<OrderedEntity>(0, 4, log);

    stepper.step(0.1);

    EXPECT_EQ(log, (std::vector<int>{2, 3, 4}));
}

TEST(StepperTest, DespawnBeforeScheduledTurnSkipsEntity) {
    simcore::Registry registry;
    simcore::Stepper stepper(registry);
    std::vector<int> log;
    auto victim = registry.spawn_with_priority<OrderedEntity>(10, 2, log);
    registry.spawn_with_priority<OrderedEntity>(-1, 1, log, [&] {
        registry.despawn(victim);
        EXPECT_TRUE(victim.valid());
    });
    registry.spawn<OrderedEntity>(3, log);

    stepper.step(0.1);

    EXPECT_EQ(log, (std::vector<int>{1, 3}));
    EXPECT_FALSE(victim.valid());
    EXPECT_EQ(registry.entity_count(), 2U);
}

TEST(StepperTest, SpawnDuringUpdateWaitsForNextStepIncludingRecycledSlots) {
    simcore::Registry registry;
    simcore::Stepper stepper(registry);
    std::vector<int> log;
    bool spawned = false;
    registry.spawn<OrderedEntity>(1, log, [&] {
        if (spawned) {
            return;
        }
        spawned = true;
        // The first spawn reuses a free slot; the rest force storage to grow.
        for (int id = 3; id < 99; ++id) {
            registry.spawn_with_priority<OrderedEntity>(-1, id, log);
        }
    });
    auto temporary = registry.spawn<OrderedEntity>(0, log);
    registry.spawn<OrderedEntity>(2, log);
    registry.despawn(temporary);

    stepper.step(0.1);
    EXPECT_EQ(log, (std::vector<int>{1, 2}));

    log.clear();
    stepper.step(0.1);
    std::vector<int> expected;
    for (int id = 3; id < 99; ++id) {
        expected.push_back(id);
    }
    expected.push_back(1);
    expected.push_back(2);
    EXPECT_EQ(log, expected);
}

TEST(StepperTest, SelfDespawnStaysDeferredUntilStepEnds) {
    simcore::Registry registry;
    simcore::Stepper stepper(registry);
    std::vector<int> log;
    simcore::EntityHandle<OrderedEntity> self;
    self = registry.spawn_with_priority<OrderedEntity>(-1, 1, log, [&] {
        registry.despawn(self);
        EXPECT_TRUE(self.valid());
        EXPECT_EQ(self->id, 1);
    });
    registry.spawn<OrderedEntity>(2, log);

    stepper.step(0.1);
    stepper.step(0.1);

    EXPECT_EQ(log, (std::vector<int>{1, 2, 2}));
    EXPECT_FALSE(self.valid());
}

TEST(StepperTest, ThrowingUpdateReleasesDeferredEntitiesAndAllowsAnotherStep) {
    simcore::Registry registry;
    simcore::Stepper stepper(registry);
    std::vector<int> log;
    simcore::EntityHandle<OrderedEntity> self;
    self = registry.spawn_with_priority<OrderedEntity>(-1, 1, log, [&] {
        registry.despawn(self);
        throw std::runtime_error("update failed");
    });
    registry.spawn<OrderedEntity>(2, log);

    EXPECT_THROW(stepper.step(0.1), std::runtime_error);
    EXPECT_FALSE(self.valid());
    EXPECT_DOUBLE_EQ(stepper.elapsed(), 0.0);
    registry.spawn<OrderedEntity>(3, log);
    stepper.step(0.1);

    EXPECT_EQ(log, (std::vector<int>{1, 2, 3}));
    EXPECT_NEAR(stepper.elapsed(), 0.1, kEpsilon);
}

TEST(StepperTest, ViewsStillUseStorageOrder) {
    simcore::Registry registry;
    simcore::Stepper stepper(registry);
    std::vector<int> log;
    registry.spawn_with_priority<OrderedEntity>(10, 1, log);
    registry.spawn_with_priority<OrderedEntity>(-10, 2, log);

    stepper.step(0.1);
    EXPECT_EQ(log, (std::vector<int>{2, 1}));

    std::vector<int> view_order;
    for (auto& entity : registry.view<OrderedEntity>()) {
        view_order.push_back(entity.id);
    }
    EXPECT_EQ(view_order, (std::vector<int>{1, 2}));
}

// T1 (continued): update() runs in registration order, so an entity that
// recycles the slot of a despawned one is still updated after the entities
// that were registered before it.
TEST(StepperTest, UpdateFollowsRegistrationOrder) {
    simcore::Registry registry;
    simcore::Stepper stepper(registry);
    std::vector<int> visited;

    struct Recorder {
        std::vector<int>* log{nullptr};
        int id{0};
        void update(double) { log->push_back(id); }
    };

    registry.spawn<Recorder>(Recorder{&visited, 1});
    auto second = registry.spawn<Recorder>(Recorder{&visited, 2});
    registry.spawn<Recorder>(Recorder{&visited, 3});
    registry.despawn(second);
    registry.spawn<Recorder>(Recorder{&visited, 4});

    stepper.step(0.1);

    EXPECT_EQ(visited, (std::vector<int>{1, 3, 4}));
}

TEST(StepperTest, EqualPrioritiesKeepOrderAfterMiddleDeletion) {
    simcore::Registry registry;
    simcore::Stepper stepper(registry);
    std::vector<int> log;
    registry.spawn_with_priority<OrderedEntity>(5, 1, log);
    auto middle = registry.spawn_with_priority<OrderedEntity>(5, 2, log);
    registry.spawn_with_priority<OrderedEntity>(5, 3, log);
    registry.spawn_with_priority<OrderedEntity>(5, 4, log);
    registry.despawn(middle);

    stepper.step(0.1);

    EXPECT_EQ(log, (std::vector<int>{1, 3, 4}));
}

TEST(StepperTest, PriorityUpdatesDispatchEventsAfterAllEntities) {
    struct UpdateEvent {
        int sender;
    };

    simcore::Registry registry;
    simcore::Stepper stepper(registry);
    std::vector<int> log;
    registry.events().subscribe<UpdateEvent>([&](const UpdateEvent& event) {
        log.push_back(event.sender);
    });
    registry.spawn_with_priority<OrderedEntity>(10, 1, log, [&] {
        registry.events().publish(UpdateEvent{11});
    });
    registry.spawn_with_priority<OrderedEntity>(-10, 2, log, [&] {
        registry.events().publish(UpdateEvent{22});
    });

    stepper.step(0.1);

    EXPECT_EQ(log, (std::vector<int>{2, 1, 22, 11}));
}
