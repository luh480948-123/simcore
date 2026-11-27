# simcore defense Q&A

Snapshot: `origin/main` at `8425738` (2026-11-27). The repository has 36 ctest
cases, the event bus, priority scheduling, the batch APIs, six contributor
identities, and six merged pull requests. PR #6 is docs-only and was merged after
the earlier five-PR snapshot, so this document uses the actual current count of
six. The suite passes 36/36 in 13.58 s with
`ctest --test-dir build -C Release --output-on-failure`.

Evidence paths are relative to the repository root at that commit. Where a
question exposes a gap, the answer states the gap and the fix instead of
smoothing it over.

## 1. Positioning and reuse

### Q1. What is simcore, and why is it a library rather than an application?

simcore is a C++20 library that owns a heterogeneous set of entities, advances
simulated time in fixed steps, and exposes typed iteration. It deliberately
leaves rendering, networking, persistence, threading, and domain rules to the
consumer, and the repository ships small consumers instead of a product
executable. One caveat: the README still says there is no scheduling policy in
the library, but current `main` does contain generic update-priority ordering;
that sentence is stale.

Evidence: `README.md:3-10`; `CMakeLists.txt:9-10`; `examples/CMakeLists.txt:1-28`;
`include/simcore/registry.h:148-164`.

### Q2. How can a new entity type be added without changing or recompiling simcore?

The only contract is the `Entity` concept: a member function callable as
`void update(double dt)`. `Registry::spawn<T>` is a template that constructs an
`EntityHolder<T>` in the consumer's translation unit and stores it behind
`EntityBase`, so no registration table or library edit is needed. The
one-minute demo declares `Thermometer` in `examples/one_minute_demo.cpp` and
spawns it directly.

Evidence: `include/simcore/entity.h:27-29,67-85`;
`include/simcore/registry.h:79-93`; `examples/one_minute_demo.cpp:20-45`.

### Q3. What reuse evidence exists beyond the in-repository demos?

The repository has a guide written explicitly for another course project and a
validated one-minute consumer bundle, but it does not contain evidence that an
independent group actually adopted the library. The honest answer is that reuse
is demonstrated by the consumer-only extension and by the guide, not by a
third-party adopter. The classroom live edit is still a rehearsal, not a
completed live demonstration.

Evidence: `docs/guide.md:7-8,25-30`; `docs/one-minute-demo.md` section 5;
`git log --oneline` contains no external consumer commits.

### Q4. Which concerns are deliberately outside the library scope?

Rendering, networking, persistence, OS or thread scheduling, and domain rules
are outside the library; deterministic update priority, event delivery, and
entity lifetime are inside because they are generic simulation concerns. Course
artifacts such as slides, the translated requirements, and the one-minute bundle
also sit outside the public API and live under `docs/` or `examples/`.

Evidence: `README.md:3-10`; `CMakeLists.txt:9-10,29-35`;
`include/simcore/registry.h:23-34`.

## 2. Design and internals

### Q5. How do type erasure and type safety coexist?

Storage is type-erased: every slot holds `std::shared_ptr<EntityBase>`, and
`EntityHolder<T>` adapts a concrete type to that base. The public API remains
typed: `spawn<T>` returns `EntityHandle<T>`, `view<T>` filters by exact runtime
type, and `EntityHandle<T>::get()` rejects a mismatched `typeid`. This keeps one
heterogeneous container without exposing `void*` to callers.

Evidence: `include/simcore/entity.h:31-35,67-85`;
`include/simcore/registry.h:79-93,287-294`;
`include/simcore/entity_handle.h:43-49`.

### Q6. How are handles kept type-safe, and how are stale handles detected?

A handle stores a `std::weak_ptr<EntityBase>`, the owning registry pointer, and a
packed identifier containing a slot index and a generation counter. `get()`
locks the weak pointer and checks `typeid(T)`; once storage is released, a
despawned or recycled slot resolves to `nullptr` instead of dangling, while an
entity despawned during the active pass stays resolvable until that pass ends by
design. `despawn` also checks the slot's alive flag, generation, and stored
pointer before doing anything.

Evidence: `include/simcore/entity_handle.h:43-49,78-83`;
`include/simcore/registry.h:188-196,302-323`.

### Q7. Why is despawn deferred, and when is storage actually released?

`despawn` marks the slot dead, decrements `entity_count()`, and appends the slot
to `pending_`; it releases storage only when `iteration_depth_` is zero. That
keeps an in-flight update pass or view traversal valid while making the removal
immediately visible to new views and counts. Calling `despawn` again on the same
or a stale handle is a no-op, so the operation is idempotent.

Evidence: `include/simcore/registry.h:166-196,352-387`.

### Q8. Why is View<T> O(N), and what does that complexity not buy?

`View<T>` advances a slot index and skips every slot that is dead or has a
different runtime type, so a full traversal costs O(total slots), not
O(matching entities). There is no per-type index, which keeps the data structure
simple and avoids maintaining secondary maps on spawn and despawn. The iterator
walks by index rather than holding iterators into the slot vector, so a `spawn`
that reallocates storage during traversal does not invalidate it.

Evidence: `include/simcore/registry.h:251-266`;
`include/simcore/view.h:11-15,53-56,118-160`.

### Q9. Is Registry thread-safe, and why is the model single-threaded?

No. The registry explicitly documents that all access must happen on one thread,
and the event bus repeats the same restriction; there are no mutexes or atomics
in either implementation. A single-threaded model keeps update order
deterministic and avoids synchronization overhead in the inner loop. Separate
registries can be owned by separate threads, but one registry must not be shared
concurrently.

Evidence: `include/simcore/registry.h:34`;
`include/simcore/event_bus.h:97`; no `<mutex>` or `<thread>` include in
`include/simcore/`.

### Q10. How does Stepper handle non-multiple durations and floating-point drift?

`run` validates its inputs, executes full `fixed_step` iterations, and passes the
remaining time as a final partial step. It uses a tolerance of
`fixed_step * 1e-9` so an exact multiple does not produce a spurious near-zero
step, and `elapsed()` accumulates the `dt` values that were actually applied.
`step` snapshots the live update order, runs all entities, and then dispatches
queued events.

Evidence: `src/stepper.cpp:17-35,37-74,118`;
`tests/stepper_test.cpp:55-95`.

### Q11. How does priority scheduling determine update order and break ties?

`spawn_with_priority` stores an integer priority on the slot; lower numbers run
first, and equal priorities run in registration order. `Stepper::step` sorts the
snapshot by `(priority, registration_order)`, so recycling an old slot cannot
make a new entity jump ahead of older peers. Views still iterate in storage
order because priority affects updates, not iteration.

Evidence: `include/simcore/registry.h:148-164,343-349`;
`src/stepper.cpp:51-56`; `tests/stepper_test.cpp:157-205`.

### Q12. How does the EventBus avoid reentrancy and iterator invalidation problems?

`publish` only enqueues a type-erased payload; `Stepper::step` calls `dispatch`
after every entity has updated. `dispatch` copies the listener list before
invoking callbacks, so subscribing or unsubscribing during delivery cannot
invalidate the loop, and a nested `dispatch` is ignored. Events published by a
listener are queued for the next dispatch rather than delivered recursively.

Evidence: `include/simcore/event_bus.h:92-97,147-170,209-215`;
`src/stepper.cpp:72`.

## 3. Build and packaging

### Q13. What CMake targets exist, and how does a consumer use them?

The core is the static library `simcore` with the alias `simcore::simcore`; the
examples add `simcore_demo`, `simcore_priority_demo`, `simcore_batch_demo`, and
`simcore_one_minute_demo`. A consumer adds the repository with
`add_subdirectory` and links `simcore::simcore`; the top-level guard keeps the
example and test subdirectories out of that consumer build. The target carries
C++20, disabled compiler extensions, and `-Wall -Wextra -Wpedantic` for
GCC/Clang.

Evidence: `CMakeLists.txt:9-23,29-35`; `examples/CMakeLists.txt:1-28`;
`docs/guide.md:25-30`.

### Q14. Does simcore support find_package, install, or export?

Not today. `CMakeLists.txt` defines the library and an `ALIAS` target but has no
`install()`, `export()`, package config, or version file, so
`find_package(simcore)` will fail; the guide's phrase "exported target" means the
alias used through `add_subdirectory`, not an installed package. The concrete
fix is to add `GNUInstallDirs`, `install(TARGETS simcore EXPORT ...)`,
`install(EXPORT ...)`, a `simcoreConfig.cmake`, and a consumer test using
`find_package`.

Evidence: `CMakeLists.txt:9-10`; `grep -n "install(" CMakeLists.txt` returns no
matches; `docs/guide.md:18,25-30`.

### Q15. What third-party dependencies exist, and can the project build offline?

The library itself has no third-party runtime dependency. Tests use GoogleTest:
CMake first tries `find_package(GTest QUIET)` and otherwise downloads version
1.15.2 with a SHA-256 hash, so a first test build without a local GoogleTest
needs network access. Doxygen is optional and only needed for the `doc` target.
Building the library and examples with tests disabled works offline.

Evidence: `tests/CMakeLists.txt:6-24`; `CMakeLists.txt:27,37-48`;
`README.md:103-106`.

## 4. Testing and documentation

### Q16. What do the 36 cases cover, and how do T1-T5 map onto them?

The suite is 36 cases: 17 stepper, 8 registry, 4 entity-handle, 6 event-bus, and
1 compile-failure test. T1 maps to `StepperTest.FixedStepRunMatchesElapsedTime`,
`FinalFrameReceivesRemainder`, and `ExplicitStepsAccumulate`; T2 to
`MultipleEntityTypesCoexist`; T3 to the two despawn-during-view cases; T4 to
`DespawnIsIdempotentForStaleHandles`; T5 to the compile-failure test. The extra
cases cover the merged features: priority order, accumulator behavior, batch
operations, handle hashing, and event delivery.

Evidence: `tests/stepper_test.cpp:55,69,83`;
`tests/registry_test.cpp:45,75,103,123`;
`tests/CMakeLists.txt:43-52`; `ctest -N` reports `Total Tests: 36`.

### Q17. How is the compile-failure test implemented?

`tests/concept_violation.cpp` defines `NotAnEntity`, which has `tick(double)` but
no `update(double)`, and then calls `registry.spawn<NotAnEntity>()`. The target is
excluded from the normal build; ctest invokes `expect_compile_failure.cmake`,
which requires a non-zero build result and matches the text
"does not satisfy the simcore::Entity concept". This makes T5 an executable
check rather than a comment.

Evidence: `tests/concept_violation.cpp:11-19`;
`tests/CMakeLists.txt:41-52`; `tests/expect_compile_failure.cmake:22-31`.

### Q18. How does Doxygen enforce documentation synchronization?

`docs/Doxyfile` enables `WARN_IF_UNDOCUMENTED`, `WARN_IF_DOC_ERROR`,
`WARN_NO_PARAMDOC`, and `WARN_AS_ERROR`, so a missing public comment fails the
documentation build instead of producing a silent gap. The `doc` target runs
Doxygen against the public headers and writes HTML under `build/docs/html`.
This is the intended mechanism for keeping the API reference aligned with code.

Evidence: `docs/Doxyfile:27-31`; `CMakeLists.txt:37-48`.

### Q19. The doc target currently fails. What exactly is wrong, and what is the fix?

`cmake --build build --target doc` exits 2 at `include/simcore/registry.h:136`:
the non-Entity overload of `spawn_many` has only `@brief` and lacks `@param` for
`count` and `args` plus a `@return`, so `WARN_NO_PARAMDOC` becomes a hard error.
`build/docs/html/index.html` is therefore not produced. The fix is to add those
three Doxygen tags to that overload and rerun the target; no library behavior
changes.

Evidence: exact command output at `include/simcore/registry.h:136-146`;
`docs/Doxyfile:27-31`; `build/docs/html/index.html` is absent.

### Q20. Are the slides, speech, and submission document in sync with current main?

Not all of them. The new `docs/Group10_simcore_Presentation.pptx` says 36/36
tests, but the older `docs/slides.pptx` still says 8/8, `docs/speech.md` says
14/14, and the outline in `docs/demo-script.md` says fourteen. The collaboration
table in `docs/submission.md` still records 5 PRs, 36 commits, and a 7-day span,
while current `main` has 6 PRs, 43 commits, and 10 days. The fix is to update or
delete the stale deck and counts, and to make the submission table read directly
from `git` and the GitHub API at submission time.

Evidence: `docs/speech.md:19`; `docs/demo-script.md:386`;
`docs/submission.md:283-286`; `docs/slides.pptx` slides 9-10;
`docs/Group10_simcore_Presentation.pptx` slides with "36/36".

## 5. Collaboration and scope

### Q21. How was the work split across the six contributors?

Git shortlog shows eight name/email pairs that collapse to six people because
wwpily and chenqiren each use two addresses. Commit counts are 19 (luh480948),
9 (Iamnotokwith), 8 (wwpily), 3 (elaine030201-droid), 2
(isabella271257zhang), and 2 (chenqiren123). Subject lines show the split: core,
build, and documentation; event bus and integration fixes; handle hashing and
slides; batch operations and README; priority scheduling; and the accumulator.
Only three of the six reach the eight-commit minimum, so the split is uneven.

Evidence: `git shortlog -sne HEAD`; `git log --format='%an|%s'`;
`docs/requirements-en.html:305`.

### Q22. What do the PR and review records actually show?

There are six merged PRs: wwpily opened three (#2, #3, #6), and isabella,
chenqiren, and elaine opened one each; luh480948 and Iamnotokwith opened none.
Only wwpily reaches the two-PR minimum, and PR #6 is documentation only. For
reviews, PR #4 has one review by its own author, PR #2 has one issue comment by
its own author, and PRs #1, #3, #5, and #6 have no reviews or comments; there are
zero inline review comments. No PR has a review from another person, so the
review requirement is not met.

Evidence: GitHub API `GET /repos/luh480948-123/simcore/pulls` and the
`reviews`/`comments` endpoints; `docs/requirements-en.html:307`.

### Q23. Does the commit history meet the 70 percent time-span requirement?

No, on the available evidence. Commits run from 2026-11-17 to 2026-11-27, which
is 10 days and about 17.9 percent of the eight-week cycle used as the course
example; the first eight commits are all dated 2026-11-17. The rubric also
prohibits concentrating work in the final week, and the history shows an early
cluster followed by later feature merges. The fix is to record the real project
cycle and distribute commits, not to rewrite dates.

Evidence: `git log --reverse --format='%ad' --date=short`;
`docs/requirements-en.html:299,308`.

### Q24. Some repository work is outside the library scope. Why is it there, and what would you fix before submission?

The library scope is entity ownership, time, typed iteration, event delivery,
priority ordering, and lifetime management; slides, the translated requirements,
rehearsal notes, and the one-minute bundle are course deliverables and therefore
live outside `include/simcore/`. The merged priority, batch, and event-bus
features are generic simulation-core work, not application logic, so they belong
in the library even though the original README wording still excludes
"scheduling policy". Before submission I would fix the `spawn_many` Doxygen
comments, refresh the stale 8/8 and 14/14 counts, update the collaboration table,
and add a real external review step; if `find_package` support is expected, I
would add install and export rules as well.

Evidence: `README.md:3-10`; `CMakeLists.txt:9-10,29-35`;
`include/simcore/registry.h:136-146`; `docs/requirements-en.html:289-308`.
