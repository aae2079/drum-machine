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
        DrumRenderer(uint32_t wWidth, uint32_t wHeight, int gridR, int gridTH, float shellLength = 0.5f, const char* windowTitle = "Drum Machine");
        ~DrumRenderer();

        //Window Managagment
        bool init();
        void initStrikeMarker();
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
        void updateCircularVertexData(const std::vector<GLfloat>& gridData);

        // Rendering
        void setClearColor(float r, float g, float b, float a);
        void clear();
        void drawElements();
        void drawShell();
        void drawStrikeMarker(std::queue<StrikeMarker>& markerQueue, float fadeTime);

        // Render State
        void enableDepthTest();
        void enableBlending();
        void setPolygonMode(GLenum face, GLenum mode);

        // Transform Matrices
        void setMatrices(const glm::mat4& model, const glm::mat4& view, const glm::mat4& proj);

        // Uniform Setters
        void setUniform1f(const char* uniformName, float value);
        void setUniformMatrix4fv(const char* uniformName, const glm::mat4& matrix);
        void setUniform3f(const char* uniformName, float v0, float v1, float v2);
        void setUniform4f(const char* uniformName, float v0, float v1, float v2, float v3);

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
        void buildShell();
        
        GLuint vao, vbo, ebo;
        GLuint shell_vao, shell_vbo, shell_ebo;
        GLsizei shellIndexCount_;
        GLuint marker_vao, marker_vbo, markerVertexCount;
        GLuint shaderProgramID;

        //meshData
        std::vector<GLfloat> vertices_;
        std::vector<GLuint> indices_;
        std::vector<GLfloat> shellVertices_;
        std::vector<GLuint> shellIndices_;
        int gridR_;
        int gridTH_;
        float shellLength_;

        int gridX;
        int gridY;
        

        //helpers
        std::string getShaderContents(const char* filename);
        void compileErrors(unsigned int shader, const char* type);
        
        void bindVBO();
        void unbindVBO();
        void bindEBO();
        void unbindEBO();

        void bindMarkersVBO();
        void unbindMarkersVBO();

};

#endif