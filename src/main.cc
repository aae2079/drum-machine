#include <iostream>
#include <vector>
#include <queue>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <mutex>
#include<condition_variable>
#include "simDefs.hpp"
#include "RectangularMembrane.hpp"
#include "CircularMembrane.hpp"
#include "drumRenderer.hpp"
#include "audioEngine.hpp"
#include "audioDSP.hpp"
#include "strikeDefs.hpp"
#include "JsonParser.hpp"

const unsigned int WIDTH  = 640;
const unsigned int HEIGHT = 480;
bool simRunning = false; //sim doesnt run on startup, waits for user to click membrane to strike and start simulating
std::atomic<bool> runAudio = true;
// Variables that help the rotation of the grid
float rotation = -30.0f;
float tilt = 15.0f;

int numDBSteps = 110; // from 0 to 100 dB in 1 dB increments

std::queue<StrikeDefs> strikeQueue;
std::mutex mtx;
std::condition_variable cv;

std::vector<float> latestGrid;
std::mutex gridMtx;

std::vector<float> audioBuf;
std::mutex audioBufMtx;

std::atomic<bool> running{true};

//openGl functions for handling user input
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
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		runAudio = !runAudio;
	
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
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 view  = glm::rotate(
                              glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.5f)),
                              glm::radians(tilt), glm::vec3(1.0f, 0.0f, 0.0f));
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

		StrikeDefs currStrike;
		currStrike.amplitude = 1.0f; // could be based on how close
		currStrike.rPos = radius;
		currStrike.thetaPos = theta;

		//mutex 
		std::lock_guard<std::mutex> lock(mtx);
		strikeQueue.push(currStrike);
		std::cout << "new event" << std::endl;
		cv.notify_one();

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

// 	std::string dbScale(numDBSteps, ' ');
// 	dbScale = "[" + dbScale + "]";
// 	std::cout << "\r" << dbScale.c_str() << -numDBSteps << " dB" << std::flush;
}

void physicsEngine(Params params){
	CircularMembrane membrane;
	membrane.init(params.timbre.radius, params.timbre.damping, params.timbre.tension, params.timbre.material_density, params.grid.grid_r, params.grid.grid_th);
	int physSteps = (int)(params.audio.sampleRate * params.audio.bufferSize / membrane.getSimRate()) + 1; //number of physics steps to run for each audio buffer's worth of time
	AudioDSP_Toolbox dsp;
	while(running){
		std::unique_lock<std::mutex> lock(mtx);
		cv.wait(lock, []{ return !strikeQueue.empty() || !running; });
		if (!running) break;
		StrikeDefs strike = strikeQueue.front();
		strikeQueue.pop();
		//Process data
		std::cout << "There is data to process! Strike at r = " << strike.rPos << ", theta = " << strike.thetaPos << std::endl;
		membrane.setInitialCondition(&strike);
		std::vector<float> physBuf(physSteps, 0.0f);

	}

}

int main(int argc, char** argv) {
	// Make OpenMP worker threads sleep between parallel regions instead of spin-waiting.
	// Must be set before the first OMP parallel region initializes the thread pool.
	#if defined(_WIN32) || defined(_WIN64)
	    _putenv("OMP_WAIT_POLICY=passive");
	#else
	setenv("OMP_WAIT_POLICY", "passive", 1);
	#endif

	if (argc < 2){
		std::cerr << "Usage: " << argv[0] << " drum_config.json" << std::endl;
		return -1;
	}
	Params params;
	parseJsonSettings(argv[1], params);

	// Initialize audio engine - starts PA thread
	AudioEngine audio(params.audio.sampleRate, params.audio.bufferSize);
	audio.start();

	std::thread physics_thread(physicsEngine,params);

	// Initialize rendering engine
   	DrumRenderer drumGui(WIDTH,HEIGHT,params.grid.grid_r, params.grid.grid_th,"Drum Machine");
   	if(!drumGui.init()){
		std::cerr << "Failed to initialize Drum Machine" << std::endl;
   	}

	appSettings();

	glfwSetKeyCallback(drumGui.getWindow(), keyCB);
	// glfwSetWindowUserPointer(drumGui.getWindow(), &state);
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
			
		// Always update and render
		{
			std::lock_guard<std::mutex> gridLock(gridMtx);
			if (!latestGrid.empty())
				drumGui.updateCircularVertexData(latestGrid);
		}
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
		if (elapsed < frameBudget) std::this_thread::sleep_for(frameBudget - elapsed);
	}

	running.store(false);
	cv.notify_all();
	physics_thread.join();
	//audio deconstructor destroys PA thread

    return 0;
}
