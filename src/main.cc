#include <iostream>
#include <vector>
#include <chrono>
#include "simDefs.hpp"
#include "RectangularMembrane.hpp"
#include "drumRenderer.hpp"

const unsigned int WIDTH  = 640;
const unsigned int HEIGHT = 480;
int firstTime = 1;
int main(void) {
	std::string input;
    float sim_time = 2.0f;
    int num_samples = sim_time * SAMPLE_RATE;

   	DrumRenderer drumGui(WIDTH,HEIGHT,"Drum Machine");
   	if(!drumGui.init()){
		std::cerr << "Failed to initialize Drum Machine" << std::endl;
   	}

   	drumGui.compileShaders("default.vert","default.frag");
	drumGui.setupVertexAttributes();

    // Variables that help the rotation of the grid
	float rotation = -30.0f;
    float tilt = 15.0f;
	drumGui.enableDepthTest();
	drumGui.enableBlending();
	drumGui.setPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Initializes matrices so they are not the null matrix
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 proj = glm::mat4(1.0f);
	RectangularMembrane membrane;
	bool simRunning = false;
	int sampsProc = 0;
	while (!drumGui.shouldClose()) {
		drumGui.pollEvents();
		// Input handling
		if (glfwGetKey(drumGui.getWindow(), GLFW_KEY_S) == GLFW_PRESS)
			simRunning = true;
		if (glfwGetKey(drumGui.getWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
			break;
		if (glfwGetKey(drumGui.getWindow(), GLFW_KEY_LEFT) == GLFW_PRESS)
			rotation -= 1.0f;
		if (glfwGetKey(drumGui.getWindow(), GLFW_KEY_RIGHT) == GLFW_PRESS)
			rotation += 1.0f;
		if (glfwGetKey(drumGui.getWindow(), GLFW_KEY_UP) == GLFW_PRESS)
			tilt += 1.0f;
		if (glfwGetKey(drumGui.getWindow(), GLFW_KEY_DOWN) == GLFW_PRESS)
			tilt -= 1.0f;

		// Step sim only if running
		if (simRunning){
			if(sampsProc > num_samples){
				simRunning = false;
				sampsProc = 0;
				membrane.setInitialCondition();
				std::cout << "Simulation finished! Press S to start again." << std::endl;
				drumGui.updateVertexData(membrane.getCurrentGrid());
				continue;
			}
			membrane.Simulate();
			sampsProc += BUFFER_SIZE;
		}
			
		// Always update and render
		drumGui.updateVertexData(membrane.getCurrentGrid());
		drumGui.setClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		drumGui.clear();
		drumGui.activateShaderProgram();

		model = glm::mat4(1.0f);
		view  = glm::mat4(1.0f);
		proj  = glm::mat4(1.0f);

		model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
		view  = glm::translate(view, glm::vec3(0.0f, -0.5f, -3.5f));
		view  = glm::rotate(view, glm::radians(tilt), glm::vec3(1.0f, 0.0f, 0.0f));
		proj  = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 2.0f, 100.0f);

		drumGui.setMatrices(model, view, proj);
		drumGui.setUniform1f("scale", 0.5f);
		drumGui.drawElements();
		drumGui.swapBuffers();
	}
    return 0;
}
