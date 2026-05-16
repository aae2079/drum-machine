#include <iostream>
#include <vector>
#include <cstdlib>
#include "simDefs.hpp"
#include "CircularMembrane.hpp"
#include "drumRenderer.hpp"
#include "audioEngine.hpp"
#include "strikeDefs.hpp"
#include "JsonParser.hpp"
#include "PhysicsThread.hpp"

const unsigned int WIDTH  = 640;
const unsigned int HEIGHT = 480;
bool simRunning = false;
bool muted = false;
float rotation = -30.0f;
float tilt = 15.0f;
float fov = 45.0f;

struct AppContext {
	Params params;
	PhysicsThread* physThread;
};

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
		muted = !muted;
}

void zoomCB(GLFWwindow* window, double xoffset, double yoffset)
{	
	auto* ctrl = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
	float sensitivity_constant = ctrl->params.zoom_sensitivity; // Adjust this value to control zoom speed
	//update field of view with scroll, clamping to reasonable range
	fov -= (float)yoffset * sensitivity_constant;
	fov = std::max(10.0f, std::min(90.0f, fov));
}

void strikeCB(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double x_pos, y_pos;
        glfwGetCursorPos(window, &x_pos, &y_pos);

        float ndcX = (float)(2.0 * x_pos / WIDTH  - 1.0);
        float ndcY = (float)(1.0 - 2.0 * y_pos / HEIGHT);

        glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 view  = glm::rotate(
                              glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.5f)),
                              glm::radians(tilt), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 proj  = glm::perspective(glm::radians(fov), (float)WIDTH / HEIGHT, 2.0f, 100.0f);

        glm::vec4 rayView = glm::inverse(proj) * glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
        rayView = glm::vec4(rayView.x, rayView.y, -1.0f, 0.0f);

        glm::mat4 invView   = glm::inverse(view);
        glm::vec3 rayDir    = glm::normalize(glm::vec3(invView * rayView));
        glm::vec3 rayOrigin = glm::vec3(invView * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

        if (std::abs(rayDir.y) < 1e-6f) return;
        float t = -rayOrigin.y / rayDir.y;
        if (t < 0.0f) return;

        glm::vec3 hitModel = glm::vec3(glm::inverse(model) * glm::vec4(rayOrigin + t * rayDir, 1.0f));
        float r2    = hitModel.x * hitModel.x + hitModel.z * hitModel.z;
		float theta = std::atan2(hitModel.z, hitModel.x);
		if (theta < 0.0f) theta += 2.0f * M_PI;

		if (r2 > 1.0f) return;

		StrikeDefs currStrike;
		currStrike.amplitude = 0.5f;
		currStrike.rPos = std::sqrt(r2);
		currStrike.thetaPos = theta;

		auto* ctrl = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
		ctrl->physThread->pushStrike(currStrike);
		std::cout << "New Event: r=" << currStrike.rPos << " theta=" << currStrike.thetaPos << std::endl;
    }
}


void appSettings(){
	std::cout << std::endl;
	std::cout << "--------------- Welcome to the Drum Machine! ----------------" << std::endl;
	std::cout << "Controls:" << std::endl;
	std::cout << "  Click membrane to strike and start simulation" << std::endl;
	std::cout << "  Arrow keys (↑↓ & ←→) to rotate/tilt view" << std::endl;
	std::cout << "  Scroll to zoom in/out" << std::endl;
	std::cout << "  M key to toggle audio on/off" << std::endl;
	std::cout << "  ESC to quit" << std::endl;
	std::cout << "-------------------------------------------------------------" << std::endl;
}

int main(int argc, char** argv) {
	#if defined(_WIN32) || defined(_WIN64)
	    _putenv("OMP_WAIT_POLICY=passive");
	#else
	setenv("OMP_WAIT_POLICY", "passive", 1);
	#endif

	if (argc < 2){
		std::cerr << "Usage: " << argv[0] << " drum_config.json" << std::endl;
		return -1;
	}

	AppContext ctrl;
	parseJsonSettings(argv[1], ctrl.params);

	AudioEngine audio(ctrl.params.audio.sampleRate, ctrl.params.audio.bufferSize);
	audio.start();

	PhysicsThread physEngine(audio);
	physEngine.start(ctrl.params);
	ctrl.physThread = &physEngine;

	DrumRenderer drumGui(WIDTH, HEIGHT, ctrl.params.grid.grid_r, ctrl.params.grid.grid_th, "Drum Machine");
	if (!drumGui.init()){
		std::cerr << "Failed to initialize Drum Machine" << std::endl;
	}

	appSettings();

	glfwSetWindowUserPointer(drumGui.getWindow(), &ctrl);
	glfwSetKeyCallback(drumGui.getWindow(), keyCB);
	glfwSetMouseButtonCallback(drumGui.getWindow(), strikeCB);
	glfwSetScrollCallback(drumGui.getWindow(), zoomCB);

	drumGui.compileShaders("shaders/default.vert","shaders/default.frag");
	drumGui.enableDepthTest();
	drumGui.enableBlending();
	drumGui.setPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view  = glm::mat4(1.0f);
	glm::mat4 proj  = glm::mat4(1.0f);

	std::vector<float> gridBuf;
	glfwSwapInterval(1); // vsync handles frame pacing
	while (!drumGui.shouldClose()) {
		drumGui.pollEvents();
		audio.mute(muted);
		if (physEngine.tryGetGrid(gridBuf, 16))
			drumGui.updateCircularVertexData(gridBuf);

		drumGui.setClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		drumGui.clear();
		drumGui.activateShaderProgram();

		model = glm::mat4(1.0f);
		view  = glm::mat4(1.0f);
		proj  = glm::mat4(1.0f);

		model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
		view  = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.5f));
		view  = glm::rotate(view, glm::radians(tilt), glm::vec3(1.0f, 0.0f, 0.0f));
		proj  = glm::perspective(glm::radians(fov), (float)WIDTH / HEIGHT, 2.0f, 100.0f);

		drumGui.setMatrices(model, view, proj);
		drumGui.setUniform1f("scale", 0.5f);
		drumGui.drawElements();
		drumGui.swapBuffers();
	}

    return 0;
}
