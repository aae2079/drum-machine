#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <cerrno>

class Renderer
{
public:
    // Constructor and Destructor
    Renderer(unsigned int width, unsigned int height, const char* windowTitle = "OpenGL Renderer");
    ~Renderer();

    // Window Management
    bool initialize();
    bool shouldClose() const;
    void swapBuffers();
    void pollEvents();
    void setViewport(int x, int y, unsigned int width, unsigned int height);

    // Shader Management
    void compileShaders(const char* vertexFile, const char* fragmentFile);
    void activateShaderProgram();
    void deleteShaderProgram();
    GLuint getShaderProgramID() const;

    // Buffer Object Management (VAO, VBO, EBO)
    void createBuffers(GLfloat* vertexData, GLsizeiptr vertexSize, GLuint* indexData, GLsizeiptr indexSize);
    void setupVertexAttributes();
    void bindVertexArray();
    void unbindVertexArray();
    void deleteBuffers();

    // Mesh Generation
    void initializeGridMesh(int gridX, int gridY);
    void updateVertexData(const std::vector<GLfloat>& vertexData);

    // Rendering
    void setClearColor(float r, float g, float b, float a);
    void clear();
    void drawElements(GLsizei indexCount);

    // Render State
    void enableDepthTest();
    void enableBlending();
    void setPolygonMode(GLenum face, GLenum mode);

    // Transform Matrices
    void setModelMatrix(const glm::mat4& model);
    void setViewMatrix(const glm::mat4& view);
    void setProjectionMatrix(const glm::mat4& projection);

    // Uniform Setters
    void setUniform1f(const char* uniformName, float value);
    void setUniformMatrix4fv(const char* uniformName, const glm::mat4& matrix);

    // Getters
    GLFWwindow* getWindow() const;
    int getWindowWidth() const;
    int getWindowHeight() const;
    const std::vector<GLfloat>& getVertices() const;
    const std::vector<GLuint>& getIndices() const;

private:
    // Window properties
    GLFWwindow* window;
    unsigned int windowWidth;
    unsigned int windowHeight;
    std::string windowTitle;

    // Shader program
    GLuint shaderProgramID;

    // Buffer objects
    GLuint VAO_ID;
    GLuint VBO_ID;
    GLuint EBO_ID;

    // Mesh data
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;
    int gridX;
    int gridY;

    // Shader helper methods
    std::string getFileContents(const char* filename);
    void compileErrors(unsigned int shader, const char* type);

    // Private buffer methods
    void bindVBO();
    void unbindVBO();
    void bindEBO();
    void unbindEBO();
};

#endif // RENDERER_HPP
