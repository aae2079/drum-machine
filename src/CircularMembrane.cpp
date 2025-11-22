// CircularMembrane.cpp
// Implementation skeleton for CircularMembrane.
// No method definitions are provided here — implement them in this file when ready.

#include "CircularMembrane.h"

// Intentionally empty: method implementations can be added by the developer.

CircularMembrane::CircularMembrane(){
    radius_ = 0.1;   // meters
    tension_ = 100.0;  // N/m
    rho_ = 0.01;     // mass density kg/m^2
    c_ = sqrt(tension_ / rho_);       // wave speed m/s
    dt_ = 0.0001;      // time step s
    dr_ = 0.001;      // radial step size m
    dtheta_ = M_PI / 180;  // angular step size radians
    Nr_ = 200;
    Ntheta_ = 1250;

    u_prev_.resize(Nr_, std::vector<double>(Ntheta_, 0.0));
    u_curr_.resize(Nr_, std::vector<double>(Ntheta_, 0.0));
    u_next_.resize(Nr_, std::vector<double>(Ntheta_, 0.0));

    for (int ii = 0; ii < Nr_; ii++){
        for (int jj = 0; jj < Ntheta_; jj++){
            r = ii * dr_;

            if (r == radius_){
                // Fixed Boundary Condition
                u_next_[ii][jj] = 0.0;
            }

            if (r == 0){
                /*****************************************/
                /*      Special Singularity Formula      */
                /*                                       */
                /*    __ 2        4(u1,j - u0,j)         */
                /*    \/  u(0) ~ ----------------        */
                /*                     /\r^2             */
                /*                                       */
                /*****************************************/

                u_next_[0][jj]  = 2
            }
        }
    }

    
}

CircularMembrane(double radius, double tension, double rho, double c, double dt, double, dr, double dtheta,
                    std::size_t Nr, std::size_t Ntheta) : radius_(radius), tension_(tension), rho_(rho), c_(c), dt_(dt), dr_(dr), dtheta_(dtheta), Nr_(Nr), Ntheta_(Ntheta) {
    // Constructor body can be empty if all initialization is done in the initializer list
}