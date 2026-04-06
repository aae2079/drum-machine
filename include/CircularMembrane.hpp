#ifndef DRUM_MACHINE_CIRCULAR_MEMBRANE_H
#define DRUM_MACHINE_CIRCULAR_MEMBRANE_H

#include <vector>
#include <cstddef>
#include <string>
#include <cmath>
#include "simDefs.hpp"
#include "audioDefs.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class CircularMembrane {
public:
    // Construct a membrane with given radius (meters) and tension (N/m)
    CircularMembrane(double radius = 0.1, double tension = 100.0, double rho = 0.01, double c = 1.0, double dt = 1/SAMPLE_RATE, double dr = 0.001, double dtheta = M_PI / 180,
                    unsigned int Nr = GRID_X, unsigned int Ntheta = GRID_Y);
    ~CircularMembrane();

    std::vector<float>& getCurrentGrid() { return u_curr_; }
    std::vector<float>& getAudioBuffer() { return audioBuf_; }
        
    
    void setInitialCondition();
    void Simulate();

private:
    double radius_;   // meters
    double tension_;  // N/m
    double rho_;     // mass density kg/m^2
    double c_;       // wave speed m/s
    double dt_;      // time step s
    double dr_;      // radial step size m
    double dtheta_;  // angular step size radians


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
