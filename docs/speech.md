# simcore one-minute speech script

**Start 0:00 · 1:00**

*(Slide 1.)*

simcore is a C++20 library for discrete-time simulation, not an application. It
answers three questions: which entities exist, how time advances, and how to
iterate entities by type. Its public API is four calls: `spawn`, `despawn`,
`view`, and `entity_count`. It knows nothing about particles or sensors. A
consumer adds it with one `add_subdirectory` line. On the verified Windows
build, three commands configure, build, and test it:
`cmake -S . -B build -G "MinGW Makefiles"`,
`cmake --build build --config Release`, and
`ctest --test-dir build -C Release`.

*(Slide 6. Run `simcore_demo`.)*

The demo reports 1.50 simulated seconds in 9 fixed steps, and 14 out of 14 tests
pass.

*(Slide 7. Add `thermometer.h`, then rebuild and run.)*

To prove reuse, I add a thermometer entity now: one value, one update function,
one include, one spawn, one print. Only the demo recompiles; the library source
and build files stay untouched. The thermometer reads 20.75 degrees.
