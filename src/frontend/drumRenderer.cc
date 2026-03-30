#include "drumRenderer.hpp"
#include <glad/glad.h>
#include <iostream>
#include <cmath>

using namespace std;

DrumRenderer::DrumRenderer(int nx, int ny, float damp, float c, float time_step, float sim_time, uint64_t wWidth, uint64_t wHeight)
            :RectangularMembrane(int nx, int ny, float damp, float c, float time_step, float sim_time),WIDTH(wWidth),HEIGHT(wHeight){

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


        GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Drum Machine", NULL, NULL);
        if (!window) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return -1;
        }

        glfwMakeContextCurrent(window);
        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

        glViewport(0,0, WIDTH, HEIGHT);

        buildMesh();

}

void DrumRenderer::compileShaders(const char *vertexFile, const char *fragmentFile)
                    : vertexFile_(vertexFile), fragmentFile_(fragmentFile){
    // Read vertexFile and fragmentFile and store the strings
	std::string vertexCode = getShaderContents(vertexFile_);
	std::string fragmentCode = getShaderConents(fragmentFile_);

	// Convert the shader source strings into character arrays
	const char* vertexSource = vertexCode.c_str();
	const char* fragmentSource = fragmentCode.c_str();

	// Create Vertex Shader Object and get its reference
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	// Attach Vertex Shader source to the Vertex Shader Object
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	// Compile the Vertex Shader into machine code
	glCompileShader(vertexShader);
	// Checks if Shader compiled succesfully
	compileErrors(vertexShader, "VERTEX");

	// Create Fragment Shader Object and get its reference
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	// Attach Fragment Shader source to the Fragment Shader Object
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	// Compile the Vertex Shader into machine code
	glCompileShader(fragmentShader);
	// Checks if Shader compiled succesfully
	compileErrors(fragmentShader, "FRAGMENT");

	// Create Shader Program Object and get its reference
	ID = glCreateProgram();
	// Attach the Vertex and Fragment Shaders to the Shader Program
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);
	// Wrap-up/Link all the shaders together into the Shader Program
	glLinkProgram(ID);
	// Checks if Shaders linked succesfully
	compileErrors(ID, "PROGRAM");

	// Delete the now useless Vertex and Fragment Shader objects
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

// Reads a text file and outputs a string with everything in the text file
std::string DrumRenderer::getShaderContents(const char* filename)
{
	std::ifstream in(filename);
	if (in)
	{
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return(contents);
	}
	throw(errno);
}

void DrumRendere::buildMesh(){
	for(int i = 0; i < GRID_X; i++){
		for (int j = 0; j < GRID_Y; j++){
			float x = (float)j / (GRID_X - 1) * 2.0f - 1.0f; 
			float z = (float)i / (GRID_Y - 1) * 2.0f - 1.0f;
			float y = 0.0f; 

			// position
			vertices_.push_back(x);
			vertices_.push_back(y);
			vertices_.push_back(z);
			// color
			vertices_.push_back(0.0f);
			vertices_.push_back(0.0f);
			vertices_.push_back(0.0f);
			// TexCoord
			vertices_.push_back((float)j / (GRID_Y - 1));
			vertices_.push_back((float)i / (GRID_X - 1));
			// normal
			vertices_.push_back(0.0f);
			vertices_.push_back(1.0f);
			vertices_.push_back(0.0f);
		}
	}

	// Generate indices_
	for (int i = 0; i < GRID_X - 1; i++) {
		for (int j = 0; j < GRID_Y - 1; j++) {
			int topLeft     = i * GRID_X + j;
			int topRight    = i * GRID_X + j + 1;
			int bottomLeft  = (i + 1) * GRID_X + j;
			int bottomRight = (i + 1) * GRID_X + j + 1;

			// triangle 1
			indices_.push_back(topLeft);
			indices_.push_back(bottomLeft);
			indices_.push_back(topRight);
			// triangle 2
			indices_.push_back(topRight);
			indices_.push_back(bottomLeft);
			indices_.push_back(bottomRight);
		}
	}
};
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
                vertices__[vertexBase + 2] = grid[idx] * SCALE;
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
                float dz_dx = (vertices__[idx_(ii+1, jj) * 6 + 2] - vertices__[idx_(ii-1, jj) * 6 + 2]) / (2.0f * dx);
                float dz_dy = (vertices__[idx_(ii, jj+1) * 6 + 2] - vertices__[idx_(ii, jj-1) * 6 + 2]) / (2.0f * dx);
                
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
                vertices__[vertexBase + 3] = nx;
                vertices__[vertexBase + 4] = ny;
                vertices__[vertexBase + 5] = nz;
            }
        }
    }
    
    // Update VBO with new vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices__.size() * sizeof(float), vertices__.data());
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
    if (indices__.size() > 0) {
        // Draw as wireframe to debug
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, indices__.size(), GL_UNSIGNED_INT, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    } else {
        std::cerr << "ERROR: No indices_ to draw!" << std::endl;
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