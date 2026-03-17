#ifndef DRUM_MACHINE_SIM_DEFS_H
#define DRUM_MACHINE_SIM_DEFS_H

#define CFL 0.25  // Courant-Friedrichs-Lewy condition for stability
#define BUFFER_SIZE 2048
#define OVERLAP 512 // Number of samples to overlap for streaming

//Sets tonality of the drum sound, bigger membrane = lower tone, smaller membrane = higher tone
//This will be useful when creating a drum set!
#define GRID_X 100
#define GRID_Y 100

#endif // DRUM_MACHINE_SIM_DEFS_H