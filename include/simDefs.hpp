#ifndef DRUM_MACHINE_SIM_DEFS_H
#define DRUM_MACHINE_SIM_DEFS_H

#define CFL 0.3  // Courant-Friedrichs-Lewy condition for stability
#define BUFFER_SIZE 2048
#define OVERLAP 512 // Number of samples to overlap for streaming

//Sets tonality of the drum sound, bigger membrane = lower tone, smaller membrane = higher tone
//This will be useful when creating a drum set!


/*------- CircularMembrane Definitions----------*/
/*

    GRID_R = radial rings
    GRID_TH = angular samples per ring

*/
#define GRID_R 50
#define GRID_TH 75
/*

Material Density for Mylar (Typical drum head mateiral) ranges from 
1.39 - 1.40 g/cm^3
*/
#define MATERIAL_DENSITY 1.4E3 // in g/m^3, converted from g/cm^3
#define TENSION 100.0f 
#define RADIUS 0.3f

#if defined(_WIN32) || defined(_WIN64)
    #define M_PI 3.14159265358979323846
#endif

#endif // DRUM_MACHINE_SIM_DEFS_H