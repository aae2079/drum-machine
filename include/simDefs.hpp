#ifndef DRUM_MACHINE_SIM_DEFS_H
#define DRUM_MACHINE_SIM_DEFS_H

#define CFL 0.3  // Courant-Friedrichs-Lewy condition for stability
#define BUFFER_SIZE 2048
#define OVERLAP 512 // Number of samples to overlap for streaming

//Sets tonality of the drum sound, bigger membrane = lower tone, smaller membrane = higher tone
//This will be useful when creating a drum set!

/*

    GRID_X in CircularCase = radial rings
    GRID_Y in CircularCase = angular samples per ring

    GRID_X in RectangularCase = number of vertices in x direction
    GRID_Y in RectangularCase = number of vertices in y direction


*/
#define GRID_X 50
#define GRID_Y 75

#if defined(_WIN32) || defined(_WIN64)
    #define M_PI 3.14159265358979323846
#endif

#endif // DRUM_MACHINE_SIM_DEFS_H