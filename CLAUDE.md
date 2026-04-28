# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Run

```bash
# Full clean build
./clean_build.sh

# Or manually
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .

# Run
./build/bin/drum-machine
```

Debug build (default) uses `-g -O0`. Release uses `-O2`. Both enable `-Wall -Wextra -Wpedantic`.

## Tests

Tests are built and run via CMake:

```bash
cd build
cmake --build .
ctest --verbose -A "2"   # argument passed to test binary = simulation time in seconds

# Or run the test binary directly
./build/bin/test_rectangular_membrane 2
```

The test binary links `RectangularMembrane.cc` and `audioEngine.cc` directly (no main app sources). Test output is written to `test/output.wav` (controlled by `WAVE_FILE` in the test source).

## Architecture

The application is a real-time physics-based drum synthesizer with three decoupled components:

**Physics backend** (`src/backend/physics/head/`): Solves the 2D wave equation using explicit finite differences. `CircularMembrane.cc` is the current solver (polar coordinates). `RectangularMembrane.cc` is the legacy Cartesian solver, still used in unit tests. Both maintain three time-step grids (`prev_`, `curr_`, `next_`) and advance via swap. Stability is enforced via the CFL condition (set to 0.2 in `include/simDefs.hpp`).

**Audio engine** (`src/backend/audio/audioEngine.cc`): PortAudio wrapper using a ring buffer (producer-consumer) to decouple the main loop from the audio callback. The main thread pushes 2048-sample chunks; the callback pulls samples. Physics runs at its own timestep rate; `sampleInterp()` resamples to 24 kHz PCM before pushing to the ring buffer.

**Renderer** (`src/frontend/drumRenderer.cc`): OpenGL 3.3 Core + GLFW. Renders the membrane as a wireframe mesh. Arrow keys control rotation/tilt; mouse clicks trigger a strike at the click position.

**Main loop** (`src/main.cc`): Runs at 60 FPS (vsync). Each frame: advance physics → resample audio → push to audio engine → render. Mouse click events call the membrane's strike function with a Gaussian displacement.

The `Surface.hpp` base class is currently empty — it exists as a placeholder for future membrane shape abstraction (rectangular, circular, elliptical).

## Key Configuration Headers

- `include/simDefs.hpp` — physics parameters (CFL, grid dimensions, tension, density, radius)
- `include/audioDefs.hpp` — audio parameters (24 kHz sample rate, 2048-sample buffer, 16-bit mono)
- `include/strikeDefs.hpp` — strike parameters (amplitude, Gaussian width, damping)

## Dependencies

OpenGL (GLAD loader in `dependencies/`), GLFW 3.3+, PortAudio 2.0+, GLM, OpenMP. CI installs these via apt on Linux and vcpkg on Windows. See `.github/workflows/cmake-multi-platform.yml` for the full dependency list.
