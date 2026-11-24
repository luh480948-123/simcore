// Demonstrates batch creation and removal without requiring an application-specific loop.

#include <simcore/simcore.h>

#include <cstdio>

namespace {

struct Particle {
    explicit Particle(int group) : group(group) {}

    void update(double) {}

    int group;
};

struct Marker {
    void update(double) {}
};

}  // namespace

int main() {
    simcore::Registry registry;
    registry.reserve(1'000);

    auto particles = registry.spawn_many<Particle>(1'000, 7);
    auto marker = registry.spawn<Marker>();
    std::printf("after batch spawn: %zu entities; first group=%d\n", registry.entity_count(),
                particles.front()->group);

    const std::size_t removed = registry.despawn_all<Particle>();
    std::printf("after despawn_all<Particle>(): removed=%zu, entities=%zu, marker=%s\n", removed,
                registry.entity_count(), marker.valid() ? "alive" : "stale");

    registry.clear();
    std::printf("after clear(): entities=%zu, marker=%s\n", registry.entity_count(),
                marker.valid() ? "alive" : "stale");
    return 0;
}
