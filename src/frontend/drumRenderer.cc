#include "drumRenderer.hpp"
#include <glad/glad.h>
#include <iostream>
#include <cmath>

using namespace std;

DrumRenderer::DrumRenderer(uint64_t wWidth, uint64_t wHeight, const char* windowTitle)
        : WIDTH(wWidth), HEIGHT(wHeight){


}

bool DrumRenderer::init(){
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    window = glfwCreateWindow(WIDTH, HEIGHT, "Drum Machine", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glViewport(0,0, WIDTH, HEIGHT);

    buildMesh();

    return true;
}

void buildMesh(){
    for(int i = 0; i < GRID_X; i++){
		for (int j = 0; j < GRID_Y; j++){
			float x = (float)j / (GRID_X - 1) * 2.0f - 1.0f; 
			float z = (float)i / (GRID_Y - 1) * 2.0f - 1.0f;
			float y = 0.0f; 

			// position
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);
			// color
			vertices.push_back(0.0f);
			vertices.push_back(0.0f);
			vertices.push_back(0.0f);
			// TexCoord
			vertices.push_back((float)j / (GRID_Y - 1));
			vertices.push_back((float)i / (GRID_X - 1));
			// normal
			vertices.push_back(0.0f);
			vertices.push_back(1.0f);
			vertices.push_back(0.0f);
		}
	}

	// Generate indices
	for (int i = 0; i < GRID_X - 1; i++) {
		for (int j = 0; j < GRID_Y - 1; j++) {
			int topLeft     = i * GRID_X + j;
			int topRight    = i * GRID_X + j + 1;
			int bottomLeft  = (i + 1) * GRID_X + j;
			int bottomRight = (i + 1) * GRID_X + j + 1;

			// triangle 1
			indices.push_back(topLeft);
			indices.push_back(bottomLeft);
			indices.push_back(topRight);
			// triangle 2
			indices.push_back(topRight);
			indices.push_back(bottomLeft);
			indices.push_back(bottomRight);
		}
	}

    createBuffers(vertices.data(), vertices.size() * sizeof(GLfloat),
                  indices.data(), indices.size() * sizeof(GLuint));
    setupVertexAttributes();
}


bool DrumRenderer::shouldClose() const
{
    return glfwWindowShouldClose(window);
}

void DrumRenderer::compileShaders(const char *vertexFile, const char *fragmentFile){
    // Read vertexFile and fragmentFile and store the strings
	std::string vertexCode = getShaderContents(vertexFile);
	std::string fragmentCode = getShaderContents(fragmentFile);

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

void DrumRenderer::compileErrors(unsigned int shader, const char* type)
{
    GLint hasCompiled;
    char infoLog[1024];

    if (type != std::string("PROGRAM"))
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
        if (hasCompiled == GL_FALSE)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "SHADER_COMPILATION_ERROR for: " << type << "\n" << infoLog << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
        if (hasCompiled == GL_FALSE)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "SHADER_LINKING_ERROR for: " << type << "\n" << infoLog << std::endl;
        }
    }
}

// Shader activation and deletion
void DrumRenderer::activateShaderProgram()
{
    glUseProgram(shaderProgramID);
}

void DrumRenderer::deleteShaderProgram()
{
    if (shaderProgramID != 0)
    {
        glDeleteProgram(shaderProgramID);
        shaderProgramID = 0;
    }
}

GLuint DrumRenderer::getShaderProgramID() const
{
    return shaderProgramID;
}

void DrumRenderer::compileErrors(unsigned int shader, const char* type){
	// Stores status of compilation
	GLint hasCompiled;
	// Character array to store error message in
	char infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_COMPILATION_ERROR for:" << type << "\n" << infoLog << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_LINKING_ERROR for:" << type << "\n" << infoLog << std::endl;
		}
	}
}

// Buffer creation
void Renderer::createBuffers(GLfloat* vertexData, GLsizeiptr vertexSize, GLuint* indexData, GLsizeiptr indexSize)
{
    // Create VAO
    glGenVertexArrays(1, &VAO_ID);
    bindVertexArray();

    // Create VBO
    glGenBuffers(1, &VBO_ID);
    bindVBO();
    glBufferData(GL_ARRAY_BUFFER, vertexSize, vertexData, GL_DYNAMIC_DRAW);

    // Create EBO
    glGenBuffers(1, &EBO_ID);
    bindEBO();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize, indexData, GL_STATIC_DRAW);

    unbindVBO();
    unbindVertexArray();
}

// Setup vertex attributes
void Renderer::setupVertexAttributes()
{
    bindVertexArray();
    bindVBO();

    // Position attribute (layout 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute (layout 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // TexCoord attribute (layout 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Normal attribute (layout 3)
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    unbindVBO();
    unbindVertexArray();
}

// Bind and unbind buffer methods
void Renderer::bindVertexArray()
{
    glBindVertexArray(vao);
}

void Renderer::unbindVertexArray()
{
    glBindVertexArray(0);
}

void Renderer::bindVBO()
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
}

void Renderer::unbindVBO()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::bindEBO()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
}

void Renderer::unbindEBO()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
