# 2D Membrane Drum Machine - Complete Implementation Plan

**Tech Stack**: C++ with SDL2 (or SFML) 

**NOTE**: 11/22/25 - Original implentation of this plan was desgined by Claude AI

## Project Overview
Build a physically-accurate drum machine by simulating a 2D membrane using the wave equation. The simulation will generate real audio from the membrane vibrations and provide an interactive GUI for user control.

---

## Phase 1: Core Wave Equation Solver (Week 1)

### Task 1.1: Project Setup
- [x] Create directory structure: `src/`, `include/`, `lib/`, `build/`
- [x] Set up build system (CMake recommended)
- [ ] Create basic classes: `Membrane`, `WaveSimulator`
- [ ] Initialize 2D grid (start with 128x128, adjustable later)
- [ ] Use `std::vector<std::vector<double>>` for displacement (u), velocity (v), and previous displacement (u_old)

### Task 1.2: Implement Finite Difference Solver
- [ ] Implement explicit finite difference method:
  ```
  u_new[i][j] = 2*u[i][j] - u_old[i][j] + c²*dt²*Laplacian(u[i][j])
  ```
- [ ] Calculate Laplacian (5-point stencil):
  ```
  Laplacian = (u[i+1][j] + u[i-1][j] + u[i][j+1] + u[i][j-1] - 4*u[i][j]) / (dx²)
  ```
- [ ] Implement fixed boundary conditions: `u = 0` at grid edges
- [ ] Add damping factor: multiply by 0.995-0.999 each step
- [ ] Ensure stability: verify `c*dt/dx < 1` (Courant-Friedrichs-Lewy condition)

### Task 1.3: Hard-coded Initial Excitation
- [ ] Implement Gaussian bump: `u[i][j] = A * exp(-r²/(2σ²))` at center
- [ ] Or velocity impulse: set `v[i][j] = intensity` at strike point
- [ ] Test with console output: print center displacement over time
- [ ] Verify wave propagation with data file output
- [ ] Create simple visualization script (Python/gnuplot) to verify physics

**Deliverable**: Working wave equation solver with verifiable propagation

---

## Phase 2: Basic GUI Setup (Week 2)

### Task 2.1: Choose and Integrate GUI Library
- [ ] **Recommended**: Install SDL2 and SDL2_ttf
  - Simple 2D rendering with `SDL_Renderer`
  - Built-in audio support
  - Cross-platform input handling
- [ ] Alternative: SFML (more C++-friendly)
- [ ] Link libraries in build system
- [ ] Create basic window (512x512 or 800x600)
- [ ] Set up render loop at 60 FPS

### Task 2.2: Render Membrane Visualization
- [ ] Map displacement values to colors:
  - Negative displacement → Blue
  - Zero displacement → Gray/White
  - Positive displacement → Red
  - Use color gradient for smooth visualization
- [ ] Create `Renderer` class with `renderMembrane()` method
- [ ] Map grid coordinates to screen pixels
- [ ] Update texture/surface each frame
- [ ] Add FPS counter display

### Task 2.3: Real-time Simulation Loop
- [ ] Decouple physics timestep from render framerate
- [ ] Physics: 10-50 substeps per render frame (tune for stability)
- [ ] Implement game loop:
  ```cpp
  while (running) {
      handleInput();
      for (int i = 0; i < physicsSubsteps; i++) {
          updateWaveEquation();
          collectAudioSample(); // Sample at 44.1kHz
      }
      render();
      limitFramerate(60);
  }
  ```
- [ ] Add pause/resume capability (Space key)
- [ ] Visual confirmation of wave propagation

**Deliverable**: Window showing animated membrane with hard-coded strike

---

## Phase 3: Audio Integration (Week 3)

### Task 3.1: Extract Audio from Simulation
- [ ] Choose sampling point(s) on membrane (center or average of region)
- [ ] Convert displacement to audio amplitude
- [ ] Accumulate samples at 44.1kHz in circular buffer
- [ ] Handle sample rate conversion if physics timestep ≠ audio sample period
- [ ] Normalize audio output to prevent clipping

### Task 3.2: Implement Audio Output
- [ ] Use SDL_Audio callback system:
  ```cpp
  void audioCallback(void* userdata, Uint8* stream, int len) {
      // Fill stream with membrane samples
  }
  ```
- [ ] Or use PortAudio for more control
- [ ] Set up thread-safe audio buffer (lock-free ring buffer)
- [ ] Initialize audio device: 44.1kHz, 16-bit, mono or stereo
- [ ] Test with sine wave first to verify audio pipeline

### Task 3.3: Synchronize Audio and Visual
- [ ] Ensure physics timestep produces exactly 44100 samples/sec
- [ ] Calculate required physics steps: `physicsStepsPerSecond = sampleRate`
- [ ] Verify no audio glitches or pops
- [ ] Add audio on/off toggle (M key for mute)
- [ ] Implement WAV file export for debugging

### Task 3.4: Parameter Tuning for Drum-like Sound
- [ ] Tune wave speed `c` (affects pitch)
- [ ] Tune damping factor (affects decay time)
- [ ] Tune strike intensity and width
- [ ] Experiment with membrane size
- [ ] Test different sampling positions

**Deliverable**: Audible drum sound synchronized with visual membrane

---

## Phase 4: Interactive Controls - Basic (Week 4)

### Task 4.1: Mouse Click for Excitation
- [ ] Implement mouse event handling
- [ ] Convert screen coordinates to grid coordinates
- [ ] Apply Gaussian velocity impulse at click location
- [ ] Visual feedback: circle at strike point
- [ ] Support click-and-drag for strike intensity (optional)
- [ ] Remove hard-coded excitation code

### Task 4.2: Keyboard Controls
- [ ] Implement parameter adjustment keys:
  - `↑/↓`: Adjust damping ±0.0001
  - `→/←`: Adjust membrane tension (wave speed c)
  - `+/-`: Adjust strike intensity
  - `Space`: Clear membrane (reset to zero)
  - `M`: Mute/unmute audio
  - `P`: Pause/resume simulation
  - `R`: Start/stop WAV recording
  - `ESC`: Quit application
- [ ] Add key repeat handling
- [ ] Provide visual feedback for parameter changes

### Task 4.3: Display Basic Info
- [ ] Integrate SDL_ttf for text rendering
- [ ] Display overlay with current parameters:
  - FPS
  - Damping coefficient
  - Wave speed
  - Audio status (on/off, recording)
- [ ] Show instructions (toggle with `H` key)
- [ ] Semi-transparent background for text readability

**Deliverable**: Fully interactive drum with mouse striking and parameter control

---

## Phase 5: Circular Membrane Implementation (Week 5)

### Task 5.1: Implement Circular Boundary
- [ ] Create circular mask: `if (distance_from_center > radius) skip_cell`
- [ ] Pre-calculate mask during initialization
- [ ] Implement circular boundary condition: `u = 0` outside circle
- [ ] Modify Laplacian near boundary:
  - Handle cells partially inside circle
  - Use one-sided differences or ghost cells
- [ ] Keep rectangular grid for simplicity

### Task 5.2: Update Visualization for Circle
- [ ] Render only circular region (draw black outside)
- [ ] Draw circle outline/rim
- [ ] Optional: Add radial grid lines for visual reference
- [ ] Adjust color mapping if needed
- [ ] Center circle in window

### Task 5.3: Circular Geometry Considerations
- [ ] Test edge strikes vs center strikes (different sounds)
- [ ] Consider implementing strike patterns:
  - Center (bass-like)
  - 2/3 radius (balanced tone)
  - Edge/rim (high pitch)
- [ ] Verify boundary reflections look correct
- [ ] Tune parameters for circular membrane

**Deliverable**: Circular drum membrane with realistic boundary behavior

---

## Phase 6: Advanced GUI Features (Week 6-7)

### Task 6.1: Enhanced Excitation Controls
- [ ] Click-and-drag for variable strike intensity
- [ ] Visual strike indicator (circle size = intensity)
- [ ] Implement strike types (number keys 1-4):
  1. Point strike (sharp, localized)
  2. Area strike (mallet, broader)
  3. Velocity impulse (instant)
  4. Displacement impulse (gradual)
- [ ] Right-click for different strike type
- [ ] Strike preview (show what will happen before releasing mouse)

### Task 6.2: Integrate Dear ImGui (Optional but Recommended)
- [ ] Add Dear ImGui with SDL2 backend
- [ ] Create parameter control panel:
  - Sliders for damping (0.95 - 0.9999)
  - Slider for tension (wave speed)
  - Slider for strike intensity
  - Slider for strike radius
  - Color scheme selector
- [ ] Dropdown menu for membrane presets
- [ ] Real-time parameter updates
- [ ] Save/load preset buttons

### Task 6.3: Pattern Sequencer UI
- [ ] Design timeline grid (16 or 32 steps)
- [ ] Click grid cells to place strikes
- [ ] Multiple tracks (different strike locations)
- [ ] Playback controls:
  - Play/Pause/Stop
  - Loop toggle
  - BPM slider (60-240)
  - Metronome click (optional)
- [ ] Highlight current beat during playback
- [ ] Clear pattern button

### Task 6.4: Visualization Enhancements
- [ ] Multiple color schemes:
  - Plasma (purple-orange)
  - Heatmap (blue-red)
  - Grayscale
  - Custom gradient
- [ ] Displacement scale slider (zoom in/out on small vibrations)
- [ ] Motion blur or trail effect (shows wave history)
- [ ] Waveform display (audio output preview)
- [ ] 3D height visualization (optional, using simple projection)
- [ ] Particle effects on strike (cosmetic)

**Deliverable**: Professional-looking drum machine with full GUI controls

---

## Phase 7: Multiple Membranes & Sound Design (Week 8)

### Task 7.1: Membrane Presets
- [ ] Create preset system:
  - Kick drum (large, low damping, center strike)
  - Snare (medium, high damping, off-center)
  - Tom (medium-large, medium damping)
  - Hi-hat (small, very high damping, edge strike)
  - Custom (user-defined)
- [ ] Store presets as JSON or config files
- [ ] Load preset button in GUI
- [ ] Save custom presets

### Task 7.2: Multi-membrane System
- [ ] Create array of independent membranes
- [ ] Assign keyboard keys to different membranes (Q, W, E, R, etc.)
- [ ] Mix audio outputs from all membranes
- [ ] Visual selection: show which membrane is active
- [ ] Individual parameter control per membrane
- [ ] Solo/mute per membrane

### Task 7.3: Basic Effects
- [ ] Volume control (master and per-membrane)
- [ ] Simple reverb (convolution or basic delay network)
- [ ] Delay effect with feedback
- [ ] Low-pass/high-pass filter
- [ ] Distortion/overdrive (optional)
- [ ] Effects bypass toggle

**Deliverable**: Multi-timbral drum machine with effects

---

## Phase 8: Advanced Features & Physics (Week 9-10)

### Task 8.1: Advanced Physics
- [ ] Non-uniform membrane (variable density/tension)
- [ ] Non-linear membrane (large amplitude behavior)
- [ ] Coupled membranes (sympathetic resonance)
- [ ] Temperature/humidity effects (stretch parameters)
- [ ] Different boundary conditions:
  - Free boundary (can vibrate)
  - Partially fixed
  - Spring boundary

### Task 8.2: Modal Analysis (Optional, Advanced)
- [ ] Calculate membrane eigenmodes
- [ ] Display mode shapes
- [ ] Allow exciting specific modes
- [ ] Educational mode showing physics

### Task 8.3: Performance Optimization
- [ ] Profile code (gprof, perf, or VS profiler)
- [ ] Optimize Laplacian calculation (cache locality)
- [ ] SIMD vectorization (SSE/AVX) for grid updates
- [ ] OpenMP parallelization:
  ```cpp
  #pragma omp parallel for
  for (int i = 1; i < N-1; i++) { ... }
  ```
- [ ] GPU acceleration with CUDA/OpenCL (optional, major effort)
- [ ] Adaptive grid refinement (focus resolution near strikes)

**Deliverable**: Optimized, feature-complete drum machine

---

## Phase 9: Export & Integration (Week 11)

### Task 9.1: Export Capabilities
- [ ] Export audio to WAV/FLAC/MP3
- [ ] Export patterns to MIDI
- [ ] Export patterns to custom format (JSON)
- [ ] Import patterns
- [ ] Screenshot membrane state (PNG)
- [ ] Record video of membrane (optional)

### Task 9.2: MIDI Integration
- [ ] MIDI input support (RtMidi library)
- [ ] Map MIDI notes to strike locations
- [ ] MIDI velocity → strike intensity
- [ ] MIDI CC → parameter control
- [ ] MIDI clock sync for sequencer

### Task 9.3: VST/Audio Plugin (Optional, Advanced)
- [ ] Use JUCE or iPlug2 framework
- [ ] Convert to VST3/AU plugin
- [ ] DAW integration
- [ ] Automation support

**Deliverable**: Integrated drum machine for music production

---

## Phase 10: Polish & Release (Week 12+)

### Task 10.1: Documentation
- [ ] User manual (controls, presets, tips)
- [ ] Tutorial video or GIFs
- [ ] Code documentation (Doxygen)
- [ ] README with build instructions
- [ ] Physics explanation document

### Task 10.2: Testing & Bug Fixes
- [ ] Cross-platform testing (Windows, Mac, Linux)
- [ ] Edge case handling (grid boundaries, extreme parameters)
- [ ] Memory leak checking (Valgrind)
- [ ] Audio glitch investigation
- [ ] GUI responsiveness testing

### Task 10.3: Packaging & Distribution
- [ ] Create installers (NSIS for Windows, DMG for Mac)
- [ ] Static linking or bundle dependencies
- [ ] GitHub releases
- [ ] Demo video
- [ ] Website/landing page (optional)

**Deliverable**: Polished, distributable drum machine

---

## Technology Stack Summary

### Core Libraries
- **SDL2**: Window management, 2D rendering, input, audio
- **SDL2_ttf**: Text rendering (alternative: stb_truetype)
- **Dear ImGui**: Advanced GUI controls (Phase 6+)

### Optional Libraries
- **RtMidi**: MIDI input/output (Phase 9)
- **libsndfile**: Advanced audio file I/O
- **FFTW**: FFT for spectral analysis/effects
- **OpenMP**: Parallelization
- **JUCE**: Plugin development (Phase 9)

### Build System
- **CMake**: Recommended for cross-platform builds
- **Makefile**: Simpler for single-platform development

---

## Project Structure
```
drum-machine/
├── src/
│   ├── main.cpp
│   ├── membrane.cpp           # Wave equation solver
│   ├── audio_engine.cpp       # Audio sampling and output
│   ├── renderer.cpp           # SDL2 rendering
│   ├── input_handler.cpp      # Mouse/keyboard events
│   ├── sequencer.cpp          # Pattern sequencer (Phase 6)
│   ├── preset_manager.cpp     # Preset loading/saving (Phase 7)
│   └── effects.cpp            # Audio effects (Phase 7)
├── include/
│   ├── membrane.h
│   ├── audio_engine.h
│   ├── renderer.h
│   ├── input_handler.h
│   ├── sequencer.h
│   ├── preset_manager.h
│   └── effects.h
├── assets/
│   ├── fonts/
│   ├── presets/
│   └── shaders/ (if using OpenGL)
├── docs/
│   └── manual.md
├── tests/
│   └── unit_tests.cpp
├── CMakeLists.txt
├── README.md
└── LICENSE
```

---

## Critical Success Factors

1. **Start Simple**: Rectangular membrane, hard-coded excitation, console output
2. **Iterate Quickly**: Get each phase working before adding complexity
3. **Test Audio Early**: Audio bugs are hard to debug
4. **Maintain Stability**: Always verify Courant condition
5. **Profile Often**: Real-time simulation is demanding
6. **Version Control**: Commit after each working feature
7. **Backup Plans**: Have alternative approaches ready (e.g., SFML if SDL2 issues)

---

## Minimum Viable Product (MVP) Checkpoints

- **MVP 1** (Week 2): Visual membrane responding to hard-coded strike
- **MVP 2** (Week 3): Above + audible drum sound
- **MVP 3** (Week 4): Above + interactive mouse striking
- **MVP 4** (Week 5): Above + circular membrane
- **MVP 5** (Week 6): Above + GUI parameter controls
- **MVP 6** (Week 7): Above + pattern sequencer
- **MVP 7** (Week 8): Above + multiple membrane types
- **MVP 8** (Week 9): Above + export capabilities
- **Final** (Week 12+): Polished, distributable application

---

## Estimated Total Timeline
- **Minimum** (MVP 3): 4 weeks (interactive drum with audio)
- **Full-featured** (MVP 6): 7 weeks (sequencer + GUI)
- **Production-ready** (MVP 8): 12+ weeks (polished, exportable)

---

## Resources & References

### Wave Equation & Numerical Methods
- "Finite Difference Computing with PDEs" by Langtangen & Linge
- "Numerical Recipes" (C++ Edition)
- Jos & Smith: "Physical Audio Signal Processing"

### SDL2 Tutorials
- LazyFoo's SDL2 tutorials
- SDL2 official documentation

### Dear ImGui
- Official GitHub repository with examples
- Integration guides for SDL2

### Audio Programming
- "Designing Audio Effect Plugins in C++" by Pirkle
- "The Audio Programming Book" by Boulanger & Lazzarini

---

## Notes
- This plan is modular: you can stop at any MVP and have a working product
- Phases 8-10 are optional enhancements
- Adjust timeline based on your available time and experience level
- Consider using Git branches for experimental features
- Join communities: r/DSP, r/audioengineering, r/cpp for help

Good luck with your drum machine! 🥁