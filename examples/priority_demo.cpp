#include <simcore/simcore.h>

#include <cstdio>

struct Task {
    explicit Task(const char* name) : name(name) {}

    void update(double dt) { std::printf("%s (dt=%.2f)\n", name, dt); }

    const char* name;
};

int main() {
    simcore::Registry registry;
    simcore::Stepper stepper(registry);

    // Create the last task first to show that priority controls update order.
    registry.spawn_with_priority<Task>(10, "Render");
    registry.spawn<Task>("Particle A");
    registry.spawn_with_priority<Task>(-10, "Sensor");
    registry.spawn<Task>("Particle B");

    std::puts("Priority order: lower numbers first; ties use registration order.");
    stepper.step(0.1);
}
