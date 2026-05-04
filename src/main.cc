#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <cstdlib>
#include "simDefs.hpp"
#include "drumRenderer.hpp"
#include "audioEngine.hpp"
#include "physicsThread.hpp"
#include "strikeDefs.hpp"

const unsigned int WIDTH  = 640;
const unsigned int HEIGHT = 480;

const int numDBSteps = 110;

struct SimState {
    PhysicsThread* physThread;
};

struct KeyStateVars {
    float rotation = -30.0f;
    float tilt     = 15.0f;
    bool  runAudio = true;
} keyState;

void keyCB(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (action != GLFW_PRESS) return;

    auto* state = static_cast<SimState*>(glfwGetWindowUserPointer(window));

    switch (key) {
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, true);  break;
        case GLFW_KEY_LEFT:   keyState.rotation -= 1.0f;               break;
        case GLFW_KEY_RIGHT:  keyState.rotation += 1.0f;               break;
        case GLFW_KEY_UP:     keyState.tilt     += 1.0f;               break;
        case GLFW_KEY_DOWN:   keyState.tilt     -= 1.0f;               break;
        case GLFW_KEY_M:
            keyState.runAudio = !keyState.runAudio;
            state->physThread->setRunAudio(keyState.runAudio);
            break;
        default: break;
    }
}

void mouseCB(GLFWwindow* window, int button, int action, int /*mods*/)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) return;

    double x_pos, y_pos;
    glfwGetCursorPos(window, &x_pos, &y_pos);

    float ndcX = (float)(2.0 * x_pos / WIDTH  - 1.0);
    float ndcY = (float)(1.0 - 2.0 * y_pos / HEIGHT);

    glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(keyState.rotation), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 view  = glm::rotate(
                          glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.5f)),
                          glm::radians(keyState.tilt), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 proj  = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 2.0f, 100.0f);

    glm::vec4 rayView   = glm::inverse(proj) * glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
    rayView             = glm::vec4(rayView.x, rayView.y, -1.0f, 0.0f);
    glm::mat4 invView   = glm::inverse(view);
    glm::vec3 rayDir    = glm::normalize(glm::vec3(invView * rayView));
    glm::vec3 rayOrigin = glm::vec3(invView * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

    if (std::abs(rayDir.y) < 1e-6f) return;
    float t = -rayOrigin.y / rayDir.y;
    if (t < 0.0f) return;

    glm::vec3 hitModel = glm::vec3(glm::inverse(model) * glm::vec4(rayOrigin + t * rayDir, 1.0f));
    float radius = hitModel.x * hitModel.x + hitModel.z * hitModel.z;
    float theta  = std::atan2(hitModel.z, hitModel.x);
    if (theta < 0.0f) theta += 2.0f * (float)M_PI;
    if (radius > 1.0f) return;

    StrikeDefs strike;
    strike.amplitude = 1.0f;
    strike.rPos      = radius;
    strike.thetaPos  = theta;

    auto* state = static_cast<SimState*>(glfwGetWindowUserPointer(window));
    state->physThread->enqueueStrike(strike);
}

void appSettings() {
    std::cout << "\n--------------- Welcome to the Drum Machine! ----------------\n";
    std::cout << "Controls:\n";
    std::cout << "  Click membrane to strike and start simulation\n";
    std::cout << "  Arrow keys (↑↓ & ←→) to rotate/tilt view\n";
    std::cout << "  M key to toggle audio on/off\n";
    std::cout << "  ESC to quit\n";
    std::cout << "-------------------------------------------------------------\n";

    std::string dbScale(numDBSteps, ' ');
    dbScale = "[" + dbScale + "]";
    std::cout << "\r" << dbScale << -numDBSteps << " dB" << std::flush;
}

void displayLevelBar(float dB) {
    int barsToShow = std::abs((int)dB);
    std::string levelStr(numDBSteps - barsToShow, '#');
    levelStr += std::string(barsToShow, ' ');
    levelStr  = "[" + levelStr + "]";
    std::cout << "\r" << levelStr << dB << " dB" << std::flush;
}

int main(void) {
#if defined(_WIN32) || defined(_WIN64)
    _putenv("OMP_WAIT_POLICY=passive");
#else
    setenv("OMP_WAIT_POLICY", "passive", 1);
#endif

    // Audio engine must outlive the physics thread (LIFO destruction order).
    AudioEngine   audio;
    PhysicsThread physThread(audio);

    audio.start();
    physThread.start();

    DrumRenderer drumGui(WIDTH, HEIGHT, "Drum Machine");
    if (!drumGui.init()) {
        std::cerr << "Failed to initialize Drum Machine\n";
        return 1;
    }

    appSettings();

    SimState state;
    state.physThread = &physThread;

    glfwSetKeyCallback(drumGui.getWindow(), keyCB);
    glfwSetMouseButtonCallback(drumGui.getWindow(), mouseCB);
    glfwSetWindowUserPointer(drumGui.getWindow(), &state);

    drumGui.compileShaders("shaders/default.vert", "shaders/default.frag");
    drumGui.enableDepthTest();
    drumGui.enableBlending();
    drumGui.setPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glm::mat4 model(1.0f), view(1.0f), proj(1.0f);

    glfwSwapInterval(1);

    while (!drumGui.shouldClose()) {
        auto frameStart = std::chrono::steady_clock::now();

        drumGui.pollEvents();

        // Display dB level when simulation is running
        if (physThread.isSimRunning())
            displayLevelBar(physThread.getdB());

        // Pull latest vertex data from physics thread and render
        std::vector<float> verts = physThread.getLatestVertexData();
        drumGui.updateCircularVertexData(verts);

        drumGui.setClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        drumGui.clear();
        drumGui.activateShaderProgram();

        model = glm::rotate(glm::mat4(1.0f), glm::radians(keyState.rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        view  = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.5f));
        view  = glm::rotate(view, glm::radians(keyState.tilt), glm::vec3(1.0f, 0.0f, 0.0f));
        proj  = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 2.0f, 100.0f);

        drumGui.setMatrices(model, view, proj);
        drumGui.setUniform1f("scale", 0.5f);
        drumGui.drawElements();
        drumGui.swapBuffers();

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::steady_clock::now() - frameStart);
        auto budget  = std::chrono::milliseconds(16);
        if (elapsed < budget)
            std::this_thread::sleep_for(budget - elapsed);
    }

    // PhysicsThread destructor stops the thread; AudioEngine destructor stops the stream.
    return 0;
}
