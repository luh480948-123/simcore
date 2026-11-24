#include <simcore/simcore.h>

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace {

/// @brief Event type carrying an amount of damage.
struct Hit {
    int damage{0};
};

/// @brief Second event type, used to prove that delivery filters by type.
struct Heal {
    int amount{0};
};

/// @brief Entity that publishes a Hit from inside its update pass.
struct Shooter {
    void update(double) {
        if (registry != nullptr) {
            registry->events().publish(Hit{power});
        }
    }

    simcore::Registry* registry{nullptr};
    int power{0};
};

}  // namespace

// Events published during an update pass are held back until the step ends.
TEST(EventBusTest, DeliveryIsDeferredUntilEndOfStep) {
    simcore::Registry registry;
    simcore::Stepper stepper(registry);

    std::vector<int> delivered;
    registry.events().subscribe<Hit>(
        [&delivered](const Hit& hit) { delivered.push_back(hit.damage); });

    auto shooter = registry.spawn<Shooter>();
    shooter->registry = &registry;
    shooter->power = 5;

    stepper.step(0.1);

    ASSERT_EQ(delivered.size(), 1U);
    EXPECT_EQ(delivered.front(), 5);
}

// A listener only receives events of the exact type it subscribed to.
TEST(EventBusTest, FiltersByEventType) {
    simcore::Registry registry;
    std::vector<int> hits;
    std::vector<int> heals;
    registry.events().subscribe<Hit>([&hits](const Hit& hit) { hits.push_back(hit.damage); });
    registry.events().subscribe<Heal>([&heals](const Heal& heal) { heals.push_back(heal.amount); });

    registry.events().publish(Hit{3});
    registry.events().publish(Heal{8});
    registry.events().dispatch();

    EXPECT_EQ(hits, (std::vector<int>{3}));
    EXPECT_EQ(heals, (std::vector<int>{8}));
}

// Listeners for one type run in the order they were registered.
TEST(EventBusTest, ListenersRunInRegistrationOrder) {
    simcore::Registry registry;
    std::vector<std::string> order;
    registry.events().subscribe<Hit>([&order](const Hit&) { order.emplace_back("first"); });
    registry.events().subscribe<Hit>([&order](const Hit&) { order.emplace_back("second"); });

    registry.events().publish(Hit{1});
    registry.events().dispatch();

    EXPECT_EQ(order, (std::vector<std::string>{"first", "second"}));
}

// Detaching a subscription stops delivery without affecting other listeners.
TEST(EventBusTest, UnsubscribeStopsDelivery) {
    simcore::Registry registry;
    int calls = 0;
    auto subscription = registry.events().subscribe<Hit>([&calls](const Hit&) { ++calls; });

    registry.events().publish(Hit{1});
    registry.events().dispatch();
    EXPECT_EQ(calls, 1);

    subscription.unsubscribe();
    EXPECT_FALSE(subscription.active());

    registry.events().publish(Hit{2});
    registry.events().dispatch();
    EXPECT_EQ(calls, 1);
}

// Publishing outside a step only delivers once a step (or dispatch) happens.
TEST(EventBusTest, PublishOutsideStepWaitsForStep) {
    simcore::Registry registry;
    simcore::Stepper stepper(registry);
    int calls = 0;
    registry.events().subscribe<Hit>([&calls](const Hit&) { ++calls; });

    registry.events().publish(Hit{9});
    EXPECT_EQ(calls, 0);

    stepper.step(0.1);
    EXPECT_EQ(calls, 1);
}

// An event published by a listener is queued for the next dispatch, not the
// current one, so a listener can never recurse infinitely.
TEST(EventBusTest, ListenerPublishIsQueuedForNextDispatch) {
    simcore::Registry registry;
    std::vector<int> heals;
    registry.events().subscribe<Heal>([&heals](const Heal& heal) { heals.push_back(heal.amount); });
    registry.events().subscribe<Hit>(
        [&registry](const Hit&) { registry.events().publish(Heal{7}); });

    registry.events().publish(Hit{1});
    registry.events().dispatch();
    EXPECT_TRUE(heals.empty());

    registry.events().dispatch();
    EXPECT_EQ(heals, (std::vector<int>{7}));
}
