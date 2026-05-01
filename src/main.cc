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
#include "audioDSP.hpp"
#include "strikeDefs.hpp"

const unsigned int WIDTH  = 640;
const unsigned int HEIGHT = 480;

int numDBSteps = 110; // from 0 to 100 dB in 1 dB increments

struct SimState {
    CircularMembrane membrane;
	DrumRenderer* renderer;
    int simRunning = 0;
	float dB = 0.0f;
};

struct KeyStateVars {
	// Variables that help the rotation of the grid
	float rotation = -30.0f;
	float tilt = 15.0f; 
	bool runAudio = true;
} keyState;

void keyCB(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		keyState.rotation -= 1.0f;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		keyState.rotation += 1.0f;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		keyState.tilt += 1.0f;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		keyState.tilt -= 1.0f;
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		keyState.runAudio = !keyState.runAudio;
	
}

void mouseCB(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double x_pos, y_pos;
        glfwGetCursorPos(window, &x_pos, &y_pos);

        // Convert screen coords to normalized coordinates
        float ndcX = (float)(2.0 * x_pos / WIDTH  - 1.0);
        float ndcY = (float)(1.0 - 2.0 * y_pos / HEIGHT);

        // Reconstruct the same matrices used in the render loop
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(keyState.rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 view  = glm::rotate(
                              glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.5f)),
                              glm::radians(keyState.tilt), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 proj  = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 2.0f, 100.0f);

        // Unproject NDC point into a view-space ray direction, then into world space
        glm::vec4 rayView = glm::inverse(proj) * glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
        rayView = glm::vec4(rayView.x, rayView.y, -1.0f, 0.0f); // direction vector

        glm::mat4 invView   = glm::inverse(view);
        glm::vec3 rayDir    = glm::normalize(glm::vec3(invView * rayView));
        glm::vec3 rayOrigin = glm::vec3(invView * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

        // Intersect ray with the membrane plane (y=0 in world space).
        // Model only rotates around Y so the XZ plane is preserved in world space.
        if (std::abs(rayDir.y) < 1e-6f) return;
        float t = -rayOrigin.y / rayDir.y;
        if (t < 0.0f) return;

        // Transform the hit point into model space and check it is inside the membrane (radius = 1.0)
        glm::vec3 hitModel = glm::vec3(glm::inverse(model) * glm::vec4(rayOrigin + t * rayDir, 1.0f));
        float radius = hitModel.x * hitModel.x + hitModel.z * hitModel.z;
		float theta = std::atan2(hitModel.z, hitModel.x);
		if (theta < 0.0f) theta += 2.0f * M_PI; // atan2 returns [-pi, pi], convert to [0, 2pi]

		if (radius > 1.0f) return;

		StrikeDefs strike;
		strike.amplitude = 1.0f;
		strike.rPos = radius;
		strike.thetaPos = theta;
		
        auto* state = static_cast<SimState*>(glfwGetWindowUserPointer(window));
        state->membrane.setInitialCondition(&strike);
        state->simRunning = true;
		state->dB = 0.0f;
    }
}
void appSettings(){
	std::cout << std::endl;
	std::cout << "--------------- Welcome to the Drum Machine! ----------------" << std::endl;
	std::cout << "Controls:" << std::endl;
	std::cout << "  Click membrane to strike and start simulation" << std::endl;
	std::cout << "  Arrow keys (↑↓ & ←→) to rotate/tilt view" << std::endl;
	std::cout << "  M key to toggle audio on/off" << std::endl;
	std::cout << "  ESC to quit" << std::endl;
	std::cout << "-------------------------------------------------------------" << std::endl;

	std::string dbScale(numDBSteps, ' ');
	dbScale = "[" + dbScale + "]";
	std::cout << "\r" << dbScale.c_str() << -numDBSteps << " dB" << std::flush;
}

void displayLevelBar(float dB) {

	//each # indicates 1 dB
	int barsToShow = std::abs((int)dB); // shift dB so that -60dB is 0 bars, and 0dB is 60 bars
	std::string levelStr(numDBSteps - barsToShow, '#'); // Block character representation
	//concat the remaining string with spaces
	levelStr += std::string(barsToShow, ' ');
	levelStr = "[" + levelStr; // Add left boundary
	levelStr += "]"; // Add right boundary
	std::cout << "\r" << levelStr << dB << " dB" << std::flush;

}

int main(void) {
	// Make OpenMP worker threads sleep between parallel regions instead of spin-waiting.
	// Must be set before the first OMP parallel region initializes the thread pool.
	#if defined(_WIN32) || defined(_WIN64)
	    _putenv("OMP_WAIT_POLICY=passive");
	#else
	setenv("OMP_WAIT_POLICY", "passive", 1);
	#endif

	// Initialize audio engine
	AudioEngine audio;
	audio.start();

	// Init DSP toolbox
	AudioDSP_Toolbox dspToolbox;

	// Initialize rendering engine
   	DrumRenderer drumGui(WIDTH,HEIGHT,"Drum Machine");
   	if(!drumGui.init()){
		std::cerr << "Failed to initialize Drum Machine" << std::endl;
   	}

	appSettings();

	// Input handling
	SimState state;
	state.membrane.init((float)RADIUS, (float)TENSION, (float)MATERIAL_DENSITY, GRID_R, GRID_TH);
	float SIM_RATE = state.membrane.getSimRate();
	//Determine number of physics steps to run per frame based on the ratio of the simulation rate to the audio sample rate, and the audio buffer size. 
	//This ensures that we produce enough audio samples for each chunk we push to the audio engine, while keeping the physics simulation in sync with the audio output.
	int physSteps = std::max(1, (int)std::ceil((double)BUFFER_SIZE * SIM_RATE / SAMPLE_RATE));
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
			if(state.dB <= -100.0f){
				state.simRunning = false;
				state.dB = 0.0f;
				drumGui.updateCircularVertexData(state.membrane.getCurrentGrid());
				continue;
			}
			state.membrane.Simulate(physSteps);
			std::vector<float> audioBuf;
			//this decouples the physics simulation rate from the audio output rate by resampling the current simBuf_ chunk to exactly BUFFER_SIZE samples, which is what pushChunk expects
			audioBuf = dspToolbox.sampleInterp(state.membrane.getPhysicsBuffer().data(),
			                                   state.membrane.getPhysicsBuffer().size(),
			                                   SIM_RATE, SAMPLE_RATE);
			state.dB = dspToolbox.calculateDecibleLevel(audioBuf);
			displayLevelBar(state.dB);
			if (keyState.runAudio) {
				audio.consumeAudio(audioBuf.data(), audioBuf.size());
			}
		}
			
		// Always update and render
		drumGui.updateCircularVertexData(state.membrane.getCurrentGrid());
		drumGui.setClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		drumGui.clear();
		drumGui.activateShaderProgram();

		model = glm::mat4(1.0f);
		view  = glm::mat4(1.0f);
		proj  = glm::mat4(1.0f);

		model = glm::rotate(model, glm::radians(keyState.rotation), glm::vec3(0.0f, 1.0f, 0.0f));
		view  = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.5f));
		view  = glm::rotate(view, glm::radians(keyState.tilt), glm::vec3(1.0f, 0.0f, 0.0f));
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
