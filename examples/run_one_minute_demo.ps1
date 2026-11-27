# Build and run the one-minute demo from a clean build directory.
#
# Usage from the repository root (CMake and MinGW g++ must be on PATH):
#   powershell -ExecutionPolicy Bypass -File .\examples\run_one_minute_demo.ps1
#
# The build directory is cmake-build-one-minute, which is ignored by git. Tests
# are disabled so the script does not fetch GoogleTest and stays well inside the
# one-minute budget.

$root = Split-Path -Parent $PSScriptRoot
$build = Join-Path $root 'cmake-build-one-minute'
$exe = Join-Path $build 'examples\simcore_one_minute_demo.exe'

Write-Host '== clean =='
if (Test-Path $build) {
    Remove-Item -Recurse -Force $build
}

$watch = [System.Diagnostics.Stopwatch]::StartNew()

Write-Host '== configure =='
cmake -S $root -B $build -G 'MinGW Makefiles' -DCMAKE_BUILD_TYPE=Release -DSIMCORE_BUILD_TESTS=OFF
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

Write-Host '== build =='
cmake --build $build --target simcore_one_minute_demo --config Release
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

Write-Host '== run =='
& $exe
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

$watch.Stop()
Write-Host ('== total: {0:N1} s ==' -f $watch.Elapsed.TotalSeconds)
