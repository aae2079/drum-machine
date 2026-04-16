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
cd drum-machine/
git submodule update --init --recursive
```
Submodule update command will pull DAVE Debugger for GDB Audio Debugging by [maxmarsc](https://github.com/maxmarsc)

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
./drum-machine
```

## Controls

### Simulation
- **S** — Start/stop simulation
- **ESC** — Exit application

### Visualization
- **Arrow Keys (↑↓)** — Tilt membrane up/down
- **Arrow Keys (←→)** — Rotate membrane left/right

## Configuration

Edit `include/simDefs.hpp` to customize:

```cpp
#define CFL 0.25              // Courant stability parameter (< 0.5)
#define BUFFER_SIZE 2048      // Audio buffer chunk size
#define OVERLAP 512           // Overlap between chunks for streaming
#define GRID_R 100            // Membrane grid width (affects tone)
#define GRID_TH 100            // Membrane grid height (affects tone)
```

Edit `include/audioDefs.hpp` for audio settings:

```cpp
#define SAMPLE_RATE 48000     // Audio sample rate (Hz)
#define NUM_CHANNELS 1        // Mono output
#define BIT_DEPTH 16          // 16-bit audio
```

## Physics Parameters

In `src/backend/RectangularMembrane.cc`, adjust these to change drum sound:

```cpp
float amp = 0.1;              // Initial displacement amplitude
float alpha = 0.01;           // Gaussian width (strike localization)
float damp = 10.0;            // Damping coefficient (higher = faster decay)
float c = 1.0;                // Wave speed (higher = higher pitch)
```

### How to Tune for Different Drum Sounds

| Parameter | Effect | For Kick | For Tom | For Snare |
|-----------|--------|----------|---------|-----------|
| `GRID_R`, `GRID_TH` | Pitch | Large (100+) | Medium (70) | Small (50) |
| `damp` | Decay time | Low (5-8) | Medium (10) | High (15-20) |
| `c` | Brightness | Low (0.8) | Medium (1.0) | High (1.2) |
| `amp` | Strike strength | High (0.2) | Medium (0.1) | Low (0.05) |

## Architecture

### RectangularMembrane (Physics)
- Stores 3 grids: `prev_`, `curr_`, `next_` (finite difference states)
- `setInitialCondition()`: Sets Gaussian strike at center
- `Simulate()`: Steps physics forward by `BUFFER_SIZE` samples
- Extracts audio from center point with 15x gain

### AudioEngine (Audio I/O)
- Uses **PortAudio** for cross-platform audio
- Ring buffer with `NUM_FRAMES` slots for producer-consumer pattern
- Callback-based playback (real-time safe)
- `pushChunk()`: Main thread → ring buffer
- `internalAudioCB()`: Audio callback → playback

### DrumRenderer (Visualization)
- **OpenGL 3.3 Core** with GLFW window management
- Generates grid mesh at initialization
- `updateVertexData()`: Modifies Y-coordinates each frame
- Wireframe mode shows membrane motion clearly
- GLM matrices for 3D transformations

## Technical Highlights

### Real-Time Audio Streaming
- Chunked processing: 2048 samples per `Simulate()` call
- Overlapping buffers (512 samples) for seamless transitions
- Ring buffer prevents audio glitches
- ~42.7 ms latency (BUFFER_SIZE / SAMPLE_RATE)

### Numerical Stability
- **CFL Condition**: `(c*dt/dx)² < 0.5` enforced in parameters
- Current: CFL = 0.25 (safe margin)
- Explicit time-stepping (stable for chosen parameters)
- Boundary conditions: Clamped edges (u = 0)

### Performance
- OpenMP parallelization of spatial grid loop
- Vectorized operations via std algorithms
- ~100×100 grid runs at 60+ FPS on modern hardware
- Profile with: `perf record ./drum-machine`

## Audio Output Quality

- **Sample Rate**: 48 kHz (professional audio standard)
- **Bit Depth**: 16-bit signed PCM
- **Channels**: Mono
- **Dynamic Range**: ±1.0 (clamped to prevent clipping)

## Future Enhancements (from Development Plan)

### Phase 2: Advanced Features
- [ ] Circular membrane with polar coordinates
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

Unit test with audio output:

```bash
cd test
make
./test_rectangular_membrane
# Press 'S' to run simulation, 'E' to exit
```

Generates `output.wav` with the drum sound (compile with `WAVE_FILE 1` in rectangularMembraneUnitTest.cc to dump wav file).

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
- Circular membrane implementation (polar coordinates)
- Optimization (GPU acceleration, SIMD)
- Interactive GUI improvements
- Additional drum presets and tuning

## Author

Developed as an exploration of physical modeling synthesis and real-time audio processing in C++.

---

**Latest Update**: March 2026  
**Current Status**: MVP with rectangular membrane, real-time audio, and 3D visualization
