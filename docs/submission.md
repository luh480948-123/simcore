# simcore submission

This document maps deliverables D1-D8 in
[requirements-en.html](requirements-en.html) and the five rubric items to the
current repository state, with the evidence used to check each claim.

## Snapshot

| Item | Value |
| --- | --- |
| Repository | <https://github.com/luh480948-123/simcore> |
| Branch | `main` |
| `origin/main` at verification | `9d1733f` (`Merge pull request #5 from luh480948-123/codex/batch-registry-operations`) |
| Verification date | 2026-11-24 |
| Platform | Windows, PowerShell 5 |
| Toolchain | CMake 3.30.5, MinGW-w64 GCC 15.2.0 (`C:\msys64\ucrt64`), Doxygen 1.9.8 |

The repository and its API returned HTTP 200 to unauthenticated requests, so it
is public. A fresh shallow clone of `9d1733f` was configured, built, and tested
with the README Windows commands; the results are in D1.

## 1. Deliverables D1-D8

| ID | Deliverable | Location | Status | Evidence |
| --- | --- | --- | --- | --- |
| D1 | Public Git repository | <https://github.com/luh480948-123/simcore> | Done | Anonymous HTTP 200; fresh clone of `9d1733f` builds and passes 36/36 tests |
| D2 | Library source | `include/simcore/`, `src/stepper.cpp` | Done | 7 headers (1,145 lines) plus a 120-line implementation unit |
| D3 | Example program | `examples/particle_demo.cpp` | Done | 121 lines, two entity types; plus priority and batch demos |
| D4 | Unit tests | `tests/` | Done | 36 ctest cases, 36/36 passed |
| D5 | Doxygen + handwritten guide | `docs/Doxyfile`, `docs/guide.md` | **Not met** | `doc` target exits 2: two undocumented `spawn_many` items at `registry.h:136` |
| D6 | README | `README.md` | Done, minor drift | 106 lines: positioning, commands, API table, links, license, layout |
| D7 | Live demonstration with live entity addition | Classroom | Prepared, stale, not yet performed | Scripts exist but quote 8 or 14 tests; the live addition has not been run in class |
| D8 | Demonstration slides, at most 10 pages | `docs/slides.pptx` | Present, content stale | Exactly 10 slides, but slides 9 and 10 claim 8/8 tests |

### D1 - public repository

- URL: <https://github.com/luh480948-123/simcore>
- Unauthenticated `GET` returned 200 for the HTML page and
  `api.github.com/repos/luh480948-123/simcore`.
- A fresh `git clone --depth 1` of `origin/main` (`9d1733f`) was configured,
  built, and tested with the README Windows block:

| Step | Time | Result |
| --- | --- | --- |
| Configure | 12.48 s | `Configuring done`, `Generating done` |
| Build | 38.81 s | exit 0, 0 warning/error lines |
| ctest | 4.06 s | `100% tests passed, 0 tests failed out of 36` |

The clone had no installed GoogleTest, so configure fetched version 1.15.2 and
the build compiled it from `_deps`; the 12.48 s includes that fetch on this
network. An earlier run with a colder download took about 62 s.

### D2 - library source

| File | Lines |
| --- | --- |
| `include/simcore/simcore.h` | 16 |
| `include/simcore/entity.h` | 91 |
| `include/simcore/entity_handle.h` | 100 |
| `include/simcore/event_bus.h` | 242 |
| `include/simcore/registry.h` | 423 |
| `include/simcore/view.h` | 175 |
| `include/simcore/stepper.h` | 98 |

That is 1,145 header lines. `src/stepper.cpp` adds 120 lines. The target is the
static library `simcore` with the alias `simcore::simcore`; it is C++20 and
builds with `-Wall -Wextra -Wpedantic` without warnings.

The public surface now covers the original four calls plus batch operations
(`spawn_many`, `despawn_all`, `clear`, `reserve`), priority scheduling
(`spawn_with_priority`), stable entity-handle equality and hashing, and a
type-safe `EventBus`.

### D3 - example programs

- `examples/particle_demo.cpp`: 121 lines, under the 200-line limit. Two entity
  types, `Particle` and `BoundedBox`, in one registry; uses `spawn`, `view`,
  `despawn`, and `Stepper::run`.
- `examples/priority_demo.cpp`: 25 lines, shows lower priority numbers updating
  first.
- `examples/batch_demo.cpp`: 40 lines, shows `spawn_many`, `despawn_all`, and
  `clear`.

Current output of the three built targets:

```text
simcore particle demo
---------------------
after spawning             | elapsed= 0.00 s | entities=6
after run(1.0, 0.3)        | elapsed= 1.00 s | entities=6
removed 2 expired particles during view traversal
after cleanup              | elapsed= 1.00 s | entities=4
after run(0.5, 0.1)        | elapsed= 1.50 s | entities=4
---------------------
simulated time : 1.50 s in 9 fixed steps
particles alive: 3 (removed 2)
inside box now : 3
farthest |x|   : 1.75 (half width 5.0)

Priority order: lower numbers first; ties use registration order.
Sensor (dt=0.10)
Particle A (dt=0.10)
Particle B (dt=0.10)
Render (dt=0.10)

after batch spawn: 1001 entities; first group=7
after despawn_all<Particle>(): removed=1000, entities=1, marker=alive
after clear(): entities=0, marker=stale
```

### D4 - unit tests

`ctest` registers 36 cases: 17 stepper, 8 registry, 4 entity-handle, 6 event-bus,
and 1 compile-failure case. Latest local run:

```text
100% tests passed, 0 tests failed out of 36
Total Test time (real) =   3.87 sec
```

The fresh clone reported the same 36/36 result in 3.99 s. The compile-failure
case still builds `tests/concept_violation.cpp` through
`tests/expect_compile_failure.cmake`, requires the build to fail, and matches the
`simcore::Entity` diagnostic.

T1-T5 are still covered by the original cases; the additional suites cover the
merged features (event bus, priority scheduling, batch operations, handle
identity).

### D5 - documentation: not met

`docs/guide.md` is 292 lines and has been extended with priority scheduling and
batch operations. `docs/Doxyfile` is 50 lines with `WARN_IF_UNDOCUMENTED`,
`WARN_NO_PARAMDOC`, `WARN_IF_DOC_ERROR`, and `WARN_AS_ERROR = YES`.

The `doc` target currently fails:

```text
include/simcore/registry.h:136: error: parameters of member
simcore::Registry::spawn_many are not documented
(warning treated as error, aborting now)
```

With `WARN_AS_ERROR` temporarily disabled, Doxygen reports exactly two warnings,
both for `spawn_many` at `registry.h:136`: its parameters and its return type are
not documented. The target exits 2, so `build/docs/html/index.html` is not
produced. This regression came from the merged batch-operations change; the
previous code generated the pages with zero warnings.

D5 is therefore incomplete by its own acceptance rule, even though the
handwritten guide exists and other class pages are generated before the abort.

### D6 - README

`README.md` is 106 lines and contains:

- one-paragraph positioning;
- an API table covering the original and merged operations;
- three build commands for Linux/macOS and three for Windows with MinGW-w64;
- demo and documentation paths;
- links to `docs/guide.md` and `docs/Doxyfile`;
- requirements, directory layout, and the MIT license.

Minor drift: the directory-layout block still describes `examples/` as
"particle_demo.cpp and the simcore_demo target", omitting the priority and batch
demos, and the documentation list omits several files under `docs/`. The
required content is present; the layout text is behind the code.

Only the Windows/MinGW block was executed here. The Linux/macOS block was not
run in this environment.

### D7 - live demonstration: prepared but stale

Status: **not yet performed**, and the prepared narration is out of date.

- `docs/demo-script.md` (406 lines) still says "Eight tests, all green" in
  Segment 3 (line 288), while its slide outline says "Fourteen ctest cases"
  (line 386).
- `docs/speech.md` (26 lines) says "14 out of 14 tests".
- `docs/demo-steps.md` (468 lines) records timings and outputs from the earlier
  8-test build (for example, ctest 12.26 s).
- The actual suite is 36 tests.

The live entity addition itself is unchanged and was rehearsed: a new
`examples/thermometer.h` with one `update(double dt)` function, plus include,
spawn, and print edits in `examples/particle_demo.cpp`. That touches only
consumer files and does not recompile `simcore`. The classroom run and the
on-stage compilation have not happened.

### D8 - slides: 10 pages, but stale

`docs/slides.pptx` is 58,509 bytes and contains exactly 10 slides, so the page
limit is met.

The content predates the merged features:

- slide 4 lists only `spawn`, `view`, `despawn`, `entity_count`, and the stepper;
  it omits batch operations, priority scheduling, handle hashing, and the event
  bus;
- slide 8 lists `simcore`, `simcore_demo`, `simcore_tests`, `doc` but not the
  priority and batch demos;
- slides 9 and 10 say "8/8 ctest cases pass" / "8/8 tests pass", while the
  current suite has 36 cases.

The deck cannot be presented as accurate evidence in its current form.

## 2. Rubric self-assessment

| Rubric item | Current judgment | Summary |
| --- | --- | --- |
| published | Meets | Public repository; fresh clone builds and passes 36/36 with README commands |
| documented | **Not met** | Doxygen strict target fails on `spawn_many`; guide and README exist |
| functional | Conditional | 36/36 tests and all three demos pass; in-class live addition pending |
| innovative | Conditional | Positioning and consumer-only edit are prepared; no external adopter or live proof yet |
| teamwork | **Not met** | Some PRs and multiple authors exist, but the 6.3 minimums are not met by all members |

### published

Evidence:

- Anonymous HTTP 200 for the repository and API.
- A fresh clone of `9d1733f` configured in 12.48 s, built in 38.81 s with zero
  warnings, and passed 36/36 tests in 4.06 s.
- The three commands used are exactly the README Windows block.

Risk: the reviewer machine needs CMake 3.20+, a C++20 compiler, and network
access if GoogleTest is not installed. There is no CI; all results are local.
The Linux/macOS commands are documented but untested here.

### documented

Evidence against:

- `cmake --build build --target doc` exits 2.
- Two Doxygen warnings at `include/simcore/registry.h:136` for `spawn_many`:
  parameters and return type are undocumented.
- `WARN_AS_ERROR = YES` turns the first into a hard failure, so
  `build/docs/html/index.html` is not generated.

Evidence for:

- `docs/guide.md` is complete and targets another course project.
- `README.md` contains the required build, usage, and license material.

Risk: this is a real regression from the batch-operations merge, not a
measurement artifact. Until the `spawn_many` comments are fixed, the
`documented` rubric item cannot be awarded.

### functional

Evidence:

- 36/36 ctest cases pass.
- `simcore_demo`, `simcore_priority_demo`, and `simcore_batch_demo` all build and
  print their summaries.
- The live addition was rehearsed on the consumer side.

Risk: acceptance explicitly requires adding the entity live in class and
compiling it successfully. That has not happened. The demo narration also needs
to be updated to the 36-test reality before it can be used.

### innovative

Evidence:

- The script argues "library, not application" before showing code.
- The rehearsed live change touched only `examples/thermometer.h` and
  `examples/particle_demo.cpp`.
- `docs/guide.md` is written for another project group and now documents the
  merged priority and batch features.

Risk: no external group has adopted the library, and no third-party reuse was
demonstrated. The score depends on the in-class live edit succeeding and on the
argument being delivered, neither of which is repository evidence.

### teamwork

This item is **not met**, although it is no longer a single-author repository.
The public history contains 5 pull requests and 6 contributor identities:

| Requirement | Actual |
| --- | --- |
| At least 8 commits per member | 36 commits total; counts are 14, 9, 6, 3, 2, 2. Only 2 of 6 identities reach 8. |
| At least 2 PRs per member | 5 PRs: wwpily 2, isabella 1, chenqiren 1, elaine 1, luh 0. Only 1 identity reaches 2. |
| At least 1 review comment from another person on each PR | PR #4's only review is by its own author; PR #2's only comment is by its own author; PRs #1, #3, and #5 have no reviews or comments; 0 inline review comments exist. No PR has an external review. |
| Commit span covering at least 70% of the development cycle | Commits run from 2026-11-17 to 2026-11-24, 7 days. Against the 8-week example that is about 12.5%; the actual cycle length is not recorded. The first 8 commits landed within minutes on 2026-11-17. |
| Different members present sections 7.1, 7.2, and 7.3 | No assignment record. `docs/demo-script.md` still uses placeholders A, B, C. |
| `main` protected, no direct pushes | The history contains merge commits from PRs, but also direct commits on `main`. Branch protection cannot be verified from the public history. |

Real collaboration happened, and this document does not deny it. The rubric,
however, requires the stated minimums, and the review, per-member PR, time-span,
and presenter conditions are not satisfied. Current teamwork judgment: 0/1.

## 3. Reviewer acceptance walkthrough

### Prerequisites

- Git, CMake 3.20 or newer, and MinGW-w64 GCC with `g++` on `PATH`.
- Network access if GoogleTest is not installed; CMake then fetches 1.15.2.
- Optional: Doxygen 1.9 or newer. The `doc` target currently fails for the
  documentation reason in D5, independent of the installation.

### Clean clone and three commands

```bat
git clone https://github.com/luh480948-123/simcore.git
cd simcore
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The three commands after `git clone` are exactly the README Windows block.
Measured on a fresh clone of `9d1733f`:

| Step | Time | Expected tail |
| --- | --- | --- |
| Configure | 12.48 s | `-- Configuring done`, `-- Generating done` |
| Build | 38.81 s | `[100%] Built target ...`, no warnings |
| ctest | 4.06 s | `100% tests passed, 0 tests failed out of 36` |

Budget about one minute; the measured total for these three steps was 55.35 s.

### Run the demos

```bat
.\build\examples\simcore_demo.exe
.\build\examples\simcore_priority_demo.exe
.\build\examples\simcore_batch_demo.exe
```

Expected outputs are the three blocks in D3: 1.50 s in 9 fixed steps and 6 to 4
entities; Sensor, Particle A, Particle B, Render; and 1001 entities falling to 0.

### Read the documentation

- Handwritten guide: `docs/guide.md`.
- README: repository root.
- Generated HTML: `build/docs/html/index.html` after
  `cmake --build build --target doc`. **This currently fails** at
  `registry.h:136`; the index is not produced until the two `spawn_many`
  documentation warnings are fixed.

### Where the live entity is added

- New file: `examples/thermometer.h`, a three-line entity with
  `void update(double dt)`.
- Edits: `examples/particle_demo.cpp` only - the include below `<vector>`, the
  `registry.spawn<Thermometer>()` line below the box spawn, and one `printf`
  below the `farthest |x|` line.
- Paste-ready text: [demo-script.md](demo-script.md), Segment 2, steps 1-4.
  Ignore its test counts in Segment 3 until they are updated.
- Command sequence: [demo-steps.md](demo-steps.md), steps 8-10, but replace its
  old timings and test totals with the current ones.

After the live edit the entity counts become 7 and 5, and the new line is
`thermometer    : 20.75 C`. The committed demo prints 6 and 4 before the edit;
both pairs are correct for their respective states.

## 4. Known gaps and document conflicts

1. **D5 fails.** `spawn_many` needs `@param` and `@return` documentation at
   `include/simcore/registry.h:136`. Until then the strict `doc` target exits 2.
2. **D7 is not complete.** The classroom demonstration has not happened, and the
   narration is stale: the script says 8 tests, its outline says 14, the speech
   says 14/14, and the suite has 36.
3. **D8 is stale.** The 10-page limit is met, but slides 9 and 10 claim 8/8
   tests and slide 4 omits the merged APIs.
4. **teamwork is not met.** Per-member commits, per-member PRs, external review
   records, the 70% time span, and the presenter split are all short of the
   requirement.
5. **The one-minute speech is not the course demo.** `docs/speech.md` is a
   138-word, roughly one-minute artifact; the course asks for 10-15 minutes.
   `docs/demo-script.md` remains the long-form plan and does not replace it.
6. **README drift.** The directory layout omits the priority and batch demos,
   and the documentation list omits several `docs/` files.
7. **The Linux/macOS block is unverified.** Only the Windows MinGW block was
   executed.
8. **No CI.** All test and documentation results come from local runs.

## 5. Evidence commands

```sh
git log --oneline --decorate -40
git shortlog -sne HEAD
ctest --test-dir build -C Release --output-on-failure
cmake --build build --target doc
```

Pull-request, review, and comment counts were read from the public GitHub API
for `luh480948-123/simcore`. Repository visibility was checked with
unauthenticated requests to the HTML page and
`https://api.github.com/repos/luh480948-123/simcore`, both returning 200.
