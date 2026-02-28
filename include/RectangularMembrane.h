#ifndef RECTANGULAR_MEMBRANE_H
#define RECTANGULAR_MEMBRANE_H


#include "audioDefs.h"
#include <vector>
#include <cmath>

#define CFL 0.25  // Courant-Friedrichs-Lewy condition for stability
/*
Class that generates grid and simulates 2d rectangular membrane vibrations
using finite difference method to solve the wave equation.

*/
class RectangularMembrane {
    public:
        RectangularMembrane(int nx, int ny, float damp, float c, float time_step, float sim_time);
        RectangularMembrane();
        void setInitialCondition();
        void Simulate(std::vector<float>& output_buffer);
        ~RectangularMembrane();
        
    private:

        //Grid Dimensions
        int nx_;
        int ny_;

        float damp_; //Used to simulate energy loss
        float c_; //Wave speed - Reminder to test what speed is best for audio

        float time_step_;

        float sim_time_; //Simulation time

        std::vector<std::vector<float>> curr_;      // Current displacement
        std::vector<std::vector<float>> prev_;  // Previous displacement
        std::vector<std::vector<float>> next_;  // Next displacement

        int num_samples_;

};

#endif // RECTANGULAR_MEMBRANE_H