# Drum Machine: Physical Modeling Synthesis

A real-time 2D membrane drum synthesizer using finite difference method (FDM) to solve the 2D wave equation. This project combines physics-based sound synthesis with interactive visualization to create a physically accurate digital drum instrument.

## Overview

This drum machine simulates the vibration of a 2D rectangular membrane (like a drum head) using numerical solutions to the 2D wave equation with damping. The displacement of the membrane is converted into audio in real-time at 48 kHz sample rate, while an interactive 3D visualization shows the membrane's motion.

<p align="center">
   <img src= "https://github.com/user-attachments/assets/7d44209a-7714-4026-81cf-dedf9e0a9ed6">

</p>

### Key Features

- **Physics-Based Synthesis**: Solves the 2D wave equation using finite difference method
- **Real-Time Audio**: 48 kHz sample rate audio output via PortAudio
- **3D Visualization**: Interactive OpenGL rendering with rotation and tilt controls
- **Damping Simulation**: Energy loss modeling for realistic drum decay
- **Modular Architecture**: Separation of physics, rendering, and audio components

## Project Structure

```
drum-machine/
├── src/
│   ├── main.cc                      # Main application loop
│   ├── backend/
│   │   ├── RectangularMembrane.cc   # Physics solver
|   |   ├── CircularMembrane.cc 
│   │   └── audioEngine.cc           # Audio I/O with PortAudio
│   └── frontend/
│       ├── drumRenderer.cc          # OpenGL rendering
│       ├── default.vert             # Vertex shader
│       └── default.frag             # Fragment shader
├── include/
│   ├── RectangularMembrane.hpp
│   ├── audioEngine.hpp
│   ├── drumRenderer.hpp
│   ├── audioDefs.hpp                # Audio configuration
│   ├── simDefs.hpp                  # Simulation parameters
│   └── wav.hpp                      # WAV file format
├── dependencies/                    # Third-party headers (GLAD, KHR)
├── test/
│   ├── rectangularMembraneUnitTest.cc
│   └── Makefile
├── CMakeLists.txt
├── clean_build.sh                   # Build script
└── drum_machine_plan.md             # Development roadmap
```

## Building & Running

### Prerequisites
- CMake ≥3.10
- C++ compiler with C++17 support
- OpenGL 3.3+ drivers installed
- GLFW 3.3+ development files
- PortAudio 2.0+ development files
- GLM headers installed
- OpenMP library available
- pkg-config utility


**macOS**:
```bash
brew install cmake glfw portaudio glm
```

**Linux (Ubuntu/Debian)**:
```bash
sudo apt-get install cmake libglfw3-dev portaudio19-dev libglm-dev
```

**Clone Repo**:
```bash
git clone https://github.com/aae2079/drum-machine.git
```

### Build

```bash
cd drum-machine/
./clean_build.sh
```

Or manually:
```bash
cd build/
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Run

```bash
cd build/bin/
mdkir shaders/
ln -sf ../../src/frontend/default/* . 
./drum-machine
```

## Controls

### Simulation
- **Mouse Click (↖)** — Runs simulation on membrane
- **ESC** — Exit application

### Visualization
- **Arrow Keys (↑↓)** — Tilt membrane up/down
- **Arrow Keys (←→)** — Rotate membrane left/right

## Configuration

Edit `include/simDefs.hpp` to customize:

```cpp
#define CFL 0.25              // Courant stability parameter (< 0.5)
#define BUFFER_SIZE 2048      // Audio buffer chunk size
#define GRID_R 70            // Membrane grid width (affects tone)
#define GRID_TH 100            // Membrane grid height (affects tone)
```

Edit `include/audioDefs.hpp` for audio settings:

```cpp
#define SAMPLE_RATE 24000     // Audio sample rate (Hz)
#define NUM_CHANNELS 1        // Mono output
#define BIT_DEPTH 16          // 16-bit audio
```

## Physics Parameters

Tune the drum sound by editing `include/simDefs.hpp`:

```cpp
#define CFL 0.2          // Courant stability parameter — controls timestep (must satisfy CFL² < 0.5)
#define GRID_R 50        // Radial rings — larger = lower pitch
#define GRID_TH 75       // Angular samples per ring
#define TENSION 150.0f   // N/m — tighter = higher pitch, faster decay
#define MATERIAL_DENSITY (1400.0f * MEMBRANE_THICKNESS)  // kg/m², derived from Mylar properties
#define RADIUS 0.3f      // meters — physical drum head size
```

Strike parameters are set in `CircularMembrane::setInitialCondition()` in `src/backend/CircularMembrane.cc`:

```cpp
float amp = 0.1f;   // Initial displacement amplitude
// Gaussian width controlled by: exp(-0.01 * ir * ir)
```

### How to Tune for Different Drum Sounds

| Parameter | Effect | For Kick | For Tom | For Snare |
|-----------|--------|----------|---------|-----------|
| `GRID_R`, `GRID_TH` | Pitch | Large (100+) | Medium (50) | Small (30) |
| `TENSION` | Pitch / decay | Low (80) | Medium (150) | High (250) |
| `RADIUS` | Pitch | Large (0.4) | Medium (0.3) | Small (0.18) |
| `amp` | Strike strength | High (0.2) | Medium (0.1) | Low (0.05) |

## Architecture

### CircularMembrane (Physics)
- Solves the 2D wave equation in polar coordinates (radial + angular)
- Stores 3 flat grids (`u_prev_`, `u_curr_`, `u_next_`) indexed as `[ir * Ntheta_ + itheta]`
- `init()`: Derives wave speed `c = sqrt(T/ρ)` and timestep `dt = CFL * dr / c`; allocates grids
- `setInitialCondition()`: Applies a Gaussian strike centred at r=0
- `Simulate()`: Runs `physSteps_` FDM timesteps (enough to cover one audio buffer); handles origin singularity by averaging angular neighbours; enforces Dirichlet boundary at outer edge
- `sampleInterp()`: Linear resampling from physics rate → 24 kHz for audio output
- Audio extracted from centre point (r=0) with 15× gain

### AudioEngine (Audio I/O)
- Uses **PortAudio** for cross-platform audio
- Ring buffer with `NUM_FRAMES` slots for producer-consumer pattern
- Callback-based playback (real-time safe)
- `pushChunk()`: Main thread → ring buffer
- `internalAudioCB()`: Audio callback → playback

### DrumRenderer (Visualization)
- **OpenGL 3.3 Core** with GLFW window management
- `updateCircularVertexData()`: Maps polar grid to Cartesian mesh vertices each frame
- Wireframe mode (GL_LINE) shows membrane displacement clearly
- GLM matrices for rotation and tilt transformations

## Technical Highlights

### Real-Time Audio Streaming
- Physics timestep derived from membrane parameters (not fixed); `physSteps_` computed so one `Simulate()` call covers exactly one audio buffer's worth of time
- `sampleInterp()` resamples physics output to 24 kHz before pushing to ring buffer
- Ring buffer prevents audio glitches under CPU load
- ~85 ms latency (BUFFER_SIZE / SAMPLE_RATE)

### Numerical Stability
- **CFL Condition**: `(c·dt/dr)² ≤ CFL²` enforced at init
- Current CFL = 0.2 (safe margin below 0.5 limit)
- Origin singularity (r=0) handled by averaging all angular neighbours
- Boundary conditions: Dirichlet (u=0) at outer edge

### Performance
- OpenMP parallelization of the spatial grid loop in `Simulate()`
- 50×75 polar grid runs at 60 FPS with vsync on modern hardware
- Reduce `GRID_R`/`GRID_TH` in `simDefs.hpp` if performance is insufficient

## Audio Output Quality

- **Sample Rate**: 24 kHz
- **Bit Depth**: 16-bit signed PCM
- **Channels**: Mono
- **Dynamic Range**: ±1.0 (clamped to prevent clipping)

## Future Enhancements (from Development Plan)

### Phase 2: Advanced Features
- [ ] Multiple strike locations (mouse interaction)
- [ ] Parameter GUI
- [ ] Simulate Drum Shell

### Phase 3: Sound Design
- [ ] Preset system (kick, tom, snare, hi-hat)
- [ ] Multiple membranes with mixing
- [ ] Basic effects (reverb, delay, filter)

### Phase 4: Production Ready
- [ ] MIDI input support (RtMidi)
- [ ] WAV file export
- [ ] VST plugin wrapper (JUCE)
- [ ] Cross-platform installers

See `drum_machine_plan.md` for detailed development roadmap.

## Testing

Unit test with audio output (tests the rectangular membrane solver):

```bash
cd build
cmake --build .
ctest --verbose -A "2"

# Or run directly:
./build/bin/test_rectangular_membrane 2
```

Generates `output.wav`. Compile the test with `WAVE_FILE 1` in `test/rectangularMembraneUnitTest.cc` to write the WAV file.

## Troubleshooting

### Build Issues

**PortAudio not found**:
```bash
export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH"
./clean_build.sh
```

**GLFW linking errors**:
```bash
# macOS
brew reinstall glfw
brew link --force libomp #libomp may need relinking 

# Linux
sudo apt-get install --reinstall libglfw3-dev
```

### Runtime Issues

**No audio output**:
- Check system volume and audio device
- Verify PortAudio initialization in console output
- Confirm audio parameters in `audioDefs.hpp`

**Simulation too fast/slow**:
- Adjust `BUFFER_SIZE` in `simDefs.hpp`
- Increase/decrease `damp_` parameter for faster/slower decay
- Check CPU usage with `top` or Activity Monitor

**Visualization lag**:
- Reduce grid size: `GRID_R`, `GRID_TH` → 64 (faster)
- Lower frame rate cap (not currently implemented)
- Enable release build: `-DCMAKE_BUILD_TYPE=Release`

## References

### Physics & Numerics
- Langtangen & Linge. *Finite Difference Computing with PDEs* (Free online)
- Smith, Julius O. *Physical Audio Signal Processing* (CCRMA, Stanford)
- Wave equation FDM: https://en.wikipedia.org/wiki/Finite_difference_method

### Audio Programming
- PortAudio: http://www.portaudio.com/
- PCM Audio Basics: https://en.wikipedia.org/wiki/Pulse-code_modulation

### Graphics
- LearnOpenGL: https://learnopengl.com/ (Modern GL tutorials)
- GLFW: https://www.glfw.org/
- GLM: https://glm.g-truc.net/
- OpenGL Course - Create 3D and 2D Graphics With C++: https://www.youtube.com/watch?v=45MIykWJ-C4&t=3828s&pp=ygUPb3BlbmdsIHR1dG9yaWFs

## License

This project is provided as-is for educational and research purposes.

## Contributing

Contributions welcome! Areas of interest:
- Optimization (GPU acceleration, SIMD)
- Interactive GUI improvements
- Additional drum presets and tuning

## Author

Developed as an exploration of physical modeling synthesis and real-time audio processing in C++.

---

**Latest Update**: April 2026  
**Current Status**: MVP with circular membrane (polar FDM), real-time audio, and 3D visualization
