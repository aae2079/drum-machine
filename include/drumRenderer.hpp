#ifndef DRUM_RENDERER_HPP
#define DRUM_RENDERER_HPP

#include "RectangularMembrane.hpp"

class DrumRenderer:RectangularMembrane{
    public:
        DrumRenderer(int nx, int ny, float damp, float c, float time_step, float sim_time, uint64_t wWidth, uint64_t wHeight);
        void compileShaders(const char* vertexFile, const char* fragmentFile);
    private:
        uint64_t WIDTH;
        uint64_t HEIGHT;
        void buildMesh();

        const char* vertexFile_;
        const char* fragmentFile_;
        std::string getShaderContents(const char* filename);
        
    }


#endif