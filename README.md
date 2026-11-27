# simcore

`simcore` is a small, domain-agnostic C++20 library for discrete-time
simulation. It owns a heterogeneous set of entities, advances simulated time in
fixed steps, and lets callers iterate entities by concrete type. It deliberately
knows nothing about what is being simulated: there is no rendering, networking,
threading, serialization, or scheduling policy inside the library. New entity
types are written entirely in the consumer's own files; `simcore` does not need
to be modified or recompiled.

## Public API at a glance

| API | Purpose |
|---|---|
| `Registry::spawn<T>(args...)` | Creates an entity of type `T` and returns an `EntityHandle<T>`. |
| `Registry::view<T>()` | Iterates over all live entities of type `T`. |
| `Registry::despawn(handle)` | Safely removes an entity, including during traversal. |
| `Registry::spawn_many<T>(count, args...)` | Creates several entities of type `T` with identical constructor arguments. |
| `Registry::despawn_all<T>()` | Safely removes every live entity of type `T`. |
| `Registry::clear()` | Safely removes every live entity. |
| `Registry::reserve(capacity)` | Preallocates registry storage for batch creation. |
| `Registry::entity_count()` | Returns the number of currently live entities. |
| `Stepper::step(dt)` | Advances every live entity by one time step. |
| `Stepper::run(duration, fixed_step)` | Advances the simulation using fixed steps and a final remainder step if needed. |
| `Stepper::elapsed()` | Returns the total simulated time advanced so far. |

## Build, test, run

Pick the block for your platform. Both blocks perform the same three steps:
configure, build, test.

Linux/macOS:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Windows with MinGW-w64 (the configuration verified in this repository; CMake
otherwise defaults to NMake Makefiles and cannot find a compiler):

```sh
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The example is built as `build/examples/simcore_demo`
(`build\examples\simcore_demo.exe` on Windows with MinGW). Multi-configuration
generators such as Visual Studio place it under `build/examples/Release/`. API
documentation is generated with `cmake --build build --target doc` into
`build/docs/html`, then opened from `build/docs/html/index.html`.

## Priority scheduling

Use `registry.spawn_with_priority<T>(priority, args...)` to choose an entity's
update priority. Lower numbers execute first, including negative priorities.
Existing `spawn<T>(args...)` calls use priority zero. Equal priorities execute
in registration order, including after deleted slots are reused.

Build the project using the commands above, then run
`./build/examples/simcore_priority_demo` (Windows MinGW:
`.\build\examples\simcore_priority_demo.exe`). The example prints Sensor,
Particle A, Particle B, then Render. The [usage guide](docs/guide.md#priority-scheduling)
explains the API and update-time creation and deletion rules.

## Batch entity operations

Use `registry.reserve(n)` before a known large batch, then create entities with
`registry.spawn_many<T>(count, args...)`. Remove one exact type with
`registry.despawn_all<T>()`, or remove all live entities with `registry.clear()`.
After building, run `./build/examples/simcore_batch_demo` (Windows MinGW:
`.\build\examples\simcore_batch_demo.exe`) to see batch creation and removal.

## One-minute demo

`examples/one_minute_demo.cpp` is the shortest end-to-end consumer: one file
that declares its own `Thermometer` entity, then uses `spawn`, `view`,
`despawn`, and `run`. It does not replace `simcore_demo` (the full particle and
box scenario) or the priority and batch demos (which isolate those merged APIs).

On Windows, from the repository root:

```powershell
powershell -ExecutionPolicy Bypass -File .\examples\run_one_minute_demo.ps1
```

The script deletes `cmake-build-one-minute`, configures with tests disabled,
builds only `simcore_one_minute_demo`, runs it, and prints the elapsed time.

## Documentation

- [Five-minute guide](docs/guide.md) — adding simcore to another project,
  writing a first entity, and fixing common compile errors.
- [Classroom demo script](docs/demo-script.md) — timed narration, live entity
  addition, Q&A preparation, and the slide outline.
- API reference — generated from the public headers by Doxygen; see
  [docs/Doxyfile](docs/Doxyfile). The generated HTML is not committed.

## Requirements

- CMake 3.20 or newer and a C++20 compiler (GCC 11+, Clang 14+, MSVC 19.29+).
- GoogleTest is needed only for the unit tests. CMake prefers an installed
  GoogleTest and otherwise downloads version 1.15.2 at configure time.
- Doxygen 1.9 or newer is needed only for the `doc` target.

## Directory layout

```
simcore/
├── CMakeLists.txt        # library target simcore::simcore, doc target, test wiring
├── include/simcore/      # public headers; simcore.h exposes the whole API
├── src/                  # non-template library implementation
├── examples/             # particle_demo.cpp and the simcore_demo target
├── tests/                # GoogleTest suites and the compile-failure check
└── docs/                 # guide.md, demo-script.md and Doxyfile
```

## License

MIT — see [LICENSE](LICENSE).
