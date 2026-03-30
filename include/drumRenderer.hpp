#ifndef DRUM_RENDERER_HPP
#define DRUM_RENDERER_HPP

#include "RectangularMembrane.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>

class DrumRenderer{
    public:
        DrumRenderer(uint32_t wWidth, uint32_t wHeight, const char* windowTitle = "Drum Machine");
        ~DrumRenderer();

        //Window Managagment
        bool init();
        bool shouldClose() const;
        void swapBuffers();
        void pollEvents();

        //Shader Management
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
        
        void updateVertexData(const std::vector<GLfloat>& gridData);

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


        GLFWwindow* getWindow() const;
        int getWindowWidth() const;
        int getWindowHeight() const;
        const std::vector<GLfloat>& getVertices() const;
        const std::vector<GLuint>& getIndices() const;       

    private:
        GLFWwindow *window;
        uint32_t WIDTH;
        uint32_t HEIGHT;
        std::string windowTitle;

        void buildMesh();
        
        GLuint vao, vbo, ebo;
        GLuint shaderProgram;

        //meshData
        std::vector<GLfloat> vertices_;
        std::vector<GLfloat> indices_;
        int gridX;
        int gridY;
        

        //helpers
        std::string getShaderContents(const char* filename);
        void compileErrors(unsigned int shader, const char* type);
        
        void bindVBO();
        void unbindVBO();
        void bindEBO();
        void unbindEBO();
};

#endif