#ifndef RECTANGULAR_MEMBRANE_H
#define RECTANGULAR_MEMBRANE_H


#include "simDefs.hpp"
#include <vector>
#include <cmath>
#include <omp.h>


/*
Class that generates grid and simulates 2d rectangular membrane vibrations
using finite difference method to solve the wave equation.

*/
class RectangularMembrane {
    public:
        RectangularMembrane(int nx=GRID_R, int ny=GRID_TH, float damp = 10.0, float c = 1.0, float time_step = 1.0/SAMPLE_RATE, float sim_time = 1.0);
        void setInitialCondition();
        void Simulate();
        ~RectangularMembrane();
        
        // Getters for visualization
        int getNx() const { return nx_; }
        int getNy() const { return ny_; }
        std::vector<float>& getCurrentGrid() { return curr_; }
        std::vector<float>& getAudioBuffer() { return audioBuf_; }
        
    private:

        //Grid Dimensions
        int nx_;
        int ny_;

        float damp_; //Used to simulate energy loss
        float c_; //Wave speed - Reminder to test what speed is best for audio

        float time_step_;

        float sim_time_; //Simulation time

        std::vector<float> curr_;      // Current displacement
        std::vector<float> prev_;  // Previous displacement
        std::vector<float> next_;  // Next displacement

        std::vector<float> audioBuf_; // Buffer to store audio output
        std::vector<float> histBuf_; // Buffer to store historical audio output for streaming

        int num_samples_;

        int firstTime = 1; // Flag to indicate if it's the first time step

};

#endif // RECTANGULAR_MEMBRANE_H