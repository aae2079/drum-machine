#ifndef STRIKE_DEFS_HPP
#define STRIKE_DEFS_HPP


/*

amplitude: normalized strike amplitude (0.0 to 1.0)
rPos: normalized radial strike position (0.0 center to 1.0 edge)
thetaPos: normalized angular strike position (0 to 2pi)


*/
struct StrikeDefs {
    float amplitude;
    float rPos;   
    float thetaPos; 
};

#endif // STRIKE_DEFS_HPP