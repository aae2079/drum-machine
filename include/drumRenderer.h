#include <vector>
#include<string>

typedef unsigned int GLuint;

#define GRID_SIZE 201
#define DX 2.0f / (GRID_SIZE - 1) // Spatial step size

class DrumRenderer {
    public:
        void init(int width, int height);
        void cleanup();
        void buildMesh();
        void initSimulation();
        
    private:
        int width_;
        int height_;
        std::vector<bool> inside_;

        GLuint vao{0}, vbo{0}, ebo{0};
        GLuint uMVP{0};

        std::vector<unsigned int> indices_;
        std::vector<float> vertices_;
        inline int idx_(int i, int j) { return i * GRID_SIZE + j; }
};