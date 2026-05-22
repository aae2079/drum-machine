#include "CylindricalResonator.hpp"

CylindricalResonator::CylindricalResonator() {}
CylindricalResonator::~CylindricalResonator() {
    u_prev_.clear();
    u_curr_.clear();
    u_next_.clear();
}
void CylindricalResonator::init(float radius, float length, float time_step,float speed, int Nr, int Ntheta, int Nz) {
    c_ = speed;
    radius_ = radius;
    length_ = length;
    Nr_ = Nr;
    Ntheta_ = Ntheta;
    Nz_ = Nz;
    dr_ = radius_ / (Nr_ - 1); // radial step size based on radius and number of radial samples
    dtheta_ = 2 * M_PI / Ntheta_; 
    dz_ = length_ / (Nz_ - 1);
    dt_  = time_step;

    // Initialize state vectors based on desired resolution (Nr_, Ntheta_, Nz_)
    u_prev_ = std::vector<float>(Nr_ * Ntheta_ * Nz_, 0.0f);
    u_curr_ = std::vector<float>(Nr_ * Ntheta_ * Nz_, 0.0f);
    u_next_ = std::vector<float>(Nr_ * Ntheta_ * Nz_, 0.0f);

    //set boundary condtions
    // cylindrical wall dp/dr_ = 0 
    // ends of cylinder dp/dz_ = 0
    for (int ii = 0; ii < Nr_; ii++) {
        for (int jj = 0; jj < Ntheta_; jj++) {
            for (int kk = 0; kk < Nz_; kk++) {
                int idx = ii * Ntheta_ * Nz_ + jj * Nz_ + kk;
                if (ii == Nr_ - 1) { // cylindrical wall
                    u_prev_[idx] = 0.0f;
                    u_curr_[idx] = 0.0f;
                    u_next_[idx] = 0.0f;
                }
                if (kk == 0 || kk == Nz_ - 1) { // ends of cylinder
                    u_prev_[idx] = 0.0f;
                    u_curr_[idx] = 0.0f;
                    u_next_[idx] = 0.0f;
                }
            }
        }
    }
}

void CylindricalResonator::resetStateVectors(){
    u_prev_ = std::vector<float>(Nr_ * Ntheta_ * Nz_, 0.0f);
    u_curr_ = std::vector<float>(Nr_ * Ntheta_ * Nz_, 0.0f);
    u_next_ = std::vector<float>(Nr_ * Ntheta_ * Nz_, 0.0f);

    //reset BC
    for (int ii = 0; ii < Nr_; ii++) {
        for (int jj = 0; jj < Ntheta_; jj++) {
            for (int kk = 0; kk < Nz_; kk++) {
                int idx = ii * Ntheta_ * Nz_ + jj * Nz_ + kk;
                if (ii == Nr_ - 1) { // cylindrical wall
                    u_prev_[idx] = 0.0f;
                    u_curr_[idx] = 0.0f;
                    u_next_[idx] = 0.0f;
                }
                if (kk == 0 || kk == Nz_ - 1) { // ends of cylinder
                    u_prev_[idx] = 0.0f;
                    u_curr_[idx] = 0.0f;
                    u_next_[idx] = 0.0f;
                }
            }
        }
    }
}

void CylindricalResonator::setVelocityField(std::vector<float>& v_2d){
    // Set it at the bottom (z=0) of the 3D domain
    for (int ii = 0; ii < Nr_; ii++) {
        for (int jj = 0; jj < Ntheta_; jj++) {
            int idx_2d = ii * Ntheta_ + jj;
            int idx_3d = ii * Ntheta_ * Nz_ + jj * Nz_ + 0;
            u_curr_[idx_3d] = v_2d[idx_2d];
        }
    }
}

std::vector<float> CylindricalResonator::getMembraneBoundaryPressure() {
    // Extract (Nr x Ntheta) slice at z=0
    std::vector<float> p_2d(Nr_ * Ntheta_);
    for (int ii = 0; ii < Nr_; ii++) {
        for (int jj = 0; jj < Ntheta_; jj++) {
            int idx_2d = ii * Ntheta_ + jj;
            int idx_3d = ii * Ntheta_ * Nz_ + jj * Nz_ + 0;
            p_2d[idx_2d] = u_curr_[idx_3d];
        }
    }
    return p_2d;
}

void CylindricalResonator::Simulate(int physSteps) {
    // Simulate resonator response based on current state and inputs
    for (int tt = 0; tt < physSteps; tt++) {
        for (int ii = 1; ii < Nr_ - 1; ii++) {
            float r = ii * dr_;
            for (int jj = 0; jj < Ntheta_; jj++) {
                for (int kk = 1; kk < Nz_ - 1; kk++) {
                    int idx     = ii       * Ntheta_ * Nz_ + jj                       * Nz_ + kk;
                    int idx_ip1 = (ii + 1) * Ntheta_ * Nz_ + jj                       * Nz_ + kk;
                    int idx_im1 = (ii - 1) * Ntheta_ * Nz_ + jj                       * Nz_ + kk;
                    int idx_jp1 = ii       * Ntheta_ * Nz_ + ((jj + 1) % Ntheta_)     * Nz_ + kk;
                    int idx_jm1 = ii       * Ntheta_ * Nz_ + ((jj - 1 + Ntheta_) % Ntheta_) * Nz_ + kk;
                    int idx_kp1 = ii       * Ntheta_ * Nz_ + jj                       * Nz_ + (kk + 1);
                    int idx_km1 = ii       * Ntheta_ * Nz_ + jj                       * Nz_ + (kk - 1);

                    float d2p_dz2     = (u_curr_[idx_kp1] - 2.0f * u_curr_[idx] + u_curr_[idx_km1]) / (dz_ * dz_);
                    float d2p_dtheta2 = (u_curr_[idx_jp1] - 2.0f * u_curr_[idx] + u_curr_[idx_jm1]) / (dtheta_ * dtheta_);

                    float dp_dr_plus  = (u_curr_[idx_ip1] - u_curr_[idx])     / dr_;
                    float dp_dr_minus = (u_curr_[idx]     - u_curr_[idx_im1]) / dr_;
                    float radial_term = (1.0f / r) * (((r + dr_ / 2.0f) * dp_dr_plus - (r - dr_ / 2.0f) * dp_dr_minus) / dr_);

                    float laplacian = radial_term + (1.0f / (r * r)) * d2p_dtheta2 + d2p_dz2;

                    u_next_[idx] = 2.0f * u_curr_[idx] - u_prev_[idx] + (c_ * c_ * dt_ * dt_) * laplacian;
                }
            }
        }
        std::swap(u_prev_, u_curr_);
        std::swap(u_curr_, u_next_);
    }
}