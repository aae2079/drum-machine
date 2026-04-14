#ifndef DRUM_MACHINE_CIRCULAR_MEMBRANE_H
#define DRUM_MACHINE_CIRCULAR_MEMBRANE_H

#include <vector>
#include <cstddef>
#include <string>
#include <cmath>
#include "simDefs.hpp"
#include "audioDefs.hpp"


class CircularMembrane {
public:
    // Construct a membrane with given radius (meters) and tension (N/m)
    CircularMembrane();
    ~CircularMembrane();

    void init(float radius, float tension, float rho_density,unsigned int Nr, unsigned int Ntheta);
    void cleanup();

    std::vector<float>& getCurrentGrid() { return u_curr_; }
    std::vector<float>& getAudioBuffer() { return audioBuf_; }
        
    
    void setInitialCondition();
    void Simulate();

private:
    float radius_;   // meters
    float tension_;  // N/m
    float rho_;     // mass density kg/m^2
    float c_;       // wave speed m/s
    float dt_;      // time step s
    float dr_;      // radial step size m
    float dtheta_;  // angular step size radians


    // Discretization / storage placeholders
    unsigned int Nr_; // radial samples
    unsigned int Ntheta_; //angular samples

    std::vector<float> u_prev_; // membrane state at previous time step
    std::vector<float> u_curr_; // membrane state at current time step
    std::vector<float> u_next_; // membrane state at next time step
    
    std::vector<float> audioBuf_; // audio buffer output
    std::vector<float> histBuf_; // history buffer for overlap
    
    int firstTime; // flag for first time processing

};

#endif // DRUM_MACHINE_CIRCULAR_MEMBRANE_H
