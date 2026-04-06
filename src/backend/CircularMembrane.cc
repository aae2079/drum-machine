// CircularMembrane.cpp
// Implementation skeleton for CircularMembrane.
// No method definitions are provided here — implement them in this file when ready.

#include "CircularMembrane.hpp"

// Intentionally empty: method implementations can be added by the developer.

CircularMembrane::CircularMembrane(double radius, double tension, double rho, double c, double dt, double dr, double dtheta,
                    unsigned int Nr, unsigned int Ntheta) : radius_(radius), tension_(tension), rho_(rho), c_(c), dt_(dt), dr_(dr), dtheta_(dtheta), Nr_(Nr), Ntheta_(Ntheta) {
 
    c_ = std::sqrt(tension_ / rho_);       // wave speed m/s

    // Initialize state vectors
    u_prev_ = std::vector<float>(Nr_ * Ntheta_, 0.0f);
    u_curr_ = std::vector<float>(Nr_ * Ntheta_, 0.0f);
    u_next_ = std::vector<float>(Nr_ * Ntheta_, 0.0f);
    audioBuf_ = std::vector<float>(BUFFER_SIZE, 0.0f);
    histBuf_ = std::vector<float>(OVERLAP, 0.0f);



    // Fix outer boundary: all angular positions at r = Nr_-1
    for (int jj = 0; jj < Ntheta_; jj++) {
        u_curr_[(Nr_ - 1) * Ntheta_ + jj] = 0.0f;
        u_prev_[(Nr_ - 1) * Ntheta_ + jj] = 0.0f;
        u_next_[(Nr_ - 1) * Ntheta_ + jj] = 0.0f;
    }

    setInitialCondition();
}

CircularMembrane::~CircularMembrane() {
    // Destructor implementation (if needed)
    u_prev_.clear();
    u_curr_.clear();
    u_next_.clear();
    audioBuf_.clear();
    histBuf_.clear();
}

void CircularMembrane::setInitialCondition(){
    // Simple Gaussian strike/pluck at center
    float amp = 0.1f;
    for (int ir = 1; ir < Nr_ - 1; ir++) {
        for (int itheta = 0; itheta < Ntheta_; itheta++) {
            // Gaussian centered at r=0, decaying outward radially
            double r_norm = (double)ir / Nr_; // normalized radius 0..1
            float val = (float)(amp * exp(-0.01 * ir * ir));
            u_curr_[ir * Ntheta_ + itheta] = val;
            u_prev_[ir * Ntheta_ + itheta] = val; 
        }
    }

    // Enforce boundary
    for (int itheta = 0; itheta < Ntheta_; itheta++) {
        u_curr_[(Nr_ - 1) * Ntheta_ + itheta] = 0.0f;
        u_prev_[(Nr_ - 1) * Ntheta_ + itheta] = 0.0f;
    }
}

void CircularMembrane::Simulate(){

    std::vector<float> curBuf(BUFFER_SIZE, 0.0f);
    for (int tt = 0; tt < (int)BUFFER_SIZE; tt++) {

        #pragma omp parallel for schedule(static) collapse(2)
        for (int ir = 1; ir < Nr_ - 1; ir++) {
            for (int itheta = 0; itheta < Ntheta_; itheta++) {

                int theta_next = (itheta + 1) % Ntheta_;
                int theta_prev = (itheta - 1 + Ntheta_) % Ntheta_;

                float u_ip1 = u_curr_[(ir + 1) * Ntheta_ + itheta];
                float u_i   = u_curr_[ir       * Ntheta_ + itheta];
                float u_im1 = u_curr_[(ir - 1) * Ntheta_ + itheta];

                double CFL2 = (c_ * dt_ / dr_) * (c_ * dt_ / dr_);

                // Correct polar Laplacian radial part:
                // d²u/dr² + (1/r)(du/dr)
                // = (u[i+1] - 2u[i] + u[i-1])/dr²  +  (u[i+1] - u[i-1])/(2*i*dr²)
                float radial_term = (float)(CFL2 * (
                    u_ip1 - 2.0f * u_i + u_im1          // d²u/dr²
                    + (u_ip1 - u_im1) / (2.0 * ir)        // (1/r)(du/dr), r = ir*dr cancels one dr
                ));

                // Angular term: skip near center to avoid blow-up
                float angular_term = 0.0f;
                if (ir > 3) {
                    angular_term = (float)((c_ * dt_) * (c_ * dt_) /
                                ((double)ir * ir * dr_ * dr_ * dtheta_ * dtheta_) * (
                        u_curr_[ir * Ntheta_ + theta_next]
                        + u_curr_[ir * Ntheta_ + theta_prev]
                        - 2.0f * u_i
                    ));
                }

                float val = 2.0f * u_i
                        - u_prev_[ir * Ntheta_ + itheta]
                        + radial_term
                        + angular_term;

                // Clamp to prevent NaN propagation
                u_next_[ir * Ntheta_ + itheta] = std::max(-1.0f, std::min(1.0f, val));
            }
        }

        // Center singularity
        double center_avg = 0.0;
        for (int itheta = 0; itheta < Ntheta_; itheta++)
            center_avg += u_curr_[1 * Ntheta_ + itheta];
        center_avg /= Ntheta_;

        float center_val = (float)(2.0 * u_curr_[0]
                                - u_prev_[0]
                                + 4.0 * (c_ * dt_ / dr_) * (c_ * dt_ / dr_)
                                * (center_avg - u_curr_[0]));
        center_val = std::max(-1.0f, std::min(1.0f, center_val));

        for (int itheta = 0; itheta < Ntheta_; itheta++)
            u_next_[0 * Ntheta_ + itheta] = center_val;

        // Outer boundary
        for (int itheta = 0; itheta < Ntheta_; itheta++)
            u_next_[(Nr_ - 1) * Ntheta_ + itheta] = 0.0f;

        std::swap(u_prev_, u_curr_);
        std::swap(u_curr_, u_next_);

        int r_mic = (int)(Nr_ * 2.0 / 3.0);
        double sample = 0.0;
        for (int itheta = 0; itheta < Ntheta_; itheta++)
            sample += u_curr_[r_mic * Ntheta_ + itheta];
        curBuf[tt] = 15.0f * (float)(sample / Ntheta_);
    }

    if (firstTime){
        audioBuf_ = curBuf;
        //copy curBuf to histBuf_ for next chunk
        std::copy(curBuf.end() - (int)OVERLAP, curBuf.end(), histBuf_.begin());
        firstTime = 0;
    } else {
        // For subsequent chunks, concatenate histBuf_ and curBuf to audioBuf_
        std::copy(histBuf_.begin(), histBuf_.end(), audioBuf_.begin());
        std::copy(curBuf.begin(), curBuf.end(), audioBuf_.begin() + (int)OVERLAP);
        // Update histBuf_ for next chunk
        std::copy(curBuf.end() - (int)OVERLAP, curBuf.end(), histBuf_.begin());
    }
}