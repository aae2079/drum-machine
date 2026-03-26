#include "drumRenderer.hpp"
#include <glad/glad.h>
#include <iostream>
#include <cmath>

using namespace std;

void DrumRenderer::init(int width, int height){
    width_ = width;
    height_ = height;
    
    std::cout << "DrumRenderer::init() called" << std::endl;
    std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    
    // Initialize OpenGL context, shaders, buffers, etc.
    initSimulation();
    compileShaders();
    buildMesh();

    glUseProgram(shaderProgram);
    uMVP = glGetUniformLocation(shaderProgram, "uMVP");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glLineWidth(2.0f);
    
    std::cout << "DrumRenderer::init() completed" << std::endl;
}

void DrumRenderer::compileShaders(){
    const char* vertexShaderSource = R"(
        #version 150
        in vec3 position;
        in vec3 normal;
        uniform mat4 uMVP;
        out vec3 FragNormal;
        void main() {
            gl_Position = uMVP * vec4(position, 1.0);
            FragNormal = normal;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 150
        in vec3 FragNormal;
        out vec4 FragColor;
        void main() {
            vec3 lightDir = normalize(vec3(0.5, 0.5, 1.0));
            float diffuse = max(dot(FragNormal, lightDir), 0.3);
            FragColor = vec4(0.3, 0.7, 1.0, 1.0) * diffuse;
        }
    )";

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    
    glBindAttribLocation(shaderProgram, 0, "position");
    glBindAttribLocation(shaderProgram, 1, "normal");
    
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Program linking failed: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void DrumRenderer::initSimulation(){
    // Get grid dimensions from membrane
    gridSize_ = membrane_.getNx();
    
    const int N2 = gridSize_ * gridSize_;
    inside_.resize(N2);
    displacements_.resize(N2, 0.0f);
    
    float span = 2.0f; // -1 to 1
    float dx = span / (gridSize_ - 1);
    
    for (int ii = 0; ii < gridSize_; ii++){
        for (int jj = 0; jj < gridSize_; jj++){
            float x = -1.0f + ii * dx;
            float y = -1.0f + jj * dx;
            inside_[idx_(ii, jj)] = (x*x + y*y) <= 1.0f; // Inside unit circle
        }
    }
    
    // Initialize and run the membrane simulation
    std::cout << "Initializing membrane with grid size: " << gridSize_ << "x" << gridSize_ << std::endl;
    membrane_.setInitialCondition();
    membrane_.Simulate();
    std::cout << "Membrane simulation completed" << std::endl;
}

void DrumRenderer::buildMesh(){
    const int N = gridSize_;

    vertices_.resize(N*N*6);
    float span = 2.0f;
    float dx = span / (N - 1);
    
    for (int ii = 0 ; ii < N; ii++){
        for (int jj = 0; jj < N; jj++){
            float x = -1.0f + ii * dx;
            float y = -1.0f + jj * dx;
            int base = idx_(ii, jj) * 6;
            vertices_[base + 0] = x;
            vertices_[base + 1] = y;
            vertices_[base + 2] = 0.0f; // z-coordinate
            vertices_[base + 3] = 0.0f; // Normal x
            vertices_[base + 4] = 0.0f; // Normal y
            vertices_[base + 5] = 1.0f; // Normal z
        }
    }

    int triangleCount = 0;
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
                triangleCount++;
            }
        }
    }

    std::cout << "Built mesh with " << triangleCount << " triangles (" << indices_.size() << " indices) from " << vertices_.size()/6 << " vertices" << std::endl;
    if (triangleCount == 0) {
        std::cerr << "WARNING: No triangles generated! Check inside_ mask" << std::endl;
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

void DrumRenderer::updateMeshDeformation(){
    // Scale factor for displacement visualization
    const float SCALE = 0.2f;
    
    const auto& grid = membrane_.getCurrentGrid();
    
    // Update vertex positions based on membrane grid
    for (int ii = 0; ii < gridSize_; ii++){
        for (int jj = 0; jj < gridSize_; jj++){
            int idx = idx_(ii, jj);
            if (inside_[idx]) {
                int vertexBase = idx * 6;
                // Update z with displacement from membrane
                vertices_[vertexBase + 2] = grid[idx] * SCALE;
            }
        }
    }
    
    // Recalculate normals based on surface deformation
    float span = 2.0f;
    float dx = span / (gridSize_ - 1);
    
    for (int ii = 1; ii < gridSize_ - 1; ii++){
        for (int jj = 1; jj < gridSize_ - 1; jj++){
            int idx = idx_(ii, jj);
            if (inside_[idx]) {
                // Calculate normal using finite differences
                float dz_dx = (vertices_[idx_(ii+1, jj) * 6 + 2] - vertices_[idx_(ii-1, jj) * 6 + 2]) / (2.0f * dx);
                float dz_dy = (vertices_[idx_(ii, jj+1) * 6 + 2] - vertices_[idx_(ii, jj-1) * 6 + 2]) / (2.0f * dx);
                
                // Normal is (-dz/dx, -dz/dy, 1)
                float nx = -dz_dx;
                float ny = -dz_dy;
                float nz = 1.0f;
                
                float len = sqrt(nx*nx + ny*ny + nz*nz);
                if (len > 0.001f) {
                    nx /= len;
                    ny /= len;
                    nz /= len;
                }
                
                int vertexBase = idx * 6;
                vertices_[vertexBase + 3] = nx;
                vertices_[vertexBase + 4] = ny;
                vertices_[vertexBase + 5] = nz;
            }
        }
    }
    
    // Update VBO with new vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_.size() * sizeof(float), vertices_.data());
}

void DrumRenderer::render(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width_, height_);

    glUseProgram(shaderProgram);

    // Update mesh deformation from current membrane state
    updateMeshDeformation();

    // Simple orthographic projection
    float aspect = (float)width_ / (float)height_;
    float scale = 0.7f;
    float mvp[16] = {
        scale/aspect, 0,     0,       0,
        0,            scale, 0,       0,
        0,            0,     -0.01f,  0,
        0,            0,     -1.5f,   1
    };

    glUniformMatrix4fv(uMVP, 1, GL_FALSE, mvp);

    glBindVertexArray(vao);
    if (indices_.size() > 0) {
        // Draw as wireframe to debug
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    } else {
        std::cerr << "ERROR: No indices to draw!" << std::endl;
    }
    glBindVertexArray(0);
    
    frameCount_++;
}

void DrumRenderer::cleanup(){
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shaderProgram);
}