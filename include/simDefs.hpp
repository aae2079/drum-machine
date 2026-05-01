#ifndef DRUM_MACHINE_SIM_DEFS_H
#define DRUM_MACHINE_SIM_DEFS_H

#define CFL 0.2  // Courant-Friedrichs-Lewy condition for stability

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
Surface density for Mylar drum head (kg/m^2).
c = sqrt(T/sigma) requires surface density, NOT volumetric density.
Derived from: volumetric density ~1400 kg/m^3 x membrane thickness ~0.1mm.
*/
#define MEMBRANE_THICKNESS 1.0e-4f                    // 0.1 mm in meters
#define MATERIAL_DENSITY   (1400.0f * MEMBRANE_THICKNESS) // ~0.14 kg/m^2
#define TENSION 150.0f // N/m, adjust for tighter or looser drum head
#define RADIUS 0.3f // meters, adjust for larger or smaller drum head
#define DAMPING 1.5f // s⁻¹, energy loss rate — higher = faster decay

#if defined(_WIN32) || defined(_WIN64)
    #define M_PI 3.14159265358979323846
#endif

#endif // DRUM_MACHINE_SIM_DEFS_H