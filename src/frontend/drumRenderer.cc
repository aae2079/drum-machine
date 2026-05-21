#include "drumRenderer.hpp"
#include <glad/glad.h>
#include <iostream>
#include <cmath>

using namespace std;

DrumRenderer::DrumRenderer(uint32_t wWidth, uint32_t wHeight, int gridR, int gridTH, float shellLength, const char* windowTitle)
        : WIDTH(wWidth), HEIGHT(wHeight), windowTitle(windowTitle), window(nullptr), vao(0), vbo(0), ebo(0),
          shell_vao(0), shell_vbo(0), shell_ebo(0), shellIndexCount_(0),
          marker_vao(0), marker_vbo(0), markerVertexCount(0),
          shaderProgramID(0), gridR_(gridR), gridTH_(gridTH), shellLength_(shellLength), gridX(50), gridY(50) {
}

DrumRenderer::~DrumRenderer() {
	deleteBuffers();
	deleteShaderProgram();
	if (window) {
		glfwDestroyWindow(window);
	}
	glfwTerminate();
}

bool DrumRenderer::init(){
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    window = glfwCreateWindow(WIDTH, HEIGHT, "Drum Machine", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glViewport(0,0, WIDTH, HEIGHT);

    buildMesh();
    buildShell();

    return true;
}

void DrumRenderer::initStrikeMarker(){
    const int segments = 100;
    const float radius = 0.02f;
    std::vector<GLfloat> markerVertices;
    markerVertices.push_back(0.0f); // center x
    markerVertices.push_back(0.0f); // center y
    markerVertices.push_back(0.0f); // center z
    for (int i = 0; i < segments; i++) {
        float theta = 2.0f * M_PI * float(i) / float(segments);
        float x = radius * cos(theta);
        float y = radius * sin(theta);
        markerVertices.push_back(x);
        markerVertices.push_back(y); // y
        markerVertices.push_back(0.0f);
    }
    markerVertexCount = segments + 1;
    glGenVertexArrays(1, &marker_vao);
    glBindVertexArray(marker_vao);
    glGenBuffers(1, &marker_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, marker_vbo);
    glBufferData(GL_ARRAY_BUFFER, markerVertices.size() * sizeof(GLfloat), markerVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
void DrumRenderer::buildMesh() {
    int nRadial  = gridR_;  // Nr
    int nAngular = gridTH_;  // Ntheta

    float radius = 1.0f; // normalized radius in NDC

    // Center vertex (index 0)
    // position
    vertices_.push_back(0.0f); // x
    vertices_.push_back(0.0f); // y (displacement)
    vertices_.push_back(0.0f); // z
    // color
    vertices_.push_back(0.0f);
    vertices_.push_back(0.0f);
    vertices_.push_back(0.0f);
    // TexCoord
    vertices_.push_back(0.5f);
    vertices_.push_back(0.5f);
    // normal
    vertices_.push_back(0.0f);
    vertices_.push_back(1.0f);
    vertices_.push_back(0.0f);

    // Build rings: r goes from 1..nRadial (ring 0 = innermost, ring nRadial-1 = outermost)
    for (int r = 0; r < nRadial; r++) {
        float rNorm = (float)(r + 1) / nRadial * radius; // radius of this ring

        for (int a = 0; a < nAngular; a++) {
            float theta = 2.0f * M_PI * (float)a / nAngular;

            float x = rNorm * cos(theta);
            float z = rNorm * sin(theta);
            float y = 0.0f; // displaced by simulation later

            // position
            vertices_.push_back(x);
            vertices_.push_back(y);
            vertices_.push_back(z);
            // color
            vertices_.push_back(0.0f);
            vertices_.push_back(0.0f);
            vertices_.push_back(0.0f);
            // TexCoord
            vertices_.push_back(0.5f + 0.5f * cos(theta) * rNorm);
            vertices_.push_back(0.5f + 0.5f * sin(theta) * rNorm);
            // normal
            vertices_.push_back(0.0f);
            vertices_.push_back(1.0f);
            vertices_.push_back(0.0f);
        }
    }

    // --- Indices ---

    // Inner ring: fan from center (vertex 0) to first ring (indices 1..nAngular)
    for (int a = 0; a < nAngular; a++) {
        int curr = 1 + a;
        int next = 1 + (a + 1) % nAngular;
        indices_.push_back(0);
        indices_.push_back(curr);
        indices_.push_back(next);
    }

    // Remaining rings: quads between ring r and ring r+1
    // vertex index for ring r, angle a = 1 + r*nAngular + a
    for (int r = 0; r < nRadial - 1; r++) {
        for (int a = 0; a < nAngular; a++) {
            int aNext = (a + 1) % nAngular;

            int bl = 1 + r       * nAngular + a;
            int br = 1 + r       * nAngular + aNext;
            int tl = 1 + (r + 1) * nAngular + a;
            int tr = 1 + (r + 1) * nAngular + aNext;

            // triangle 1
            indices_.push_back(bl);
            indices_.push_back(tl);
            indices_.push_back(br);
            // triangle 2
            indices_.push_back(br);
            indices_.push_back(tl);
            indices_.push_back(tr);
        }
    }

    createBuffers(vertices_.data(), vertices_.size() * sizeof(GLfloat),
                  indices_.data(), indices_.size() * sizeof(GLuint));
    setupVertexAttributes();
}


void DrumRenderer::buildShell() {
    const float radius = 1.0f;  // matches membrane radius
    const int nAngular = gridTH_;
    const int numRings = 10; // top and bottom

    // Two rings: top (y=0, flush with membrane edge) and bottom (y=-shellLength_)
    for (int ring = 0; ring < numRings; ring++) {
        float t = (float)ring / (numRings - 1);
        float y = -t * shellLength_;

        for (int a = 0; a < nAngular; a++) {
            float theta = 2.0f * M_PI * (float)a / nAngular;
            float x  = radius * cos(theta);
            float z  = radius * sin(theta);
            float nx = cos(theta);  // outward radial normal
            float nz = sin(theta);

            shellVertices_.insert(shellVertices_.end(), {
                x, y, z,                          // position
                0.0f, 0.0f, 0.0f,                 // color
                (float)a / nAngular, (float)ring,  // texcoord
                nx, 0.0f, nz                       // normal
            });
        }
    }

    // Quad strip connecting each adjacent pair of rings
    for (int ring = 0; ring < numRings - 1; ring++) {
        GLuint topBase = ring * nAngular;
        GLuint botBase = (ring + 1) * nAngular;
        for (int a = 0; a < nAngular; a++) {
            int aNext = (a + 1) % nAngular;
            GLuint t0 = topBase + a,      t1 = topBase + aNext;
            GLuint b0 = botBase + a,      b1 = botBase + aNext;

            shellIndices_.insert(shellIndices_.end(), { t0, b0, t1 });
            shellIndices_.insert(shellIndices_.end(), { t1, b0, b1 });
        }
    }

    shellIndexCount_ = (GLsizei)shellIndices_.size();

    glGenVertexArrays(1, &shell_vao);
    glBindVertexArray(shell_vao);

    glGenBuffers(1, &shell_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, shell_vbo);
    glBufferData(GL_ARRAY_BUFFER, shellVertices_.size() * sizeof(GLfloat), shellVertices_.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &shell_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shell_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, shellIndices_.size() * sizeof(GLuint), shellIndices_.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void DrumRenderer::drawShell() {
    glBindVertexArray(shell_vao);
    glDrawElements(GL_TRIANGLE_STRIP_ADJACENCY, shellIndexCount_, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

bool DrumRenderer::shouldClose() const
{
    return glfwWindowShouldClose(window);
}

void DrumRenderer::compileShaders(const char *vertexFile, const char *fragmentFile){
    // Read vertexFile and fragmentFile and store the strings
	std::string vertexCode = getShaderContents(vertexFile);
	std::string fragmentCode = getShaderContents(fragmentFile);

	// Convert the shader source strings into character arrays
	const char* vertexSource = vertexCode.c_str();
	const char* fragmentSource = fragmentCode.c_str();

	// Create Vertex Shader Object and get its reference
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	// Attach Vertex Shader source to the Vertex Shader Object
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	// Compile the Vertex Shader into machine code
	glCompileShader(vertexShader);
	// Checks if Shader compiled succesfully
	compileErrors(vertexShader, "VERTEX");

	// Create Fragment Shader Object and get its reference
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	// Attach Fragment Shader source to the Fragment Shader Object
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	// Compile the Vertex Shader into machine code
	glCompileShader(fragmentShader);
	// Checks if Shader compiled succesfully
	compileErrors(fragmentShader, "FRAGMENT");

	// Create Shader Program Object and get its reference
	shaderProgramID = glCreateProgram();
	// Attach the Vertex and Fragment Shaders to the Shader Program
	glAttachShader(shaderProgramID, vertexShader);
	glAttachShader(shaderProgramID, fragmentShader);
	// Wrap-up/Link all the shaders together into the Shader Program
	glLinkProgram(shaderProgramID);
	// Checks if Shaders linked succesfully
	compileErrors(shaderProgramID, "PROGRAM");

	// Delete the now useless Vertex and Fragment Shader objects
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

std::string DrumRenderer::getShaderContents(const char* filename)
{
	std::ifstream in(filename);
	if (in)
	{
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return(contents);
	}
	throw(errno);
}

void DrumRenderer::compileErrors(unsigned int shader, const char* type)
{
    GLint hasCompiled;
    char infoLog[1024];

    if (type != std::string("PROGRAM"))
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
        if (hasCompiled == GL_FALSE)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "SHADER_COMPILATION_ERROR for: " << type << "\n" << infoLog << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
        if (hasCompiled == GL_FALSE)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "SHADER_LINKING_ERROR for: " << type << "\n" << infoLog << std::endl;
        }
    }
}

// Shader activation and deletion
void DrumRenderer::activateShaderProgram()
{
    glUseProgram(shaderProgramID);
}

GLuint DrumRenderer::getShaderProgramID() const
{
    return shaderProgramID;
}

void DrumRenderer::updateCircularVertexData(const std::vector<GLfloat>& gridData) {
    int nRadial  = gridR_;
    int nAngular = gridTH_;

    // Center vertex stays at y=0 (fixed boundary at edge, free at center)
    // vertices_[1] = 0.0f; // already zero

    // Update each ring vertex's y from the polar grid data
    // gridData is indexed as gridData[r * nAngular + a]

    for (int r = 0; r < nRadial; r++) {
        for (int a = 0; a < nAngular; a++) {
            int vertexIdx  = 1 + r * nAngular + a;     // vertex index in vertices_
            int vertexStart = vertexIdx * 11;           // 11 floats per vertex
            vertices_[vertexStart + 1] = gridData[r * nAngular + a]; // y = displacement
        }
    }

    bindVBO();
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_.size() * sizeof(GLfloat), vertices_.data());
    unbindVBO();
}

void DrumRenderer::updateVertexData(const std::vector<GLfloat>& gridData)
{
	for (int i = 0; i < gridX; i++) {
		for (int j = 0; j < gridY; j++) {
			int vertexStart = (i * gridX + j) * 11;
			vertices_[vertexStart + 1] = gridData[j + i * gridX];
		}
	}
    bindVBO();
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_.size() * sizeof(GLfloat), vertices_.data());
    unbindVBO();
}

void DrumRenderer::setMatrices(const glm::mat4& model, const glm::mat4& view,const glm::mat4& proj)
{
		int modelLoc = glGetUniformLocation(shaderProgramID, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		int viewLoc = glGetUniformLocation(shaderProgramID, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		int projLoc = glGetUniformLocation(shaderProgramID, "proj");
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

}

// Buffer creation
void DrumRenderer::createBuffers(GLfloat* vertexData, GLsizeiptr vertexSize, GLuint* indexData, GLsizeiptr indexSize)
{
    // Create VAO
    glGenVertexArrays(1, &vao);
    bindVertexArray();

    // Create VBO
    glGenBuffers(1, &vbo);
    bindVBO();
    glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(GLfloat), vertices_.data(), GL_DYNAMIC_DRAW);

    // Create EBO
    glGenBuffers(1, &ebo);
    bindEBO();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(GLuint), indices_.data(), GL_STATIC_DRAW);

    unbindVBO();
    unbindVertexArray();
}

// Setup vertex attributes
void DrumRenderer::setupVertexAttributes()
{
    bindVertexArray();
    bindVBO();

    // Position attribute (layout 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute (layout 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // TexCoord attribute (layout 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Normal attribute (layout 3)
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    unbindVBO();
    unbindVertexArray();
}

// Bind and unbind buffer methods
void DrumRenderer::bindVertexArray()
{
    glBindVertexArray(vao);
}

void DrumRenderer::unbindVertexArray()
{
    glBindVertexArray(0);
}

void DrumRenderer::bindVBO()
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
}

void DrumRenderer::unbindVBO()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DrumRenderer::bindEBO()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
}

void DrumRenderer::unbindEBO()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// Uniform setters
void DrumRenderer::setUniform1f(const char* uniformName, float value)
{
    GLint loc = glGetUniformLocation(shaderProgramID, uniformName);
    glUniform1f(loc, value);
}

void DrumRenderer::setUniform3f(const char* uniformName, float v0, float v1, float v2)
{
    GLint loc = glGetUniformLocation(shaderProgramID, uniformName);
    glUniform3f(loc, v0, v1, v2);
}

void DrumRenderer::setUniform4f(const char* uniformName, float v0, float v1, float v2, float v3)
{
    GLint loc = glGetUniformLocation(shaderProgramID, uniformName);
    glUniform4f(loc, v0, v1, v2, v3);
}

void DrumRenderer::drawElements(){
	bindVertexArray();
	glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
}

void DrumRenderer::drawStrikeMarker(std::queue<StrikeMarker>& markerQueue, float fadeTime){
    std::queue<StrikeMarker> remaining;
    while (!markerQueue.empty()) {
        StrikeMarker marker = markerQueue.front();
        markerQueue.pop();
        float age = glfwGetTime() - marker.spawnTime;
        float opacity = 1.0f - (age / fadeTime);
        if (opacity > 0.0f) {
            glm::mat4 identity(1.0f);
            setMatrices(identity, identity, identity);
            setUniform3f("uMarkerOffset", marker.x, marker.y, 0.0f);
            setUniform4f("uColor", 1.0f, 0.0f, 0.0f, opacity);
            glBindVertexArray(marker_vao);
            glDrawArrays(GL_TRIANGLE_FAN, 0, markerVertexCount);
            glBindVertexArray(0);
            remaining.push(marker);
        }
    }
    markerQueue = std::move(remaining);
    setUniform3f("uMarkerOffset", 0.0f, 0.0f, 0.0f);
}

void DrumRenderer::enableDepthTest()
{
    glEnable(GL_DEPTH_TEST);
}

void DrumRenderer::enableBlending()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void DrumRenderer::setPolygonMode(GLenum face, GLenum mode)
{
    glPolygonMode(face, mode);
}


void DrumRenderer::deleteBuffers(){
	if (vao != 0) {
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}
	if (vbo != 0) {
		glDeleteBuffers(1, &vbo);
		vbo = 0;
	}
	if (ebo != 0) {
		glDeleteBuffers(1, &ebo);
		ebo = 0;
	}

    if (shell_vao != 0) {
        glDeleteVertexArrays(1, &shell_vao);
        shell_vao = 0;
    }
    if (shell_vbo != 0) {
        glDeleteBuffers(1, &shell_vbo);
        shell_vbo = 0;
    }
    if (shell_ebo != 0) {
        glDeleteBuffers(1, &shell_ebo);
        shell_ebo = 0;
    }

    if(marker_vao != 0) {
        glDeleteVertexArrays(1, &marker_vao);
        marker_vao = 0;
    }
    if(marker_vbo != 0) {
        glDeleteBuffers(1, &marker_vbo);
        marker_vbo = 0;
    }
}

void DrumRenderer::deleteShaderProgram()
{
	if (shaderProgramID != 0)
	{
		glDeleteProgram(shaderProgramID);
		shaderProgramID = 0;
	}
}

void DrumRenderer::pollEvents()
{
	glfwPollEvents();
}

void DrumRenderer::setClearColor(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
}

void DrumRenderer::clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void DrumRenderer::swapBuffers()
{
	glfwSwapBuffers(window);
}

GLFWwindow* DrumRenderer::getWindow() const
{
	return window;
}

std::vector<GLfloat>& DrumRenderer::getVertices()
{
    return vertices_;
}

std::vector<GLuint>& DrumRenderer::getIndices()
{
    return indices_;
}

