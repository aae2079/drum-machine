#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include <vector>
#include "simDefs.hpp"
#include "VAO.hpp"
#include "VBO.hpp"
#include "EBO.hpp"
#include "shaderClass.hpp"
#include "RectangularMembrane.hpp"

const unsigned int WIDTH  = 640;
const unsigned int HEIGHT = 480;


// // Vertices coordinates
// GLfloat vertices[] =
// { //     COORDINATES     /        COLORS        /    TexCoord    /       NORMALS     //
// 	-1.0f, 0.0f,  1.0f,		0.0f, 0.0f, 0.0f,		0.0f, 0.0f,		0.0f, 1.0f, 0.0f,
// 	-1.0f, 0.0f, -1.0f,		0.0f, 0.0f, 0.0f,		0.0f, 1.0f,		0.0f, 1.0f, 0.0f,
// 	 1.0f, 0.0f, -1.0f,		0.0f, 0.0f, 0.0f,		1.0f, 1.0f,		0.0f, 1.0f, 0.0f,
// 	 1.0f, 0.0f,  1.0f,		0.0f, 0.0f, 0.0f,		1.0f, 0.0f,		0.0f, 1.0f, 0.0f
// };

// // Indices for vertices order
// GLuint indices[] =
// {
// 	0, 1, 2,
// 	0, 2, 3
// };

std::vector<GLfloat> vertices;
std::vector<GLuint> indices;


 
void initShape(void){
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
};


int main(void) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    RectangularMembrane membrane;

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

    //patch y
	initShape();
    // Generates Shader object using shaders default.vert and default.frag
	Shader shaderProgram("default.vert", "default.frag");

	// Generates Vertex Array Object and binds it
	VAO VAO1;
	VAO1.Bind();
	// Generates Vertex Buffer Object and links it to vertices
	VBO VBO1(vertices.data(), vertices.size() * sizeof(GLfloat));
	// Generates Element Buffer Object and links it to indices
	EBO EBO1(indices.data(), indices.size() * sizeof(GLuint));
	// Links VBO attributes such as coordinates and colors to VAO
	VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 11 * sizeof(float), (void*)0);
	VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	VAO1.LinkAttrib(VBO1, 2, 2, GL_FLOAT, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	VAO1.LinkAttrib(VBO1, 3, 3, GL_FLOAT, 11 * sizeof(float), (void*)(8 * sizeof(float)));
	// Unbind all to prevent accidentally modifying them
	VAO1.Unbind();
	VBO1.Unbind();
	EBO1.Unbind();

    // Gets ID of uniform called "scale"
	GLuint uniID = glGetUniformLocation(shaderProgram.ID, "scale");

    // Variables that help the rotation of the pyramid
	float rotation = -30.0f;
    float tilt = 15.0f;
	double prevTime = glfwGetTime();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(window)) {

        // Step sim and update mesh
        membrane.Simulate();
        for (int i = 0; i < GRID_X; i++) {
            for (int j = 0; j < GRID_Y; j++) {
                int vertexStart = (i * GRID_X + j) * 11;
                vertices[vertexStart + 1] = membrane.getCurrentGrid()[j + i * GRID_X];
            }
        }
        glBindBuffer(GL_ARRAY_BUFFER, VBO1.ID);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(GLfloat), vertices.data());


        // Specify the color of the background
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		// Clean the back buffer and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Tell OpenGL which Shader Program we want to use
		shaderProgram.Activate();

        // Replace with:
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            rotation -= 1.0f;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            rotation += 1.0f;
        if (glfwGetKey(window,GLFW_KEY_UP) == GLFW_PRESS)
            tilt += 1.0f;
        if(glfwGetKey(window,GLFW_KEY_DOWN) == GLFW_PRESS)
            tilt -= 1.0f;

        // Initializes matrices so they are not the null matrix
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 proj = glm::mat4(1.0f);

		// Assigns different transformations to each matrix
		model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
		view = glm::translate(view, glm::vec3(0.0f, -0.5f, -3.5f));
        view = glm::rotate(view, glm::radians(tilt), glm::vec3(1.0f, 0.0f, 0.0f));  
		proj = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 2.0f, 100.0f);

		// Outputs the matrices into the Vertex Shader
		int modelLoc = glGetUniformLocation(shaderProgram.ID, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		int viewLoc = glGetUniformLocation(shaderProgram.ID, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		int projLoc = glGetUniformLocation(shaderProgram.ID, "proj");
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

		// Assigns a value to the uniform; NOTE: Must always be done after activating the Shader Program
		glUniform1f(uniID, 0.5f);

        VAO1.Bind();

        glDrawElements(GL_TRIANGLES,indices.size(),GL_UNSIGNED_INT,0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    VAO1.Delete();
    VBO1.Delete();
    EBO1.Delete();
    shaderProgram.Delete();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}