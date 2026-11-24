# simcore in five minutes

`simcore` is a library that owns simulation entities and advances time; it is
not an application framework, and it does not define what an entity looks like
beyond a single `update(double dt)` member function.

This guide is written for another course project that wants to reuse `simcore`
without reading its source.

## Requirements

- CMake 3.20 or newer.
- A C++20 compiler: GCC 11+, Clang 14+, or MSVC 19.29+.

## Step 1: add simcore to your project

Clone or copy `simcore` next to your project, then add it as a subdirectory and
link the exported target:

```cmake
cmake_minimum_required(VERSION 3.20)
project(my_simulation LANGUAGES CXX)

# Adjust the path to wherever simcore lives.
add_subdirectory(third_party/simcore simcore)

add_executable(my_simulation main.cpp)
target_link_libraries(my_simulation PRIVATE simcore::simcore)
```

`target_link_libraries` with `simcore::simcore` provides the include directory
(`simcore/simcore.h`) and the C++20 requirement; do not add include paths or
`-std=c++20` manually.

Build and run:

```sh
cmake -S . -B build
cmake --build build
./build/my_simulation
```

## Step 2: write your first entities

Create `main.cpp` with two entity types and use all four core APIs:

```cpp
#include <simcore/simcore.h>

#include <cstdio>

// Your entity: any type with "void update(double dt)".
struct Drone {
    Drone(double altitude, double climb_rate)
        : altitude(altitude), climb_rate(climb_rate) {}

    void update(double dt) { altitude += climb_rate * dt; }

    double altitude;
    double climb_rate;
};

// A second, unrelated entity type in the same registry.
struct Clock {
    void update(double dt) { time += dt; }

    double time{0.0};
};

int main() {
    simcore::Registry registry;
    simcore::Stepper stepper(registry);

    auto drone = registry.spawn<Drone>(10.0, 2.0);
    auto clock = registry.spawn<Clock>();

    // 1.0 s is not a multiple of 0.3 s: the steps are 0.3, 0.3, 0.3, 0.1.
    stepper.run(1.0, 0.3);

    std::printf("elapsed=%.2f s altitude=%.2f m clock=%.2f s entities=%zu\n",
                stepper.elapsed(), drone->altitude, clock->time, registry.entity_count());

    // Views yield only entities of the requested type.
    for (auto& d : registry.view<Drone>()) {
        if (d.altitude > 11.0) {
            registry.despawn(drone);  // safe inside a traversal
        }
    }

    std::printf("after despawn: entities=%zu\n", registry.entity_count());
    return 0;
}
```

Expected output:

```
elapsed=1.00 s altitude=12.00 m clock=1.00 s entities=2
after despawn: entities=1
```

## Batch entity operations

Use the batch APIs when a simulation creates or removes many entities at once.
`spawn_many()` gives every created entity the same constructor arguments, while
`reserve()` reduces container reallocations before a known large batch.

```cpp
simcore::Registry registry;

registry.reserve(1'000);
auto particles = registry.spawn_many<Drone>(1'000, 10.0, 2.0);

std::printf("entities=%zu\n", registry.entity_count());  // 1000
std::size_t removed = registry.despawn_all<Drone>();
std::printf("removed=%zu entities=%zu\n", removed, registry.entity_count());
// removed=1000 entities=0
```

`despawn_all<T>()` removes only the exact type `T`; it leaves other entity
types alive. `clear()` removes every live entity. Both functions follow the
same deferred-destruction rule as `despawn(handle)`, so they remain safe when
called during a view traversal or an update pass. Existing EventBus listeners
and queued events are not changed by `clear()`.

Use individual `spawn()` calls when entities need different constructor
arguments or move-only construction data.

What to remember:

- `spawn<T>(args...)` constructs `T` in place and returns `EntityHandle<T>`,
  which supports `->`, `*`, `get()`, and `valid()`.
- `view<T>()` yields mutable references to live `T` entities only.
- `despawn(handle)` may be called during a view traversal or from `update()`;
  the entity disappears immediately and its memory is released after the
  traversal or update pass ends. It is safe to despawn the entity you are
  currently visiting.
- `stepper.run(duration, step)` calls `update(dt)` on every live entity in
  ascending priority order, with registration order breaking ties, using `step`
  for each frame and the remainder for the
  final frame. `stepper.elapsed()` reports the accumulated simulated time.

## Priority scheduling

Use `spawn_with_priority<T>(priority, args...)` when one entity must update
before another. The priority is separate from the arguments passed to the
entity's constructor. Lower numbers run first, and ordinary `spawn()` uses zero.

```cpp
registry.spawn_with_priority<Drone>(10, 10.0, 2.0);
registry.spawn_with_priority<Clock>(-10);
stepper.step(0.1);  // Clock updates before Drone.
```

Entities with equal priorities update in registration order, even when a new
entity reuses an older entity's storage slot. The priority is set at creation.
Views retain their storage traversal order.

The stepper captures live entities and their order at the start of each step.
Entities spawned during an update receive their first update on the next step.
An entity despawned before its scheduled turn is skipped, while destruction is
deferred until active updates and view traversals finish. A failed update ends
the active update pass and leaves elapsed time unchanged for that step.

Scheduling scans S allocated slots and sorts L live entities, taking
O(S + L log L) time and O(L) extra space per step. The runnable
`examples/priority_demo.cpp` shows negative priorities, default priorities, and
registration order for ties. After building, run
`./build/examples/simcore_priority_demo` (Windows MinGW:
`.\build\examples\simcore_priority_demo.exe`).

Expected output:

```text
Priority order: lower numbers first; ties use registration order.
Sensor (dt=0.10)
Particle A (dt=0.10)
Particle B (dt=0.10)
Render (dt=0.10)
```

## Step 3: add another entity type later

1. Create a new header or source file in **your** project, for example
   `src/temperature.h`.
2. Give the type a member function callable as `void update(double dt)`.
3. Call `registry.spawn<Temperature>(...)` from your own code.
4. Nothing in `simcore` changes, and `simcore` is not recompiled.

If the new type needs to interact with others, use `registry.view<Other>()`
inside its `update()`; nested views are supported.

## Step 4 (optional): route typed events

`simcore` can route typed events between entities that never reference each
other. Subscribe to a type, publish events by value, and the stepper delivers
everything queued at the end of each step:

```cpp
struct Hit {
    int damage;
};

simcore::Registry registry;
simcore::Stepper stepper(registry);

registry.events().subscribe<Hit>([](const Hit& hit) {
    std::printf("hit for %d\n", hit.damage);
});

registry.events().publish(Hit{5});  // queued, nothing runs yet
stepper.step(0.1);                  // prints "hit for 5"
```

What to remember:

- `publish<T>(event)` never calls listeners directly; it queues the event.
  Delivery happens on the next `Stepper::step()` (or an explicit
  `registry.events().dispatch()`), in publish order. This mirrors the deferred
  destruction of `despawn()`.
- `subscribe<T>(listener)` registers a listener for exactly type `T` and returns
  a `Subscription`. The listener stays registered until you call
  `subscription.unsubscribe()`; discarding the returned value keeps it alive.
- An event published from inside a listener is delivered on the following
  dispatch, so listeners cannot recurse without bound.

## Common compile errors

### "does not satisfy the simcore::Entity concept"

Full diagnostic (GCC):

```
error: static assertion failed: simcore::Registry::spawn<T>: T does not satisfy
the simcore::Entity concept. An entity type must provide a member function
callable as 'void update(double dt)'.
```

Cause: the type passed to `spawn<T>()` has no `update` member, has the wrong
name, or returns something other than `void`.

Fix: add exactly one member function with a compatible signature:

```cpp
struct MyEntity {
    void update(double dt);  // must exist and return void
};
```

A `const` member function also works, because it is callable on a non-const
entity. A function taking `float` does not work; the step is a `double`.

### "T is not constructible from the arguments forwarded to spawn()"

Cause: `spawn<T>(args...)` tries to construct `T` with the arguments you
passed.

Fix: pass matching constructor arguments, for example
`registry.spawn<MyEntity>(1.0, 2.0)` for
`MyEntity(double, double)`, instead of `registry.spawn<MyEntity>()`.

### "simcore/simcore.h: No such file or directory"

Cause: the consumer target does not link `simcore`.

Fix: add `target_link_libraries(your_target PRIVATE simcore::simcore)`; that
target carries the include directory. Do not hard-code an include path.

### C++20 features are rejected

Cause: the consumer does not use the imported target, so the C++20 requirement
is not propagated.

Fix: link `simcore::simcore`, or set `target_compile_features(your_target
PRIVATE cxx_std_20)` (or `-std=c++20`) on the target that includes
`simcore/simcore.h`.

## Behaviour notes

- **Deferred destruction.** `entity_count()` and `view<T>()` stop seeing an
  entity as soon as `despawn()` is called, but its storage is released when the
  current traversal or update pass ends. This is what makes despawn-during-
  iteration safe.
- **Idempotent despawn.** Calling `despawn()` again with the same handle, or
  with a handle whose slot has been recycled, is a no-op.
- **Type filtering is exact.** `view<Base>()` does not yield derived entities;
  iterate `view<Derived>()` instead.
- **Deferred events.** `publish<T>()` only queues an event; listeners run when
  the step ends (or on an explicit `dispatch()`), in publish order. Events and
  destruction therefore share the same deferred model.
- **Single-threaded.** A `Registry`, its views, a `Stepper`, and the registry's
  event bus must be used from one thread.
