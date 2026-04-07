#ifndef DRUM_MACHINE_SIM_DEFS_H
#define DRUM_MACHINE_SIM_DEFS_H

#define CFL 0.6  // Courant-Friedrichs-Lewy condition for stability
#define BUFFER_SIZE 2048
#define OVERLAP 512 // Number of samples to overlap for streaming

//Sets tonality of the drum sound, bigger membrane = lower tone, smaller membrane = higher tone
//This will be useful when creating a drum set!
#define GRID_X 100
#define GRID_Y 100

#if defined(_WIN32) || defined(_WIN64)
    #define M_PI 3.14159265358979323846
#endif

#endif // DRUM_MACHINE_SIM_DEFS_H