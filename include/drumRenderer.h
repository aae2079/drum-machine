#include <vector>
#include<string>
#include "RectangularMembrane.h"

typedef unsigned int GLuint;

class DrumRenderer {
    public:
        void init(int width, int height);
        void cleanup();
        void buildMesh();
        void initSimulation();
        void render();
        
    private:
        void compileShaders();
        void updateMeshDeformation();
        
        int width_;
        int height_;
        int gridSize_;
        std::vector<bool> inside_;
        std::vector<float> displacements_;

        GLuint vao{0}, vbo{0}, ebo{0};
        GLuint shaderProgram{0};
        GLuint uMVP{0};

        std::vector<unsigned int> indices_;
        std::vector<float> vertices_;
        RectangularMembrane membrane_;
        
        int frameCount_{0};
        
        inline int idx_(int i, int j) { return i * gridSize_ + j; }
};