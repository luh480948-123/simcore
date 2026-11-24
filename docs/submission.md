# simcore submission

This document maps the deliverables in
[requirements-en.html](requirements-en.html) (D1-D8) and the five rubric items to
what actually exists in this repository, with the evidence used to check each
claim.

## Snapshot

| Item | Value |
| --- | --- |
| Repository | <https://github.com/luh480948-123/simcore> |
| Branch | `main` |
| HEAD at verification | `2f484d9` (`docs: add executable demo step checklist`) |
| Verification date | 2026-11-24 |
| Working tree | Clean before this document was added |
| Platform | Windows, PowerShell 5 |
| Toolchain | CMake 3.30.5, MinGW-w64 GCC 15.2.0 (`C:\msys64\ucrt64`), Doxygen 1.9.8 |

The repository page and the GitHub API both returned HTTP 200 to an
unauthenticated request on 2026-11-24, so the repository is public and readable
without an account.

## 1. Deliverables D1-D8

| ID | Deliverable | Location | Status | Evidence |
| --- | --- | --- | --- | --- |
| D1 | Public Git repository | <https://github.com/luh480948-123/simcore> | Done | Anonymous HTTP 200; clean clone built at `d2d6396`; build inputs unchanged since |
| D2 | Library source | `include/simcore/`, `src/stepper.cpp` | Done | 6 headers (680 lines) plus a 54-line implementation unit; commit `45696a6` |
| D3 | Example program | `examples/particle_demo.cpp` | Done | 121 lines, two entity types, `simcore_demo` target; commit `3da49f0` |
| D4 | Unit tests | `tests/` | Done | 8 registered ctest cases, 8/8 passed; commit `d0bdfa2` |
| D5 | Doxygen + handwritten guide | `docs/Doxyfile`, `docs/guide.md`, `build/docs/html` | Done | `doc` target exits 0 with 0 warnings; guide is 188 lines |
| D6 | README | `README.md` | Done | 69 lines: positioning, build commands, links, requirements, layout, MIT license |
| D7 | Live demonstration with live entity addition | Classroom | Prepared, not yet performed | Script and rehearsal evidence exist; the classroom run has not happened |
| D8 | Demonstration slides, at most 10 pages | `docs/slides.pptx` | Done | Exactly 10 slides in the PPTX package; 58,509 bytes; commit `bb018da` |

### D1 - public repository

- URL: <https://github.com/luh480948-123/simcore>
- Unauthenticated `GET` returned 200 for both the HTML page and
  `api.github.com/repos/luh480948-123/simcore`.
- A clean clone at commit `d2d6396` was configured, built, tested, and run with
  the README Windows commands.
- From `d2d6396` to `2f484d9` only these files changed:
  `docs/demo-script.md`, `docs/demo-steps.md`, `docs/slides.pptx`,
  `docs/speech.md`. `README.md`, `docs/Doxyfile`, `docs/guide.md`,
  `CMakeLists.txt`, `include/`, `src/`, `examples/`, and `tests/` are identical,
  so the clean-clone result still applies to the current source.

### D2 - library source

Public headers and their line counts:

| File | Lines |
| --- | --- |
| `include/simcore/simcore.h` | 15 |
| `include/simcore/entity.h` | 91 |
| `include/simcore/entity_handle.h` | 76 |
| `include/simcore/registry.h` | 273 |
| `include/simcore/view.h` | 175 |
| `include/simcore/stepper.h` | 50 |

`src/stepper.cpp` adds 54 lines of non-template implementation. The target is
`simcore` (static library) with the alias `simcore::simcore`; it is C++20 and
builds with `-Wall -Wextra -Wpedantic` without warnings.

### D3 - example program

- File: `examples/particle_demo.cpp`, 121 lines, under the 200-line limit.
- Two entity types in one registry: `Particle` and `BoundedBox`.
- Uses `spawn`, `view`, `despawn`, and `Stepper::run`.
- Target: `simcore_demo`, built by `examples/CMakeLists.txt`.
- Current committed source output (rebuilt and rerun on 2026-11-24):

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
```

### D4 - unit tests

Eight ctest cases cover T1-T5:

| Test | Requirement |
| --- | --- |
| `StepperTest.FixedStepRunMatchesElapsedTime` | T1 |
| `StepperTest.FinalFrameReceivesRemainder` | T1 |
| `StepperTest.ExplicitStepsAccumulate` | T1 |
| `RegistryTest.MultipleEntityTypesCoexist` | T2 |
| `RegistryTest.DespawnInsideViewKeepsTraversalValid` | T3 |
| `RegistryTest.DespawnedUnvisitedEntityIsSkipped` | T3 |
| `RegistryTest.DespawnIsIdempotentForStaleHandles` | T4 |
| `simcore.concept.rejects_non_entity` | T5 |

Latest run on 2026-11-24:

```text
100% tests passed, 0 tests failed out of 8
Total Test time (real) =   3.53 sec
```

The T5 case builds `tests/concept_violation.cpp` through
`tests/expect_compile_failure.cmake`, requires the build to fail, and matches the
`simcore::Entity` diagnostic.

### D5 - documentation

- `docs/Doxyfile` is 50 lines. It enables `WARN_IF_UNDOCUMENTED`,
  `WARN_NO_PARAMDOC`, `WARN_IF_DOC_ERROR`, and `WARN_AS_ERROR = YES`.
- `cmake --build build --target doc` exited 0 on 2026-11-24 with zero matches for
  `warning` or `error` in its output.
- Generated pages exist at `build/docs/html/index.html` and
  `build/docs/html/classsimcore_1_1_registry.html`. Generated HTML is not
  committed, as the requirements allow.
- `docs/guide.md` is 188 lines and addresses another course project as the
  reader. It contains a complete `add_subdirectory` example, a runnable
  two-entity `main.cpp` with expected output, the steps for adding a new entity
  type, and four common compile errors including the concept failure.

### D6 - README

`README.md` is 69 lines and contains:

- a one-paragraph positioning of the library;
- three build commands for Linux/macOS and three for Windows with MinGW-w64;
- demo and documentation paths;
- links to `docs/guide.md` and `docs/Doxyfile`;
- requirements, directory layout, and the MIT license.

Only the Windows/MinGW block was executed in this environment. The Linux/macOS
block has not been run here.

### D7 - live demonstration

Status: **prepared, not yet performed**. The classroom event is still ahead, so
this deliverable cannot be marked complete from repository evidence alone.

Prepared material:

- `docs/demo-script.md` (406 lines): three segments of about 3, 5, and 2
  minutes, exact narration, the complete `Thermometer` entity, and the three
  edits to `examples/particle_demo.cpp`.
- `docs/demo-steps.md` (468 lines): executable checklist with commands,
  measured output, timings, failure handling, and reset steps.
- Local rehearsal notes, deliberately ignored by git and not part of the
  repository, record three runs. The full live edit was applied, rebuilt, and
  run; the rebuild took 7.87 s in that rehearsal and the edited demo printed
  `thermometer    : 20.75 C`.

The live addition touches only `examples/thermometer.h` (new) and
`examples/particle_demo.cpp` (include, spawn, printf). No file under
`include/simcore/` or `src/` is modified, and `simcore` is not recompiled.

### D8 - demonstration slides

- File: `docs/slides.pptx`, 58,509 bytes.
- The PPTX package contains exactly 10 `ppt/slides/slideN.xml` parts, matching
  the 10-page limit.
- Slide 1 contains the project name, the positioning line, the four API names,
  and `https://github.com/luh480948-123/simcore`.
- Slide 10 contains the wrap-up, the `8/8 tests pass` claim, the same repository
  URL, and `10 / 10`.
- The deck was generated at 16:9 (960 x 540 pt) and inspected page by page
  during preparation.

## 2. Rubric self-assessment

| Rubric item | Current judgment | Summary |
| --- | --- | --- |
| published | Meets | Public repository, README commands, clean-clone build verified |
| documented | Meets | Strict Doxygen target passes, guide and README complete |
| functional | Conditional | Tests and demo verified; the in-class live addition is pending |
| innovative | Conditional | Positioning and reuse boundary are demonstrated; live proof is pending |
| teamwork | Not met (0/1) | Single author, direct pushes to `main`, no branches, PRs, or reviews |

### published

Evidence:

- Repository returns HTTP 200 anonymously.
- README contains three configure/build/test commands for each platform.
- A clean clone at `d2d6396` completed configure, build, and ctest with the
  Windows commands, and all build inputs are unchanged at `2f484d9`.

Risk: the reviewer machine needs CMake 3.20+, a C++20 compiler, and network
access during the first configure because GoogleTest 1.15.2 is downloaded. There
is no CI configuration; all evidence is from local runs. The Linux/macOS
commands are documented but were not executed here.

### documented

Evidence:

- `doc` target exits 0 with `WARN_AS_ERROR = YES` and no warnings.
- Every public header is included in the Doxygen input; the Registry and concept
  pages were generated.
- `docs/guide.md` is written for another project group and includes runnable
  code with expected output.
- `README.md` contains the required positioning, commands, links, license, and
  directory layout.

Risk: only GCC 15.2.0 with Doxygen 1.9.8 was verified. MSVC and Clang were not
tested, and the Doxygen warning policy depends on that generator remaining in
use.

### functional

Evidence:

- ctest reports 8/8 passing.
- `simcore_demo` builds and prints the summary shown in D3.
- The live addition was rehearsed successfully and changed only consumer files.

Risk: the acceptance condition explicitly requires the new entity to be added
live in class and to compile and run. That step has not happened yet. If it
fails on stage, both `functional` and `innovative` may be judged incomplete, as
the requirements state.

### innovative

Evidence:

- The demo script argues the "library, not application" position before showing
  code.
- The rehearsed live change touched only `examples/thermometer.h` and
  `examples/particle_demo.cpp`; `git status --short` showed exactly those two
  paths.
- `docs/guide.md` explicitly targets another course project as its reader.

Risk: no external group has actually adopted the library, and the "third-party
reuse" demonstration was not performed. The innovation claim rests on the
rehearsed consumer-only change and the guide, not on an independent adopter.
The final point also depends on the live demonstration succeeding.

### teamwork

This item is **not met**. The repository history does not satisfy section 6.3 or
the presenter split in section 7:

| Requirement | Actual |
| --- | --- |
| At least 8 commits per member | 12 commits total, all by one author (`luh480948`) |
| At least 2 PRs per member | 0 pull requests |
| At least 1 review comment from another person on each PR | 0 review comments |
| Commit span covering at least 70% of the development cycle | 2026-11-17 to 2026-11-20, about 3 days; roughly 5% of an assumed 8-week cycle |
| Different members present sections 7.1, 7.2, and 7.3 | One author; no multi-member split exists |
| `main` protected, no direct pushes | All commits were pushed directly to `main`; no feature branches were used |

The commit messages do follow the required `<type>: <description>` form, and
there are no empty messages. That does not compensate for the missing branch,
PR, review, time-span, and presenter requirements. This document does not claim
otherwise.

## 3. Reviewer acceptance walkthrough

### Prerequisites

- Git, CMake 3.20 or newer, and MinGW-w64 GCC with `g++` on `PATH`.
- Network access for the first configure, because GoogleTest 1.15.2 is fetched
  when no installed GoogleTest is found.
- Optional: Doxygen 1.9 or newer for the `doc` target.

### Clean clone and three commands

```bat
git clone https://github.com/luh480948-123/simcore.git
cd simcore
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The three commands after `git clone` are exactly the README Windows block. In
the last complete run from a deleted build directory they took:

| Step | Time | Expected tail of output |
| --- | --- | --- |
| Configure | 62.30 s | `-- Configuring done`, `-- Generating done` |
| Build | 115.64 s | `[100%] Built target simcore_demo` |
| ctest | 12.26 s | `100% tests passed, 0 tests failed out of 8` |

A repeat ctest run with the build already present took 3.53 s on 2026-11-24.
Budget about 3 minutes 10 seconds for the first pass on a warm network.

### Run the demo

```bat
.\build\examples\simcore_demo.exe
```

Expected output is the block in D3: 1.50 s in 9 fixed steps, 6 entities falling
to 4, 3 particles alive, 3 inside the box, farthest `|x|` 1.75.

### Generate and read the documentation

```bat
cmake --build build --target doc
```

The generated HTML lands in `build/docs/html`; open
`build/docs/html/index.html`. The handwritten material is
`docs/guide.md`, and the README is at the repository root. The `doc` target took
4.46 s in the last full run and exited 0 with no warnings.

### Where the live entity is added

- New file: `examples/thermometer.h`, a three-line entity with
  `void update(double dt)`.
- Edits: `examples/particle_demo.cpp` only - the include below `<vector>`, the
  `registry.spawn<Thermometer>()` line below the box spawn, and one `printf`
  below the `farthest |x|` line.
- Exact paste-ready text and ordering:
  [demo-script.md](demo-script.md), Segment 2, steps 1-4.
- Executable command sequence and expected output:
  [demo-steps.md](demo-steps.md), steps 8-10.

After the live edit the entity counts become 7 and 5, and the new output line is
`thermometer    : 20.75 C`. The 6 and 4 in the pre-edit output belong to the
committed demo; both numbers are correct for their respective states.

## 4. Known gaps and document conflicts

1. **D7 is not complete.** The classroom demonstration and the live compilation
   have not happened. `functional` and `innovative` stay conditional until they
   do.
2. **teamwork is not met.** One author, direct pushes to `main`, no PRs, no
   reviews, no feature branches, and a three-day commit span. This is stated
   plainly rather than presented as a collaboration history.
3. **The one-minute speech is shorter than the course requirement.**
   `docs/speech.md` is a 138-word, roughly one-minute artifact. The course asks
   for a 10-15 minute demonstration; `docs/demo-script.md` is the 12-13 minute
   plan, and the one-minute file does not replace it.
4. **README documentation list is incomplete.** It links `guide.md` and
   `demo-script.md` but not `speech.md`, `slides.pptx`, `demo-steps.md`, or this
   file. The files exist; the list is behind.
5. **The Linux/macOS command block is unverified.** Only the Windows MinGW block
   has been executed in this environment.
6. **The clean-clone check predates later documentation commits.** It was run at
   `d2d6396`. The source, tests, examples, CMake files, README, guide, and
   Doxyfile are unchanged through `2f484d9`; only demonstration documentation
   was added afterwards.
7. **No CI.** All test and documentation results come from local runs, not from
   a hosted pipeline.

## 5. Evidence commands

The claims above can be reproduced with:

```sh
git log --oneline --decorate -12
git diff --name-only d2d6396..HEAD
git shortlog -sne HEAD
ctest --test-dir build -C Release --output-on-failure
cmake --build build --target doc
```

Repository visibility was checked with unauthenticated requests to
`https://github.com/luh480948-123/simcore` and
`https://api.github.com/repos/luh480948-123/simcore`, both returning 200.
