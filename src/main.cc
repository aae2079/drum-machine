
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "drumRenderer.h"

static DrumRenderer gDrum;
static int gWidth = 900;
static int gHeight = 900;

// Create a flat plane
GLfloat vertices[] = 
{ // Coordinates         /     Colors         /   TexCoord             / NORMALS    //
    -1.0f,  0.0f, 1.0f,   0.0f, 0.0f, 0.0f,      0.0f, 0.0f,        0.0f, 1.0f, 0.0f,
     -1.0f, 0.0f, -1.0f,   0.0f, 0.0f, 0.0f,      0.0f, 1.0f,        0.0f, 1.0f, 0.0f,
     1.0f,  0.0f, -1.0f,   0.0f, 0.0f, 0.0f,      1.0f, 1.0f,        0.0f, 1.0f, 0.0f,
    1.0f,  0.0f, 1.0f,   0.0f, 0.0f, 0.0f,      1.0f, 0.0f,        0.0f, 1.0f, 0.0f
};

GLuint indices[] = {
    0, 1, 2,
    0, 2, 3
};

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }

    if (key == GLFW_KEY_S && action == GLFW_PRESS){
        //simulate
    }

    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS){
        //change preset
    }

    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS){
        //change preset
    }
}
int main(void){
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit()){
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(gWidth, gHeight, "Drum Machine | Press S to Start Simulation", NULL, NULL);
    if (!window){
        glfwTerminate();
        return -1;
    }
    glfwSetKeyCallback(window, key_callback);
    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    gDrum.init(gWidth, gHeight);
    
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)){
        /* Render here */
        gDrum.render();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
