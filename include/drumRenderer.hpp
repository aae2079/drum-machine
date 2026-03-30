#ifndef DRUM_RENDERER_HPP
#define DRUM_RENDERER_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>
#include "simDefs.hpp"

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
        void drawElements();

        // Render State
        void enableDepthTest();
        void enableBlending();
        void setPolygonMode(GLenum face, GLenum mode);

        // Transform Matrices
        void setMatrices(const glm::mat4& model, const glm::mat4& view, const glm::mat4& proj);

        // Uniform Setters
        void setUniform1f(const char* uniformName, float value);
        void setUniformMatrix4fv(const char* uniformName, const glm::mat4& matrix);


        GLFWwindow* getWindow() const;
        int getWindowWidth() const;
        int getWindowHeight() const;
        std::vector<GLfloat>& getVertices();
        std::vector<GLuint>& getIndices();       

    private:
        GLFWwindow *window;
        uint32_t WIDTH;
        uint32_t HEIGHT;
        std::string windowTitle;

        void buildMesh();
        
        GLuint vao, vbo, ebo;
        GLuint shaderProgramID;

        //meshData
        std::vector<GLfloat> vertices_;
        std::vector<GLuint> indices_;
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