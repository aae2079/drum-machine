#include <iostream>
#include <vector>
#include <chrono>
#include "simDefs.hpp"
#include "RectangularMembrane.hpp"
#include "drumRenderer.hpp"

const unsigned int WIDTH  = 640;
const unsigned int HEIGHT = 480;
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
    while (!drumGui.shouldClose()) {
		std::string input;
		std::cout << "Press S to start Drum Simulation: (E to exit) " << std::endl;
		std::cin >> input;
		if (input == "S" || input == "s"){
			 std::cout << "Starting Drum Simulation..." << std::endl;
            int sampsProc = 0;
            RectangularMembrane membrane;
			while (sampsProc < num_samples){
				auto start = std::chrono::high_resolution_clock::now();
                // Generate ONE chunk of 1024 samples
                membrane.Simulate();
				auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> duration = end - start;

				drumGui.updateVertexData(membrane.getCurrentGrid());

				// Specify the color of the background
				drumGui.setClearColor(0.1f, 0.1f, 0.1f, 1.0f);
				// Clean the back buffer and depth buffer
				drumGui.clear();
				// Tell OpenGL which Shader Program we want to use
				drumGui.activateShaderProgram();

				if (glfwGetKey(drumGui.getWindow(), GLFW_KEY_LEFT) == GLFW_PRESS)
					rotation -= 1.0f;
				if (glfwGetKey(drumGui.getWindow(), GLFW_KEY_RIGHT) == GLFW_PRESS)
					rotation += 1.0f;
				if (glfwGetKey(drumGui.getWindow(),GLFW_KEY_UP) == GLFW_PRESS)
					tilt += 1.0f;
				if(glfwGetKey(drumGui.getWindow(),GLFW_KEY_DOWN) == GLFW_PRESS)
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
				drumGui.setMatrices(model,view,proj);
				// Assigns a value to the uniform; NOTE: Must always be done after activating the Shader Program
				drumGui.setUniform1f("scale", 0.5f);

				drumGui.drawElements();
				drumGui.swapBuffers();
				drumGui.pollEvents();
				sampsProc += BUFFER_SIZE-(int)OVERLAP;

			}
		}else if(input == "E" || input == "e"){
            std::cout << "Exiting Drum Simulation..." << std::endl;
            break;
        } else {
            std::cout << "Invalid input. Please press S to start or E to exit." << std::endl;
        }
       
    }

    return 0;
}
