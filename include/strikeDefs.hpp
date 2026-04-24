#ifndef STRIKE_DEFS_HPP
#define STRIKE_DEFS_HPP

struct StrikeDefs {
    float amplitude; // normalized strike amplitude (0.0 to 1.0)
    float rPos;    // normalized radial strike position (0.0 center to 1.0 edge)
    float thetaPos; // normalized angular strike position (0 to 2pi)
};

#endif // STRIKE_DEFS_HPP