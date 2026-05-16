#ifndef DRUM_MACHINE_CIRCULAR_MEMBRANE_H
#define DRUM_MACHINE_CIRCULAR_MEMBRANE_H

#include <vector>
#include <cstddef>
#include <string>
#include <cmath>
#include "strikeDefs.hpp"

#define CFL 0.2  // Courant-Friedrichs-Lewy condition for stability
#if defined(_WIN32) || defined(_WIN64)
    #define M_PI 3.14159265358979323846
#endif

class CircularMembrane {
public:
    // Construct a membrane with given radius (meters) and tension (N/m)
    CircularMembrane();
    ~CircularMembrane();

    void init(float radius, float damp, float tension, float rho_density,unsigned int Nr, unsigned int Ntheta);
    void cleanup();

    std::vector<float>& getCurrentGrid() { return u_curr_; }
    float& getSimRate() { return simRate_; }

    bool isActive();
    
    void setInitialCondition(const StrikeDefs* strike);
    void Simulate(int physSteps, std::vector<float>& physBuf);


private:
    float radius_;   // meters
    float tension_;  // N/m 
    float damp_;
    float rho_;     // mass density kg/m^2
    float c_;       // wave speed m/s
    float dt_;      // time step s
    float dr_;      // radial step size m
    float dtheta_;  // angular step size radians
    float simRate_; // simulation sample rate (Hz)

    float maxAmplitude_=100.0f; // track max amplitude for normalization or other purposes


    // Discretization / storage placeholders
    unsigned int Nr_; // radial samples
    unsigned int Ntheta_; //angular samples

    std::vector<float> u_prev_; // membrane state at previous time step
    std::vector<float> u_curr_; // membrane state at current time step
    std::vector<float> u_next_; // membrane state at next time step
    std::vector<float> simBuf_; // buffer for current simulation chunk
    

};

#endif // DRUM_MACHINE_CIRCULAR_MEMBRANE_H
