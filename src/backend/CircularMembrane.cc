// CircularMembrane.cpp
// Implementation skeleton for CircularMembrane.
// No method definitions are provided here — implement them in this file when ready.

#include "CircularMembrane.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>
// Intentionally empty: method implementations can be added by the developer.

CircularMembrane::CircularMembrane() {}

CircularMembrane::~CircularMembrane() {
    // Destructor implementation (if needed)
    u_prev_.clear();
    u_curr_.clear();
    u_next_.clear();
    simBuf_.clear();
}
void CircularMembrane::init(float radius, float tension, float rho_density, unsigned int Nr, unsigned int Ntheta){
    radius_ = radius;
    tension_ = tension;
    rho_ = rho_density;
    Nr_ = Nr;
    Ntheta_ = Ntheta;

    
    dr_ = radius_ / (Nr_ - 1); // radial step size based on radius and number of radial samples
    dtheta_ = 2 * M_PI / Ntheta_; // angular step size based on number of angular samples
    c_ = std::sqrt(tension_ / rho_);       // wave speed m/s
    dt_ = CFL * dr_ / c_; // time step based on CFL condition for stability
    simRate_ = 1.0f / dt_; // simulation sample rate in Hz
    // Initialize state vectors
    u_prev_ = std::vector<float>(Nr_ * Ntheta_, 0.0f);
    u_curr_ = std::vector<float>(Nr_ * Ntheta_, 0.0f);
    u_next_ = std::vector<float>(Nr_ * Ntheta_, 0.0f);
    physSteps_ = std::max(1, (int)std::ceil((double)BUFFER_SIZE * simRate_ / SAMPLE_RATE));
    simBuf_ = std::vector<float>(physSteps_, 0.0f);


    // Dirichlet fixed outer boundary: all angular positions at r = Nr_-1
    for (int jj = 0; jj < Ntheta_; jj++) {
        u_curr_[(Nr_ - 1) * Ntheta_ + jj] = 0.0f;
        u_prev_[(Nr_ - 1) * Ntheta_ + jj] = 0.0f;
        u_next_[(Nr_ - 1) * Ntheta_ + jj] = 0.0f;
    }
}

void CircularMembrane::cleanup() {
    // Clear vectors to free memory
        u_prev_.clear();
        u_curr_.clear();
    u_next_.clear();
    simBuf_.clear();
}

void CircularMembrane::setInitialCondition(){
    // Simple Gaussian strike/pluck at center
    float amp = 0.1f;

    #pragma omp parallel for schedule(static) collapse(2)
    for (int ir = 1; ir < Nr_ - 1; ir++) {
        for (int itheta = 0; itheta < Ntheta_; itheta++) {
            // Gaussian centered at r=0, decaying outward radially
            double r_norm = (double)ir / Nr_; // normalized radius 0..1
            float val = (float)(amp * exp(-0.01 * ir * ir));
            u_curr_[ir * Ntheta_ + itheta] = val;
            u_prev_[ir * Ntheta_ + itheta] = val; 
        }
    }
}

void CircularMembrane::Simulate(){
    // Run exactly enough physics steps to cover one audio buffer's worth of time.
    // upSample() on simBuf_ will then produce exactly BUFFER_SIZE audio samples.
    std::vector<float> curBuf(physSteps_, 0.0f);
    for(int tt = 0; tt < physSteps_; tt++){
        // --- spatial update ----
        #pragma omp parallel for schedule(static)
        for (int ii = 1; ii < Nr_ - 1; ii++){
            float r = ii * dr_;
            for (int jj = 0; jj < Ntheta_; jj++){
                int j_plus = (jj + 1) % Ntheta_;
                int j_minus = (jj - 1 + Ntheta_) % Ntheta_;

                //d2u/dr2
                float d2u_dr2 = (u_curr_[(ii + 1) * Ntheta_ + jj] - 2.0 * u_curr_[ii * Ntheta_ + jj] + u_curr_[(ii - 1) * Ntheta_ + jj]) / (dr_ * dr_);
                
                //(1/r) * du/dr
                float du_dr = (u_curr_[(ii + 1) * Ntheta_ + jj] - u_curr_[(ii - 1) * Ntheta_ + jj]) / (2.0 * dr_);
                float term_r = du_dr / r;

                // (1/r^2) * d2u/dtheta2
                float d2u_dtheta2 = (u_curr_[ii * Ntheta_ + j_plus] - 2.0 * u_curr_[ii * Ntheta_ + jj] + u_curr_[ii * Ntheta_ + j_minus]) / (dtheta_ * dtheta_);
                float term_theta = d2u_dtheta2 / (r * r);
                float laplacian = d2u_dr2 + term_r + term_theta;

                u_next_[ii * Ntheta_ + jj] = 2.0f * u_curr_[ii * Ntheta_ + jj] - u_prev_[ii * Ntheta_ + jj] + (c_ * c_ * dt_ * dt_) * laplacian;
            }
        }

        //enforce Dirilechlet
        for (int jj = 0; jj < Ntheta_; jj++) {
            u_next_[(Nr_ - 1) * Ntheta_ + jj] = 0.0f;
        }

        //origin singularity (r = 0): i will average over all theta neighbors to enforce symmetry
        double avg = 0.0;
        for (int jj = 0; jj < Ntheta_;jj++){
            avg += u_curr_[1 * Ntheta_ + jj];
        }
        avg /= Ntheta_;
        for (int jj = 0; jj < Ntheta_; jj++){
            u_next_[0 * Ntheta_ + jj] = avg;
        }
        //sample audio at center of membrane
        curBuf[tt] = 15.0f * u_curr_[0]; // center point r=0, all theta the same
        //advance  simulation
        std::swap(u_prev_, u_curr_);
        std::swap(u_curr_, u_next_);
   

        if (std::isnan(u_next_[Nr_/2 * Ntheta_]) || std::isinf(u_next_[Nr_/2 * Ntheta_])) {
            std::cerr << "BLOW UP at t=" << tt << std::endl;
            break;
        }

 
    }
    simBuf_ = std::move(curBuf);
}

std::vector<float> CircularMembrane::sampleInterp(float *in, int inLen, float inFs, float outFs){
    if (inLen <= 0 || inFs <= 0.0f || outFs <= 0.0f) return {};

    // Output length = ceil(inLen * outFs / inFs).
    // Caller is responsible for allocating at least that many floats.
    int outLen = (int)std::ceil((double)inLen * outFs / inFs);
    std::vector<float> out(outLen, 0.0f);

    // Step size in input-sample coordinates per output sample.
    // < 1.0 when upsampling (outFs > inFs), > 1.0 when downsampling.
    double step = (double)inFs / outFs;

    for (int n = 0; n < outLen; n++) {
        double pos = n * step;          // fractional index into in[]
        int    i0  = (int)pos;          // left neighbour
        int    i1  = i0 + 1;            // right neighbour
        double alpha = pos - i0;        // weight for i1, in [0, 1)

        // Clamp so we never read past the end of the input.
        i0 = std::min(i0, inLen - 1);
        i1 = std::min(i1, inLen - 1);

        out[n] = (float)((1.0 - alpha) * in[i0] + alpha * in[i1]);
    }

    return out;
}