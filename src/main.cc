#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <cstdlib>
#include "simDefs.hpp"
#include "RectangularMembrane.hpp"
#include "CircularMembrane.hpp"
#include "drumRenderer.hpp"
#include "audioEngine.hpp"

float SIM_RATE; // global variable to hold the simulation sample rate, will be set by CircularMembrane init and used by main loop for upsampling

const unsigned int WIDTH  = 640;
const unsigned int HEIGHT = 480;
int firstTime = 1;
bool simRunning = false;
// Variables that help the rotation of the grid
float rotation = -30.0f;
float tilt = 15.0f;

typedef struct {
    CircularMembrane membrane;
    int simRunning = 0;
    int sampsProc = 0;
}SimState;


void keyCB(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		rotation -= 1.0f;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		rotation += 1.0f;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		tilt += 1.0f;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		tilt -= 1.0f;
}

void mouseCB(GLFWwindow* window, int button, int action, int mods)
{
	// Placeholder for mouse input handling if needed in the future
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        auto* state = static_cast<SimState*>(glfwGetWindowUserPointer(window));
        state->membrane.setInitialCondition();
        state->sampsProc = 0;
        state->simRunning = true;
    }
}

int main(void) {
	// Make OpenMP worker threads sleep between parallel regions instead of spin-waiting.
	// Must be set before the first OMP parallel region initializes the thread pool.
	#if defined(_WIN32) || defined(_WIN64)
	    _putenv("OMP_WAIT_POLICY=passive");
	#else
	setenv("OMP_WAIT_POLICY", "passive", 1);
	#endif
	std::string input;
    float sim_time = 2.0f;
    int num_samples = sim_time * SAMPLE_RATE;

	// Initialize audio engine
	AudioEngine audio;
	audio.start();

	// Initialize rendering engine
   	DrumRenderer drumGui(WIDTH,HEIGHT,"Drum Machine");
   	if(!drumGui.init()){
		std::cerr << "Failed to initialize Drum Machine" << std::endl;
   	}
	// Input handling
	SimState state;
	state.membrane.init((float)RADIUS, (float)TENSION, (float)MATERIAL_DENSITY, GRID_R, GRID_TH);
	SIM_RATE = state.membrane.getSimRate();
	glfwSetKeyCallback(drumGui.getWindow(), keyCB);
	glfwSetWindowUserPointer(drumGui.getWindow(), &state);
	glfwSetMouseButtonCallback(drumGui.getWindow(), mouseCB);

   	drumGui.compileShaders("shaders/default.vert","shaders/default.frag");


	drumGui.enableDepthTest();
	drumGui.enableBlending();
	drumGui.setPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Initializes matrices so they are not the null matrix
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 proj = glm::mat4(1.0f);

	glfwSwapInterval(1); // Enable vsync for smoother rendering
	while (!drumGui.shouldClose()) {
		auto frameStart = std::chrono::steady_clock::now();
		drumGui.pollEvents();
		// Step sim only if running
		if (state.simRunning){
			if(state.sampsProc > num_samples){
				state.simRunning = false;
				state.sampsProc = 0;
				std::cout << "Simulation finished! Click again." << std::endl;
				drumGui.updateCircularVertexData(state.membrane.getCurrentGrid());
				continue;
			}
			state.membrane.Simulate();
			std::vector<float> audioBuf;
			//this decouples the physics simulation rate from the audio output rate by resampling the current simBuf_ chunk to exactly BUFFER_SIZE samples, which is what pushChunk expects
			audioBuf = state.membrane.sampleInterp(state.membrane.getPhysicsBuffer().data(),
			                                   state.membrane.getPhysicsBuffer().size(),
			                                   SIM_RATE, SAMPLE_RATE);
			audio.pushChunk(audioBuf.data(), audioBuf.size());
			
			//add logger here eventually
			
			audio.delay();
			state.sampsProc += BUFFER_SIZE;
		}
			
		// Always update and render
		drumGui.updateCircularVertexData(state.membrane.getCurrentGrid());
		drumGui.setClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		drumGui.clear();
		drumGui.activateShaderProgram();

		model = glm::mat4(1.0f);
		view  = glm::mat4(1.0f);
		proj  = glm::mat4(1.0f);

		model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
		view  = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.5f));
		view  = glm::rotate(view, glm::radians(tilt), glm::vec3(1.0f, 0.0f, 0.0f));
		proj  = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 2.0f, 100.0f);

		drumGui.setMatrices(model, view, proj);
		drumGui.setUniform1f("scale", 0.5f);
		drumGui.drawElements();
		drumGui.swapBuffers();

		auto frameEnd = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - frameStart);
		auto frameBudget = std::chrono::milliseconds(16);
		if (elapsed < frameBudget)
			std::this_thread::sleep_for(frameBudget - elapsed);
		}
    return 0;
}
