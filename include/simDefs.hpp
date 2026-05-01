#ifndef DRUM_MACHINE_SIM_DEFS_H
#define DRUM_MACHINE_SIM_DEFS_H

#include "audioDefs.hpp"

#define CFL 0.2  // Courant-Friedrichs-Lewy condition for stability

#if defined(_WIN32) || defined(_WIN64)
    #define M_PI 3.14159265358979323846
#endif

typedef struct {
    CircularMembrane membrane;
    int simRunning = 0;
	float dB = 0.0f;
}SimState;

typedef struct{
    /*
    Surface density for Mylar drum head (kg/m^2).
    c = sqrt(T/sigma) requires surface density, NOT volumetric density.
    Derived from: volumetric density ~1400 kg/m^3 x membrane thickness ~0.1mm.
    */
    float membrane_thickness; // in meters
    float material_density; // in kg/m^2
    float tension; // in N/m
    float radius; // in meters
    float damping; // in s⁻¹, energy loss rate — higher = faster decay
}TimbreParams;

typedef struct {
    unsigned int grid_r; // radial rings
    unsigned int grid_th; // angular samples per ring
}RadialDimensions;

/*------- CircularMembrane Definitions----------*/
typedef struct {
    AudioDefinitions audio;
    TimbreParams timbre;
    RadialDimensions grid;
}Params;

#endif // DRUM_MACHINE_SIM_DEFS_H