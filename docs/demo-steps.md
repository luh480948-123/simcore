# simcore demo - executable step list

This checklist was run end to end on this machine on 2026-11-20. Every command
and output below is copied from that run, not from earlier notes. Times are
measured wall-clock times.

Environment used:

- Windows PowerShell 5
- CMake 3.30.5
- MinGW-w64 GCC 15.2.0 (`C:\msys64\ucrt64\bin`)
- Doxygen 1.9.8

Preconditions: `cmake`, `g++`, and `doxygen` are on `PATH`. The repository is at
`C:\Users\zhh\Desktop\CS 2310 project`. No build directory is required at the
start.

Precedence if the documents disagree: the README wins for build commands and
paths; `docs/demo-script.md` wins for demo order and spoken narration; this file
is the execution checklist.

## 0. Open the terminal and enter the repository (about 5 seconds)

```powershell
cd "C:\Users\zhh\Desktop\CS 2310 project"
git status --short
```

Expected screen: `git status --short` prints nothing. The working tree is clean
and only the ignored `build/` directory may exist.

Common failure: `cd` fails because the path contains a space and is not quoted.
Use the quotes exactly as above.

## 1. Delete the old build directory (0.01 seconds measured)

```powershell
Remove-Item -Recurse -Force build
```

Expected screen: no output, and `Test-Path build` returns `False`.

Common failure: `Remove-Item` reports `Access is denied` because
`simcore_demo.exe` or `simcore_tests.exe` is still running. Close the program
and rerun. On a machine without PowerShell, use `rmdir /s /q build`.

## 2. Configure from scratch (62.3 seconds measured)

```powershell
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
```

Expected screen (complete output from this run):

```text
-- The CXX compiler identification is GNU 15.2.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: C:/msys64/ucrt64/bin/c++.exe - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- The C compiler identification is GNU 15.2.0
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: C:/msys64/ucrt64/bin/cc.exe - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Configuring done (55.4s)
-- Generating done (6.5s)
-- Build files have been written to: C:/Users/zhh/Desktop/CS 2310 project/build
```

The 55.4 s configure time includes fetching GoogleTest 1.15.2 on the first run.
No warning or error line appeared.

Common failures:

- `CMake Error: Generator ... could not find any instance of Visual Studio`:
  the `-G "MinGW Makefiles"` argument was omitted. Use the exact command above.
- `No CMAKE_CXX_COMPILER could be found`: MinGW is not on `PATH`. Start the
  MinGW shell or add `C:\msys64\ucrt64\bin` to `PATH`.
- Configure appears to hang at `FetchContent`: the first run is downloading
  GoogleTest. Use a network connection, or reuse an already populated
  `build/` directory from a pre-demo build.
- A stale failure survives a retry: delete `build/` again and rerun step 2.

## 3. Build everything (115.64 seconds measured)

```powershell
cmake --build build --config Release
```

Expected screen (complete output from this run):

```text
[  6%] Building CXX object CMakeFiles/simcore.dir/src/stepper.cpp.obj
[ 13%] Linking CXX static library libsimcore.a
[ 13%] Built target simcore
[ 20%] Building CXX object examples/CMakeFiles/simcore_demo.dir/particle_demo.cpp.obj
[ 26%] Linking CXX executable simcore_demo.exe
[ 26%] Built target simcore_demo
[ 33%] Building CXX object _deps/googletest-build/googletest/CMakeFiles/gtest.dir/src/gtest-all.cc.obj
[ 40%] Linking CXX static library ..\..\..\lib\libgtest.a
[ 40%] Built target gtest
[ 46%] Building CXX object _deps/googletest-build/googletest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.obj
[ 53%] Linking CXX static library ..\..\..\lib\libgtest_main.a
[ 53%] Built target gtest_main
[ 60%] Building CXX object tests/CMakeFiles/simcore_tests.dir/registry_test.cpp.obj
[ 66%] Building CXX object tests/CMakeFiles/simcore_tests.dir/stepper_test.cpp.obj
[ 73%] Linking CXX executable simcore_tests.exe
[ 73%] Built target simcore_tests
[ 80%] Building CXX object _deps/googletest-build/googlemock/CMakeFiles/gmock.dir/src/gmock-all.cc.obj
[ 86%] Linking CXX static library ..\..\..\lib\libgmock.a
[ 86%] Built target gmock
[ 93%] Building CXX object _deps/googletest-build/googlemock/CMakeFiles/gmock_main.dir/src/gmock_main.cc.obj
[100%] Linking CXX static library ..\..\..\lib\libgmock_main.a
[100%] Built target gmock_main
```

No warning or error line appeared in the build log.

Common failures:

- `fatal error: ... No such file or directory`: an edit was saved outside the
  repository or a required source file is missing. Run `git status --short` and
  restore the file from `HEAD`.
- The build stops at a test source: the failure is in the test translation unit,
  not in the library. Read the first `error:` line; fix that file and rebuild.
- Nothing recompiles when it should: a stale `build/` cache is in use. Return to
  step 1 for a clean build.

## 4. Run the tests (12.26 seconds measured)

```powershell
ctest --test-dir build -C Release --output-on-failure
```

Expected screen (complete output from this run):

```text
Internal ctest changing into directory: C:/Users/zhh/Desktop/CS 2310 project/build
Test project C:/Users/zhh/Desktop/CS 2310 project/build
    Start 1: StepperTest.FixedStepRunMatchesElapsedTime
1/8 Test #1: StepperTest.FixedStepRunMatchesElapsedTime ..........   Passed    0.09 sec
    Start 2: StepperTest.FinalFrameReceivesRemainder
2/8 Test #2: StepperTest.FinalFrameReceivesRemainder .............   Passed    0.06 sec
    Start 3: StepperTest.ExplicitStepsAccumulate
3/8 Test #3: StepperTest.ExplicitStepsAccumulate .................   Passed    0.07 sec
    Start 4: RegistryTest.MultipleEntityTypesCoexist
4/8 Test #4: RegistryTest.MultipleEntityTypesCoexist .............   Passed    0.05 sec
    Start 5: RegistryTest.DespawnInsideViewKeepsTraversalValid
5/8 Test #5: RegistryTest.DespawnInsideViewKeepsTraversalValid ...   Passed    0.05 sec
    Start 6: RegistryTest.DespawnedUnvisitedEntityIsSkipped
6/8 Test #6: RegistryTest.DespawnedUnvisitedEntityIsSkipped ......   Passed    0.06 sec
    Start 7: RegistryTest.DespawnIsIdempotentForStaleHandles
7/8 Test #7: RegistryTest.DespawnIsIdempotentForStaleHandles .....   Passed    0.05 sec
    Start 8: simcore.concept.rejects_non_entity
8/8 Test #8: simcore.concept.rejects_non_entity ..................   Passed    6.75 sec

100% tests passed, 0 tests failed out of 8

Total Test time (real) =   7.43 sec
```

The T5 compile-failure test is the long one (6.75 s). The wrapper took 12.26 s
including CTest startup and teardown.

Common failures:

- `No tests were found`: the build directory is missing or the configure step
  failed. Rebuild and rerun from the repository root.
- A test fails after an edit: run `ctest --test-dir build -C Release
  --output-on-failure` again and read the assertion. Do not hide the failure by
  deleting the test.
- `Unable to find executable`: `build/` was deleted after configure. Rebuild.

## 5. Run the demo (0.74 seconds measured)

```powershell
.\build\examples\simcore_demo.exe
```

Expected screen (complete output from this run):

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

Common failures:

- `The term ... is not recognized`: the current directory is not the repository
  root. Run `pwd` and return to the root.
- The file does not exist: on a multi-config generator use
  `build\examples\Release\simcore_demo.exe`; the MinGW command above works.
- The output has no thermometer line: the executable is from before the live
  edit. Rebuild `simcore_demo` first.

## 6. Generate the API documentation (4.46 seconds measured)

```powershell
cmake --build build --target doc
```

Expected screen (key lines from this run; Doxygen prints about 100 progress
lines):

```text
[100%] Generating simcore API documentation
Doxygen version used: 1.9.8 (c2fe5c3e4986974eb2a97608b24086683502f07f)
Notice: Output directory 'C:/Users/zhh/Desktop/CS 2310 project/build/docs' does not exist. I have created it for you.
Searching for include files...
...
Generating docs for compound simcore::Registry...
Generating docs for concept simcore::Entity...
...
[100%] Built target doc
```

The `Notice` is normal on the first documentation run; it is not a warning. No
warning or error line appeared. The generated index is:

```text
build\docs\html\index.html
```

Common failures:

- `No rule to make target 'doc'`: the build was configured as a subdirectory
  consumer, or CMake did not create the target. Configure the repository as the
  top-level project.
- `Doxygen was not found at configure time`: install Doxygen 1.9 or newer, put
  it on `PATH`, delete `build/`, and repeat steps 2 and 6.
- `HTML index does not exist`: inspect the first Doxygen error line; a missing
  input path or an invalid `docs/Doxyfile` entry prevents generation.

## 7. Open the documentation page (about 5 seconds)

```powershell
Start-Process '.\build\docs\html\index.html'
```

Expected screen: the browser opens the simcore Doxygen index. Click **Classes**,
**Class List**, **Registry**; the page shows `spawn`, `despawn`, `view`, and
`entity_count`. Click **Concepts**, **Entity** to show the `update(double dt)`
contract.

Common failure: the browser shows a missing-file page because step 6 has not
run or `build/docs/html/index.html` was deleted. Rerun step 6.

## 8. Add a new entity live (typing about 2 minutes; rebuild 11.75 seconds)

### 8.1 Create `examples/thermometer.h`

In the editor, create the file inside `examples/` (not the repository root and
not `build/`) and paste exactly:

```cpp
#pragma once

/// A minimal entity used to prove that a new type needs no library changes.
class Thermometer {
public:
    /// Advance the temperature by 0.5 degrees per simulated second.
    void update(double dt) { celsius += 0.5 * dt; }

    double celsius = 20.0;
};
```

Save the file. The editor tab path must end with `examples/thermometer.h`.

### 8.2 Add the include in `examples/particle_demo.cpp`

Find `#include <vector>` and insert below it:

```cpp
#include "thermometer.h"
```

### 8.3 Spawn the entity

Find the exact line:

```cpp
    auto box = registry.spawn<BoundedBox>(registry, kHalfWidth);
```

Insert directly below it:

```cpp
    auto thermometer = registry.spawn<Thermometer>();
```

### 8.4 Print its state

Find the exact line:

```cpp
    std::printf("farthest |x|   : %.2f (half width %.1f)\n", farthest, kHalfWidth);
```

Insert directly below it:

```cpp
    std::printf("thermometer    : %.2f C\n", thermometer->celsius);
```

### 8.5 Rebuild only the demo (11.75 seconds measured)

```powershell
cmake --build build --target simcore_demo --config Release
```

Expected screen:

```text
[ 50%] Built target simcore
[ 75%] Building CXX object examples/CMakeFiles/simcore_demo.dir/particle_demo.cpp.obj
[100%] Linking CXX executable simcore_demo.exe
[100%] Built target simcore_demo
```

The library target is already up to date; only `particle_demo.cpp` compiles.

### 8.6 Run the edited demo (2.22 seconds measured)

```powershell
.\build\examples\simcore_demo.exe
```

Expected screen (complete output from this run):

```text
simcore particle demo
---------------------
after spawning             | elapsed= 0.00 s | entities=7
after run(1.0, 0.3)        | elapsed= 1.00 s | entities=7
removed 2 expired particles during view traversal
after cleanup              | elapsed= 1.00 s | entities=5
after run(0.5, 0.1)        | elapsed= 1.50 s | entities=5
---------------------
simulated time : 1.50 s in 9 fixed steps
particles alive: 3 (removed 2)
inside box now : 3
farthest |x|   : 1.75 (half width 5.0)
thermometer    : 20.75 C
```

The count rises from 6 to 7 (and from 4 to 5 after cleanup) because the
thermometer is now a registered entity.

Common failures:

- `fatal error: thermometer.h: No such file or directory`: the header was saved
  outside `examples/`. Move it and rebuild.
- The build succeeds but there is no thermometer line: the file was not saved,
  or the old executable was run. Save all and rerun 8.5 before 8.6.
- `static assertion failed: ... does not satisfy the simcore::Entity concept`:
  the `update` signature is wrong. It must be callable as
  `void update(double dt)`.
- The editor's terminal is inside `build/`: relative paths fail. Return to the
  repository root.

## 9. Prove that only consumer files changed (about 1 second)

```powershell
git status --short
git diff --stat
```

Expected output from this run:

```text
 M examples/particle_demo.cpp
?? examples/thermometer.h
warning: in the working copy of 'examples/particle_demo.cpp', LF will be replaced by CRLF the next time Git touches it
 examples/particle_demo.cpp | 4 ++++
 1 file changed, 4 insertions(+)
```

`git status --short` is the command that lists both consumer files.
`git diff --stat` covers tracked files only, so it shows the modified demo file
with four added lines and does **not** list the new untracked header. No file
under `include/simcore/` or `src/` appears. The LF/CRLF line is a normal Git
line-ending notice on Windows, not a build warning.

Common failure: pointing only at `git diff --stat` and claiming it lists both
files. It does not; use `git status --short` for the new file.

## 10. Reset after rehearsal (optional, about 1 second)

```powershell
git checkout -- examples/particle_demo.cpp
Remove-Item examples\thermometer.h
git status --short
```

Expected screen: the final `git status --short` prints nothing.

Common failure: `Remove-Item` reports that the file does not exist because the
header was never saved under `examples/`. The checkout still restores the demo.

## 11. Hand-off and close

Hand the presentation to the next speaker with the exact line from the demo
script:

> C will show how the work is recorded, tested, and documented.

After the final segment, close with:

> That is how work is recorded, verified, and documented.

## Measured time summary

| Step | Action | Measured time |
| --- | --- | ---: |
| 0 | Open terminal and check status | about 5 s |
| 1 | Delete `build/` | 0.01 s |
| 2 | Configure from scratch | 62.3 s |
| 3 | Build everything | 115.64 s |
| 4 | Run ctest | 12.26 s |
| 5 | Run the demo | 0.74 s |
| 6 | Generate documentation | 4.46 s |
| 7 | Open the documentation page | about 5 s |
| 8.5 | Rebuild only `simcore_demo` | 11.75 s |
| 8.6 | Run the edited demo | 2.22 s |
| 9 | Scope proof | about 1 s |
| 10 | Reset | about 1 s |

Clean machine stages 1-7 took about 3 minutes 15 seconds. The live-edit rebuild
and run took another 14 seconds of machine time; allow about 2 minutes for
typing and switching windows during the actual demo.

## Document consistency check

- **README vs this checklist:** the three Windows commands, the demo path
  `build\examples\simcore_demo.exe`, and the documentation path
  `build\docs\html\index.html` are identical. README is authoritative for these
  commands and paths.
- **demo-script vs this checklist:** the order (build, test, run, documentation,
  live entity, scope proof) matches. `demo-script.md` builds only
  `simcore_demo` during the live segment; this checklist also builds everything
  from scratch first, which is a superset, not a conflict.
- **Known inconsistency:** `demo-script.md` shows the pre-edit output with
  `entities=6` and `entities=4`, but after the live edit the same executable
  prints `entities=7` and `entities=5`. Both are correct for their moment; this
  checklist uses the post-edit values in step 8.6 and does not repeat the
  pre-edit counts after the edit.
- **Known inconsistency:** README's directory-layout paragraph lists only
  `guide.md`, `demo-script.md`, and `Doxyfile` under `docs/`; the actual tree
  also contains `speech.md`, `slides.pptx`, and this file. The actual tree wins;
  the README list is incomplete but does not affect commands.
- **Known conflict outside these three files:** `docs/speech.md` is a one-minute
  pitch, while the course requirement calls for a 10-15 minute demonstration.
  For the classroom run, `demo-script.md` and this checklist take precedence;
  the one-minute speech is not a substitute for the full demonstration.
