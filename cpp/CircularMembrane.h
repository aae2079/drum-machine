#ifndef DRUM_MACHINE_CIRCULAR_MEMBRANE_H
#define DRUM_MACHINE_CIRCULAR_MEMBRANE_H

#include <vector>
#include <cstddef>
#include <string>

// Skeleton class for a circular membrane model.
// This header declares the public API and minimal data members.
class CircularMembrane {
public:
    // Construct a membrane with given radius (meters) and tension (N/m)
    CircularMembrane();
    CircularMembrane(double radius, double tension, double rho, double c, double dt, double, dr, double dtheta,
                    std::size_t Nr, std::size_t Ntheta);
    ~CircularMembrane();

private:
    double radius_;   // meters
    double tension_;  // N/m
    double rho_;     // mass density kg/m^2
    double c_;       // wave speed m/s
    double dt_;      // time step s
    double dr_;      // radial step size m
    double dtheta_;  // angular step size radians


    // Discretization / storage placeholders
    std::size_t Nr_; // radial samples
    std::size_t Ntheta_; //angular samples

    std::vector<std::vector<double>> u_prev_; // membrane state at previous time step
    std::vector<std::vector<double>> u_curr_; // membrane state at current time step
    std::vector<std::vector<double>> u_next_; // membrane state at next time step

};

#endif // DRUM_MACHINE_CIRCULAR_MEMBRANE_H
