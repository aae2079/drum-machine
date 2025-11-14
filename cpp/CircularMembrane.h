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
    CircularMembrane(double radius = 0.1, double tension = 100.0);
    ~CircularMembrane();

private:
    double radius_;   // meters
    double tension_;  // N/m

    // Discretization / storage placeholders
    std::size_t radialSamples_;
    std::size_t angularSamples_;
    std::vector<double> modeData_; // placeholder for computed data
};

#endif // DRUM_MACHINE_CIRCULAR_MEMBRANE_H
