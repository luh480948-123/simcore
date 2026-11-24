#include <simcore/simcore.h>

#include <gtest/gtest.h>

#include <cstddef>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

/// @brief Entity used to observe scheduling and view behaviour.
struct TestParticle {
    explicit TestParticle(int id) : id(id) {}

    void update(double dt) {
        ++updates;
        last_dt = dt;
    }

    int id;
    int updates{0};
    double last_dt{0.0};
};

/// @brief Second entity type, proving that heterogeneous types coexist.
struct TestBox {
    explicit TestBox(double size) : size(size) {}

    void update(double dt) {
        ++updates;
        last_dt = dt;
    }

    double size;
    int updates{0};
    double last_dt{0.0};
};

}  // namespace

// T2: multiple entity types coexist in one registry, share the same time step,
// and each view yields only entities of the requested type.
TEST(RegistryTest, MultipleEntityTypesCoexist) {
    simcore::Registry registry;
    simcore::Stepper stepper(registry);

    auto particle = registry.spawn<TestParticle>(1);
    auto box = registry.spawn<TestBox>(2.0);
    EXPECT_EQ(registry.entity_count(), 2U);

    stepper.step(0.25);
    EXPECT_EQ(particle->updates, 1);
    EXPECT_EQ(box->updates, 1);
    EXPECT_DOUBLE_EQ(particle->last_dt, 0.25);
    EXPECT_DOUBLE_EQ(box->last_dt, 0.25);

    std::size_t particles = 0;
    for (auto& entity : registry.view<TestParticle>()) {
        ++particles;
        EXPECT_EQ(entity.id, 1);
    }
    std::size_t boxes = 0;
    for (auto& entity : registry.view<TestBox>()) {
        ++boxes;
        EXPECT_DOUBLE_EQ(entity.size, 2.0);
    }
    EXPECT_EQ(particles, 1U);
    EXPECT_EQ(boxes, 1U);
}

// T3: despawning entities inside a view traversal neither crashes nor
// invalidates the traversal, and entity_count() is correct afterwards.
TEST(RegistryTest, DespawnInsideViewKeepsTraversalValid) {
    simcore::Registry registry;
    std::vector<simcore::EntityHandle<TestParticle>> handles;
    for (int id = 0; id < 3; ++id) {
        handles.push_back(registry.spawn<TestParticle>(id));
    }

    std::size_t visited = 0;
    for (auto& particle : registry.view<TestParticle>()) {
        ++visited;
        // The current entity is still alive for the whole iteration.
        particle.last_dt = 42.0;
        for (auto& handle : handles) {
            if (handle.get() == &particle) {
                registry.despawn(handle);
            }
        }
    }

    EXPECT_EQ(visited, 3U);
    EXPECT_EQ(registry.entity_count(), 0U);
    for (auto& handle : handles) {
        EXPECT_FALSE(handle.valid());
    }
}

// T3 (continued): an entity despawned before the iterator reaches it is simply
// skipped, while the remaining entities are still visited normally.
TEST(RegistryTest, DespawnedUnvisitedEntityIsSkipped) {
    simcore::Registry registry;
    auto first = registry.spawn<TestParticle>(1);
    auto second = registry.spawn<TestParticle>(2);

    std::size_t visited = 0;
    for (auto& particle : registry.view<TestParticle>()) {
        (void)particle;
        ++visited;
        registry.despawn(second);
    }

    EXPECT_EQ(visited, 1U);
    EXPECT_EQ(registry.entity_count(), 1U);
    EXPECT_TRUE(first.valid());
    EXPECT_FALSE(second.valid());
}

// T4: defined behaviour for stale handles. Policy: despawn() is idempotent and
// stale handles (destroyed entity or reused slot) are ignored without effect.
TEST(RegistryTest, DespawnIsIdempotentForStaleHandles) {
    simcore::Registry registry;
    auto handle = registry.spawn<TestParticle>(1);

    registry.despawn(handle);
    EXPECT_EQ(registry.entity_count(), 0U);
    EXPECT_FALSE(handle.valid());

    registry.despawn(handle);
    registry.despawn(handle);
    EXPECT_EQ(registry.entity_count(), 0U);

    auto replacement = registry.spawn<TestParticle>(2);
    EXPECT_TRUE(replacement.valid());

    // The stale handle must not remove the entity that recycled its slot.
    registry.despawn(handle);
    EXPECT_EQ(registry.entity_count(), 1U);
    EXPECT_TRUE(replacement.valid());
}

TEST(RegistryTest, SpawnManyCreatesValidHandlesWithSharedArguments) {
    simcore::Registry registry;
    registry.reserve(1'000);

    auto particles = registry.spawn_many<TestParticle>(1'000, 42);

    ASSERT_EQ(particles.size(), 1'000U);
    EXPECT_EQ(registry.entity_count(), 1'000U);
    for (const auto& particle : particles) {
        ASSERT_TRUE(particle.valid());
        EXPECT_EQ(particle->id, 42);
    }
}

TEST(RegistryTest, DespawnAllRemovesOnlyTheRequestedType) {
    simcore::Registry registry;
    auto particles = registry.spawn_many<TestParticle>(3, 7);
    auto box = registry.spawn<TestBox>(2.0);

    EXPECT_EQ(registry.despawn_all<TestParticle>(), 3U);
    EXPECT_EQ(registry.entity_count(), 1U);
    EXPECT_TRUE(box.valid());
    for (const auto& particle : particles) {
        EXPECT_FALSE(particle.valid());
    }

    std::size_t remaining_boxes = 0;
    for (const auto& remaining : registry.view<TestBox>()) {
        (void)remaining;
        ++remaining_boxes;
    }
    EXPECT_EQ(remaining_boxes, 1U);
}

TEST(RegistryTest, ClearInvalidatesEveryHandle) {
    simcore::Registry registry;
    auto particle = registry.spawn<TestParticle>(1);
    auto box = registry.spawn<TestBox>(3.0);

    registry.clear();

    EXPECT_EQ(registry.entity_count(), 0U);
    EXPECT_FALSE(particle.valid());
    EXPECT_FALSE(box.valid());
}

TEST(RegistryTest, DespawnAllDuringViewDefersStorageReleaseSafely) {
    simcore::Registry registry;
    auto particles = registry.spawn_many<TestParticle>(3, 9);

    std::size_t visited = 0;
    std::size_t removed = 0;
    for (auto& particle : registry.view<TestParticle>()) {
        (void)particle;
        ++visited;
        removed = registry.despawn_all<TestParticle>();
    }

    EXPECT_EQ(visited, 1U);
    EXPECT_EQ(removed, 3U);
    EXPECT_EQ(registry.entity_count(), 0U);
    for (const auto& handle : particles) {
        EXPECT_FALSE(handle.valid());
    }
}

TEST(EntityHandleTest, EqualityTracksEntityIdentity) {
    simcore::Registry registry;
    auto handle = registry.spawn<TestParticle>(1);
    const auto copy = handle;
    auto other = registry.spawn<TestParticle>(2);
    const simcore::EntityHandle<TestParticle> empty;
    const simcore::EntityHandle<TestParticle> another_empty;

    EXPECT_EQ(handle, copy);
    EXPECT_NE(handle, other);
    EXPECT_NE(handle, empty);
    EXPECT_EQ(empty, another_empty);
}

TEST(EntityHandleTest, EqualityIncludesOwningRegistry) {
    simcore::Registry first_registry;
    simcore::Registry second_registry;
    auto first = first_registry.spawn<TestParticle>(1);
    auto second = second_registry.spawn<TestParticle>(1);

    ASSERT_EQ(first.id(), second.id());
    EXPECT_NE(first, second);
}

TEST(EntityHandleTest, HashRemainsStableAfterDespawn) {
    using Handle = simcore::EntityHandle<TestParticle>;

    simcore::Registry registry;
    auto handle = registry.spawn<TestParticle>(1);
    const auto copy = handle;
    std::unordered_set<Handle> handles;
    handles.insert(handle);
    handles.insert(copy);
    ASSERT_EQ(handles.size(), 1U);

    const std::size_t original_hash = std::hash<Handle>{}(handle);
    registry.despawn(handle);

    EXPECT_FALSE(handle.valid());
    EXPECT_EQ(std::hash<Handle>{}(handle), original_hash);
    EXPECT_EQ(handles.count(handle), 1U);

    auto replacement = registry.spawn<TestParticle>(2);
    EXPECT_NE(handle, replacement);
    EXPECT_EQ(handles.count(replacement), 0U);
}

TEST(EntityHandleTest, WorksAsUnorderedMapKey) {
    using Handle = simcore::EntityHandle<TestParticle>;

    simcore::Registry registry;
    auto first = registry.spawn<TestParticle>(1);
    auto second = registry.spawn<TestParticle>(2);
    std::unordered_map<Handle, std::string> names;
    names.emplace(first, "first");
    names.emplace(second, "second");

    const auto copy = first;
    names[copy] = "updated";

    EXPECT_EQ(names.size(), 2U);
    EXPECT_EQ(names.at(first), "updated");
    EXPECT_EQ(names.at(second), "second");
}
