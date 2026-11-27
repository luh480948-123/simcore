// simcore one-minute demo.
//
// The shortest end-to-end consumer: one file, four core calls, and one new
// entity type (Thermometer) declared here rather than in the library.
//
// Differences from the other examples:
// - simcore_demo is the full particle/box scenario with deferred cleanup.
// - simcore_priority_demo shows update ordering only.
// - simcore_batch_demo shows spawn_many/despawn_all/clear only.
// This file is the minimal story used by the one-minute talk.

#include <simcore/simcore.h>

#include <cstddef>
#include <cstdio>

namespace {

/// @brief A consumer-defined entity; simcore does not know this type.
struct Thermometer {
    double celsius{20.0};
    void update(double dt) { celsius += 0.5 * dt; }
};

/// @brief A short-lived pulse used to show view traversal and despawn.
struct Pulse {
    double value{1.0};
    double ttl{2.0};
    void update(double dt) {
        value += dt;
        ttl -= dt;
    }
};

}  // namespace

int main() {
    simcore::Registry registry;
    simcore::Stepper stepper(registry);

    std::puts("simcore one-minute demo");
    std::puts("-----------------------");

    // Thermometer is declared in this file, not in include/simcore/.
    auto thermometer = registry.spawn<Thermometer>();
    registry.spawn<Pulse>();
    auto second = registry.spawn<Pulse>();
    registry.spawn<Pulse>();
    std::printf("spawned: %zu entities\n", registry.entity_count());

    // 1.0 s at 0.3 s: three full steps plus a 0.1 s remainder.
    stepper.run(1.0, 0.3);

    std::size_t pulses_seen = 0;
    for (const auto& pulse : registry.view<Pulse>()) {
        (void)pulse;
        ++pulses_seen;
    }
    std::printf("view<Pulse> before despawn: %zu\n", pulses_seen);

    // Despawning inside the traversal is safe; destruction is deferred.
    for (auto& pulse : registry.view<Pulse>()) {
        if (&pulse == second.get()) {
            registry.despawn(second);
            break;
        }
    }
    std::puts("despawned one pulse during view traversal");

    // 0.5 s at 0.1 s: five more steps, nine in total.
    stepper.run(0.5, 0.1);

    std::size_t alive = 0;
    for (const auto& pulse : registry.view<Pulse>()) {
        (void)pulse;
        ++alive;
    }

    std::printf("elapsed: %.2f s in 9 fixed steps (4 + 5)\n", stepper.elapsed());
    std::printf("view<Pulse> alive: %zu\n", alive);
    std::printf("view<Thermometer> celsius: %.2f\n", thermometer->celsius);
    std::puts("Thermometer is declared in this file, not in include/simcore/");

    return 0;
}
