#include "renderer.hpp"
#include "simDefs.hpp"

// Constructor
Renderer::Renderer(unsigned int width, unsigned int height, const char* windowTitle)
    : window(nullptr), windowWidth(width), windowHeight(height), 
      windowTitle(windowTitle), shaderProgramID(0), VAO_ID(0), VBO_ID(0), EBO_ID(0),
      gridX(0), gridY(0)
{
}

// Destructor
Renderer::~Renderer()
{
    deleteBuffers();
    deleteShaderProgram();
    if (window)
    {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

// Initialize GLFW and create window
bool Renderer::initialize()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(windowWidth, windowHeight, windowTitle.c_str(), NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    setViewport(0, 0, windowWidth, windowHeight);

    return true;
}

// Window Management
bool Renderer::shouldClose() const
{
    return glfwWindowShouldClose(window);
}

void Renderer::swapBuffers()
{
    glfwSwapBuffers(window);
}

void Renderer::pollEvents()
{
    glfwPollEvents();
}

void Renderer::setViewport(int x, int y, unsigned int width, unsigned int height)
{
    glViewport(x, y, width, height);
}

// Get file contents for shader loading
std::string Renderer::getFileContents(const char* filename)
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
        return contents;
    }
    throw(errno);
}

// Compile shaders
void Renderer::compileShaders(const char* vertexFile, const char* fragmentFile)
{
    // Read shader files
    std::string vertexCode = getFileContents(vertexFile);
    std::string fragmentCode = getFileContents(fragmentFile);

    const char* vertexSource = vertexCode.c_str();
    const char* fragmentSource = fragmentCode.c_str();

    // Compile Vertex Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    compileErrors(vertexShader, "VERTEX");

    // Compile Fragment Shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    compileErrors(fragmentShader, "FRAGMENT");

    // Link Shader Program
    shaderProgramID = glCreateProgram();
    glAttachShader(shaderProgramID, vertexShader);
    glAttachShader(shaderProgramID, fragmentShader);
    glLinkProgram(shaderProgramID);
    compileErrors(shaderProgramID, "PROGRAM");

    // Clean up
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

// Check for shader compilation errors
void Renderer::compileErrors(unsigned int shader, const char* type)
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
void Renderer::activateShaderProgram()
{
    glUseProgram(shaderProgramID);
}

void Renderer::deleteShaderProgram()
{
    if (shaderProgramID != 0)
    {
        glDeleteProgram(shaderProgramID);
        shaderProgramID = 0;
    }
}

GLuint Renderer::getShaderProgramID() const
{
    return shaderProgramID;
}

// Buffer creation
void Renderer::createBuffers(GLfloat* vertexData, GLsizeiptr vertexSize, GLuint* indexData, GLsizeiptr indexSize)
{
    // Create VAO
    glGenVertexArrays(1, &VAO_ID);
    bindVertexArray();

    // Create VBO
    glGenBuffers(1, &VBO_ID);
    bindVBO();
    glBufferData(GL_ARRAY_BUFFER, vertexSize, vertexData, GL_DYNAMIC_DRAW);

    // Create EBO
    glGenBuffers(1, &EBO_ID);
    bindEBO();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize, indexData, GL_STATIC_DRAW);

    unbindVBO();
    unbindVertexArray();
}

// Setup vertex attributes
void Renderer::setupVertexAttributes()
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
void Renderer::bindVertexArray()
{
    glBindVertexArray(VAO_ID);
}

void Renderer::unbindVertexArray()
{
    glBindVertexArray(0);
}

void Renderer::bindVBO()
{
    glBindBuffer(GL_ARRAY_BUFFER, VBO_ID);
}

void Renderer::unbindVBO()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::bindEBO()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_ID);
}

void Renderer::unbindEBO()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// Delete buffers
void Renderer::deleteBuffers()
{
    if (VAO_ID != 0)
    {
        glDeleteVertexArrays(1, &VAO_ID);
        VAO_ID = 0;
    }
    if (VBO_ID != 0)
    {
        glDeleteBuffers(1, &VBO_ID);
        VBO_ID = 0;
    }
    if (EBO_ID != 0)
    {
        glDeleteBuffers(1, &EBO_ID);
        EBO_ID = 0;
    }
}

// Initialize grid mesh
void Renderer::initializeGridMesh(int gridX, int gridY)
{
    this->gridX = gridX;
    this->gridY = gridY;

    vertices.clear();
    indices.clear();

    // Generate vertices for grid
    for (int i = 0; i < gridX; i++)
    {
        for (int j = 0; j < gridY; j++)
        {
            float x = (float)j / (gridX - 1) * 2.0f - 1.0f;
            float z = (float)i / (gridY - 1) * 2.0f - 1.0f;
            float y = 0.0f;

            // Position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Color (default black)
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);

            // TexCoord
            vertices.push_back((float)j / (gridY - 1));
            vertices.push_back((float)i / (gridX - 1));

            // Normal (default pointing up)
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);
            vertices.push_back(0.0f);
        }
    }

    // Generate indices for grid
    for (int i = 0; i < gridX - 1; i++)
    {
        for (int j = 0; j < gridY - 1; j++)
        {
            GLuint topLeft = i * gridX + j;
            GLuint topRight = i * gridX + j + 1;
            GLuint bottomLeft = (i + 1) * gridX + j;
            GLuint bottomRight = (i + 1) * gridX + j + 1;

            // Triangle 1
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            // Triangle 2
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    createBuffers(vertices.data(), vertices.size() * sizeof(GLfloat),
                  indices.data(), indices.size() * sizeof(GLuint));
    setupVertexAttributes();
}

// Update vertex data
void Renderer::updateVertexData(const std::vector<GLfloat>& vertexData)
{
    bindVBO();
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexData.size() * sizeof(GLfloat), vertexData.data());
    unbindVBO();
}

// Rendering state
void Renderer::setClearColor(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
}

void Renderer::clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::drawElements(GLsizei indexCount)
{
    bindVertexArray();
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}

// Render state options
void Renderer::enableDepthTest()
{
    glEnable(GL_DEPTH_TEST);
}

void Renderer::enableBlending()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Renderer::setPolygonMode(GLenum face, GLenum mode)
{
    glPolygonMode(face, mode);
}

// Transform matrices
void Renderer::setModelMatrix(const glm::mat4& model)
{
    int modelLoc = glGetUniformLocation(shaderProgramID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
}

void Renderer::setViewMatrix(const glm::mat4& view)
{
    int viewLoc = glGetUniformLocation(shaderProgramID, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void Renderer::setProjectionMatrix(const glm::mat4& projection)
{
    int projLoc = glGetUniformLocation(shaderProgramID, "proj");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

// Uniform setters
void Renderer::setUniform1f(const char* uniformName, float value)
{
    GLint loc = glGetUniformLocation(shaderProgramID, uniformName);
    glUniform1f(loc, value);
}

void Renderer::setUniformMatrix4fv(const char* uniformName, const glm::mat4& matrix)
{
    GLint loc = glGetUniformLocation(shaderProgramID, uniformName);
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(matrix));
}

// Getters
GLFWwindow* Renderer::getWindow() const
{
    return window;
}

int Renderer::getWindowWidth() const
{
    return windowWidth;
}

int Renderer::getWindowHeight() const
{
    return windowHeight;
}

const std::vector<GLfloat>& Renderer::getVertices() const
{
    return vertices;
}

const std::vector<GLuint>& Renderer::getIndices() const
{
    return indices;
}
