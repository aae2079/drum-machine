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


void CircularMembrane::setInitialCondition(const StrikeDefs* strike){
    // Convert strike position to index-space Cartesian coords so r and theta
    // distances are in the same units before computing the Gaussian.
    float r_s = strike->rPos * (Nr_ - 1);
    float xs  = r_s * std::cos(strike->thetaPos);
    float zs  = r_s * std::sin(strike->thetaPos);

    #pragma omp parallel for schedule(static) collapse(2)
    for (int ir = 1; ir < Nr_ - 1; ir++) {
        for (int itheta = 0; itheta < Ntheta_; itheta++) {
            float xi    = ir * std::cos(itheta * dtheta_);
            float zi    = ir * std::sin(itheta * dtheta_);
            float dist2 = (xi - xs)*(xi - xs) + (zi - zs)*(zi - zs);
            // sigma=0.08: ~14% amplitude at 5 ring-units, ~0% at 10 ring-units
            float val   = strike->amplitude * std::exp(-0.08f * dist2);
            u_curr_[ir * Ntheta_ + itheta] = val;
            u_prev_[ir * Ntheta_ + itheta] = val;
        }
    }
}

void CircularMembrane::Simulate(){
    // Run exactly enough physics steps to cover one audio buffer's worth of time.
    // sampleInterp() on simBuf_ will then produce exactly BUFFER_SIZE audio samples.
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
                // Skip angular term where 1/r² makes the Courant number > 1.
                // Stability requires: ii * dtheta_ >= CFL  (derived from c*dt/(ir*dr*dtheta) <= 1).
                // Near-origin rings are effectively symmetric from the origin averaging condition.
                float term_theta = 0.0f;
                if ((float)ii * dtheta_ >= CFL) {
                    float d2u_dtheta2 = (u_curr_[ii * Ntheta_ + j_plus] - 2.0 * u_curr_[ii * Ntheta_ + jj] + u_curr_[ii * Ntheta_ + j_minus]) / (dtheta_ * dtheta_);
                    term_theta = d2u_dtheta2 / (r * r);
                }
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


