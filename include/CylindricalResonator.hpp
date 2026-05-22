#ifndef DRUM_MACHINE_CYLINDRICAL_RESONATOR_HPP
#define DRUM_MACHINE_CYLINDRICAL_RESONATOR_HPP

#include <vector>
#include <cstddef>
#include <string>
#include <cmath>
#include "CircularMembrane.hpp"

class CylindricalResonator {
public:
    CylindricalResonator();
    ~CylindricalResonator();

    void init(float radius, float length, float time_step, float speed, int Nr, int Ntheta, int Nz);

    //getrs and setrs
    void setVelocityField(std::vector<float>& v_2d);
    std::vector<float> getMembraneBoundaryPressure();
    void resetStateVectors();

    void Simulate(int physSteps);

private:
    float radius_;
    float c_;
    float dt_;
    float dr_;
    float dtheta_;
    float dz_;

    unsigned int Nr_; // radial samples
    unsigned int Ntheta_; // angular samples
    unsigned int Nz_; // axial samples

    float length_; // length of the resonator in meters
    std::vector<float> u_curr_; // current state of the resonator
    std::vector<float> u_prev_; // previous state of the resonator
    std::vector<float> u_next_; // next state of the resonator

    std::vector<float> velocityInput_; // Placeholder for velocity field data
    std::vector<float> pressureOutput_; // Placeholder for pressure output data


};

#endif // DRUM_MACHINE_CYLINDRICAL_RESONATOR_HPP