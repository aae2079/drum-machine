# Drum Machine: Physical Modeling Synthesis

A real-time 2D membrane drum synthesizer using finite difference method (FDM) to solve the 2D wave equation. This project combines physics-based sound synthesis with interactive visualization to create a physically accurate digital drum instrument.

## Overview

This drum machine simulates the vibration of a 2D circular membrane (like a drum head) using numerical solutions to the 2D wave equation in polar coordinates with damping. The displacement of the membrane is converted into audio in real-time at 96 kHz, while an interactive 3D visualization shows the membrane's motion. Users can click anywhere on the rendered membrane to strike it at that exact position.

<p align="center">
   <img src="https://github.com/user-attachments/assets/50881e13-a1ef-493e-a4f9-566fb767dfe8" />

</p>

### Key Features

- **Physics-Based Synthesis**: Solves the 2D wave equation in polar coordinates using finite difference method
- **Threaded Physics Engine**: Physics runs in a dedicated background thread (`PhysicsThread`), decoupled from the render loop via mutex/condition-variable queues
- **Real-Time Audio**: 96 kHz audio output via PortAudio with ring-buffer producer-consumer decoupling
- **Audio DSP Toolbox**: Dedicated dsp module (`AudioDSP_Toolbox`) for resampling and gain
- **JSON Configuration**: All simulation and audio parameters loaded at runtime from `drum_config.json` — no recompilation required
- **Strike Placement**: Ray-cast mouse interaction lets you strike any point on the membrane; strike position (r, θ) is passed to the physics solver
- **3D Visualization**: Interactive OpenGL rendering with rotation, tilt, scroll zoom, and audio toggle controls
- **Damping Simulation**: Energy loss modeling for realistic drum decay
- **Modular Architecture**: Physics, audio engine, DSP, and rendering are fully decoupled components

## Project Structure

```
drum-machine/
├── src/
│   ├── main.cc                              # Main application loop
│   ├── backend/
│   │   ├── physics/
│   │   │   ├── PhysicsThread.cc             # Dedicated physics thread (run loop, strike queue)
│   │   │   └── head/
│   │   │       ├── CircularMembrane.cc      # Polar FDM physics solver
│   │   │       └── RectangularMembrane.cc   # Cartesian FDM solver (legacy/tests)
│   │   └── audio/
│   │       ├── dsp/
│   │       │   └── audioDSP.cc             # AudioDSP_Toolbox: resampling & DSP utilities
│   │       └── engine/
│   │           └── audioEngine.cc          # PortAudio I/O with ring buffer
│   ├── frontend/
│   │   ├── drumRenderer.cc                 # OpenGL rendering
│   │   ├── default.vert                    # Vertex shader
│   │   └── default.frag                    # Fragment shader
│   └── 3rdparty/
│       └── glad.c                          # GLAD OpenGL loader
├── include/
│   ├── PhysicsThread.hpp                    # Physics thread interface
│   ├── CircularMembrane.hpp
│   ├── RectangularMembrane.hpp
│   ├── audioEngine.hpp
│   ├── audioDSP.hpp                         # AudioDSP_Toolbox interface
│   ├── drumRenderer.hpp
│   ├── JsonParser.hpp                       # JSON config loader (rapidjson)
│   ├── Surface.hpp                          # Abstract membrane base (placeholder)
│   ├── audioDefs.hpp                        # AudioDefinitions struct
│   ├── simDefs.hpp                          # Params, TimbreParams, RadialDimensions structs
│   ├── strikeDefs.hpp                       # Strike parameters (amplitude, rPos, thetaPos)
│   └── wav.hpp                              # WAV file format
├── config/
│   └── drum_config.json                     # Runtime configuration (audio, timbre, grid, zoom)
├── dependencies/                            # Third-party headers (GLAD, KHR)
├── test/
│   └── rectangularMembraneUnitTest.cc
├── CMakeLists.txt
└── clean_build.sh                           # Build script
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
brew install cmake glfw portaudio glm rapidjson-dev
```

**Linux (Ubuntu/Debian)**:
```bash
sudo apt-get install cmake libglfw3-dev portaudio19-dev libglm-dev rapidjson-dev
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

If built manually run the following cmds:
```bash
cd build/bin/
mkdir -p shaders/ config/
ln -sf ../../src/frontend/default* shaders/
ln -sf ../../config/* config/
```

### Run
```bash
./drum-machine config/drum_config.json
```
## Controls

### Simulation
- **Mouse Click** — Strike the membrane at the clicked position; ray-cast maps the click to the membrane's (r, θ) coordinates
- **ESC** — Exit application

### Visualization
- **Arrow Keys (↑↓)** — Tilt membrane up/down
- **Arrow Keys (←→)** — Rotate membrane left/right
- **Scroll Wheel** — Zoom in/out (field of view, 10°–90°)
- **M** — Toggle audio output on/off

## Configuration

All runtime parameters are loaded from `config/drum_config.json` at startup — no recompilation needed. Pass it as the first argument to the binary:

```bash
./drum-machine config/drum_config.json
```

```json
{
    "audio": {
        "sample_rate": 96000.0,
        "buffer_size": 512,
        "bit_depth": 16,
        "num_channels": 1,
        "audio_format_pcm": 1
    },
    "timbre": {
        "membrane_thickness": 0.0001,
        "material_density": 1400.0,
        "tension": 100.0,
        "radius": 0.5,
        "damping": 1.0
    },
    "dimensions": {
        "grid_r": 50,
        "grid_th": 75
    },
    "zoom": {
        "sensitivity_constant": 0.5
    }
}
```

| Field | Description |
|-------|-------------|
| `audio.sample_rate` | Output sample rate in Hz (default 96000) |
| `audio.buffer_size` | Samples per audio buffer — affects latency |
| `timbre.tension` | N/m — tighter = higher pitch, faster decay |
| `timbre.material_density` | kg/m³ — multiplied by thickness to get surface density |
| `timbre.radius` | Physical drum head radius in meters |
| `timbre.damping` | Energy loss rate (s⁻¹) — higher = shorter sustain |
| `dimensions.grid_r` | Radial rings — larger = lower pitch, more CPU |
| `dimensions.grid_th` | Angular samples per ring |
| `zoom.sensitivity_constant` | Scroll wheel zoom speed |

## Physics Parameters

Strike parameters are passed via the `StrikeDefs` struct (defined in `include/strikeDefs.hpp`):

```cpp
struct StrikeDefs {
    float amplitude;  // Normalized strike amplitude (0.0–1.0)
    float rPos;       // Normalized radial position (0.0 = center, 1.0 = edge)
    float thetaPos;   // Angular position in radians (0 to 2π)
};
```

The Gaussian width is controlled inside `CircularMembrane::setInitialCondition()` (`src/backend/physics/head/CircularMembrane.cc`).

### How to Tune for Different Drum Sounds

| JSON field | Effect | For Kick | For Tom | For Snare |
|------------|--------|----------|---------|-----------|
| `dimensions.grid_r` / `grid_th` | Pitch | Large (100+) | Medium (50) | Small (30) |
| `timbre.tension` | Pitch / decay | Low (80) | Medium (150) | High (250) |
| `timbre.radius` | Pitch | Large (0.4) | Medium (0.3) | Small (0.18) |
| `timbre.damping` | Sustain | Low (0.5) | Medium (1.0) | High (3.0) |
| Strike `amplitude` | Strike strength | High (0.8) | Medium (0.5) | Low (0.2) |

## Architecture

<p align="center">
   <img src="https://github.com/user-attachments/assets/5b312b35-5778-4805-91d1-baece7332942" />
</p>
*Architecture Designed by Aaron Escobar. Diagram generated using Claude

### PhysicsThread (Threading)
- Runs `CircularMembrane` on a dedicated background thread, decoupled from the render loop
- Strike events are enqueued via `pushStrike()`; grid snapshots are shared with the renderer via `tryGetGrid()`

### CircularMembrane (Physics)
- Solves the 2D wave equation in polar coordinates using explicit finite differences
- Three time-step grids (`u_prev_`, `u_curr_`, `u_next_`) advanced via pointer swap
- Handles origin singularity by averaging angular neighbours; Dirichlet boundary at outer edge

### AudioDSP_Toolbox (DSP)
- `sampleInterp()`: Linear resampling from the physics simulation rate → 96 kHz output rate
- `applyGain()`: Scales the audio buffer before pushing to the ring buffer

### AudioEngine (Audio I/O)
- PortAudio wrapper with a 15-slot ring buffer for producer-consumer audio streaming
- `pushChunk()` is called from the physics thread; the real-time callback drains it

### DrumRenderer (Visualization)
- **OpenGL 3.3 Core** with GLFW window management
- Renders the polar grid as a wireframe mesh; GLM matrices handle rotation, tilt, and FOV zoom

## Technical Highlights

### Real-Time Audio Streaming
- Physics runs in its own thread; `sampleInterp()` resamples output to 96 kHz before pushing to the ring buffer
- Ring buffer prevents audio glitches under CPU load
- ~5 ms latency (512 samples / 96 kHz)

### Numerical Stability
- **CFL Condition**: `(c·dt/dr)² ≤ CFL²` enforced at init
- Current CFL = 0.2 (safe margin below 0.5 limit)
- Origin singularity (r=0) handled by averaging all angular neighbours
- Boundary conditions: Dirichlet (u=0) at outer edge

### Performance
- OpenMP parallelization of the spatial grid loop in `Simulate()`
- 50×75 polar grid runs smoothly on modern hardware
- Reduce `grid_r`/`grid_th` in `drum_config.json` if performance is insufficient

## Audio Output Quality

- **Sample Rate**: 96 kHz (configurable in `drum_config.json`)
- **Bit Depth**: 16-bit signed PCM
- **Channels**: Mono
- **Dynamic Range**: ±1.0 (clamped to prevent clipping)

## Future Enhancements (from Development Plan)

### Phase 2: Advanced Features
- [x] Multiple strike locations (mouse interaction — ray-cast to (r, θ))
- [x] Threaded physics engine (decoupled from render loop)
- [x] JSON-based runtime configuration
- [x] Scroll-to-zoom camera control
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
- Confirm `audio.sample_rate` and `audio.buffer_size` in `drum_config.json`

**Simulation too fast/slow**:
- Adjust `audio.buffer_size` in `drum_config.json`
- Increase `timbre.damping` for faster decay; decrease for longer sustain
- Check CPU usage with `top` or Activity Monitor

**Visualization lag**:
- Reduce grid size: lower `dimensions.grid_r` / `grid_th` in `drum_config.json`
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

**Latest Update**: May 2026  
**Current Status**: Threaded architecture with dedicated physics thread, JSON runtime config, 96 kHz audio, scroll zoom, and ray-cast strike placement
