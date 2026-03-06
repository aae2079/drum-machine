#include "drumRenderer.h"
#include <glad/glad.h>
void DrumRenderer::init(int width, int height){
    width_ = width;
    height_ = height;
    // Initialize OpenGL context, shaders, buffers, etc.
    initSimulation();
    buildMesh();

    glUseProgram(0); // Use default shader for now
    uMVP = glGetUniformLocation(0, "uMVP");

    glEnable(GL_DEPTH_TEST);
    glLineWidth(1.0f);
}

void DrumRenderer::initSimulation(){
    // Initialize simulation parameters, buffers, etc.
    const int N2 = GRID_SIZE * GRID_SIZE;
    inside_.resize(N2);
    for (int ii = 0; ii < GRID_SIZE; ii++){
        for (int jj = 0; jj < GRID_SIZE; jj++){
            float x = -1.0f + ii * DX;
            float y = -1.0f + jj * DX;
            inside_[idx_(ii, jj)] = (x*x + y*y) <= 1.0f; // Inside unit circle
        }
    }
}

void DrumRenderer::buildMesh(){
    const int N = GRID_SIZE;

    vertices_.resize(N*N*6);
    for (int ii = 0 ; ii < N; ii++){
        for (int jj = 0; jj < N; jj++){
            float x = -1.0f + ii * DX;
            float y = -1.0f + jj * DX;
            int base = idx_(ii, jj) * 6;
            vertices_[base + 0] = x;
            vertices_[base + 1] = y;
            vertices_[base + 2] = 0.0f; // z-coordinate
            vertices_[base + 3] = 0.0f; // Normal x
            vertices_[base + 4] = 0.0f; // Normal y
            vertices_[base + 5] = 1.0f; // Normal z
        }
    }

    for (int ii = 0; ii < N - 1; ii++){
        for (int jj = 0; jj < N - 1; jj++){
            unsigned int topLeft = idx_(ii, jj);
            unsigned int topRight = idx_(ii + 1, jj);
            unsigned int bottomLeft = idx_(ii, jj + 1);
            unsigned int bottomRight = idx_(ii + 1, jj + 1);

            if (inside_[topLeft] && inside_[bottomRight] && inside_[topRight] && inside_[bottomLeft]){
                indices_.push_back(topLeft);
                indices_.push_back(topRight);
                indices_.push_back(bottomLeft);
                indices_.push_back(topRight);
                indices_.push_back(bottomLeft);
                indices_.push_back(bottomRight);
            }
        }
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(float), vertices_.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(unsigned int), indices_.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}