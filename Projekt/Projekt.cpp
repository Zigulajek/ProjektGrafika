#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

// Function declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
unsigned int loadShader(const char* vertexPath, const char* fragmentPath);

// Camera settings
glm::vec3 cameraPos = glm::vec3(18.0f, 50.0f, 20.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw = -90.0f;
float pitch = -80.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0f / 2.0;
float fov = 45.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window
    GLFWwindow* window = glfwCreateWindow(1600, 1200, "Solar System", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Load OpenGL functions using GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Set viewport
    glViewport(0, 0, 1600, 1200);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Set input callbacks
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Capture the mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Build and compile shaders
    unsigned int shaderProgram = loadShader("vertex_shader.glsl", "fragment_shader.glsl");

    // Define vertices for the planets and the sun (for simplicity, we use a sphere for each)
    float radius = 0.5f;
    int sectorCount = 36;
    int stackCount = 18;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    for (int i = 0; i <= stackCount; ++i) {
        float theta = i * glm::pi<float>() / stackCount;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (int j = 0; j <= sectorCount; ++j) {
            float phi = j * 2 * glm::pi<float>() / sectorCount;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            float x = cosPhi * sinTheta;
            float y = cosTheta;
            float z = sinPhi * sinTheta;

            // vertex position
            vertices.push_back(radius * x);
            vertices.push_back(radius * y);
            vertices.push_back(radius * z);

            // normal vector
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }

    // Generate indices
    for (int i = 0; i < stackCount; ++i) {
        for (int j = 0; j < sectorCount; ++j) {
            int first = (i * (sectorCount + 1)) + j;
            int second = first + sectorCount + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    // Convert vertices and indices vectors to arrays
    float* sphereVertices = vertices.data();
    unsigned int* sphereIndices = indices.data();

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), sphereVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), sphereIndices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Lighting settings
    glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

    // Define positions for the planets
    glm::vec3 planetPositions[] = {
        glm::vec3(0.0f, 0.0f, 0.0f),   // Slonce
        glm::vec3(5.0f, 0.0f, 0.0f),   // Merkury
        glm::vec3(8.0f, 0.0f, 0.0f),    // Wenus
        glm::vec3(10.0f, 0.0f, 0.0f),    // Ziemia
        glm::vec3(13.0f, 0.0f, 0.0f),    // Mars
        glm::vec3(17.0f, 0.0f, 0.0f),    // Jowisz
        glm::vec3(22.0f, 0.0f, 0.0f),    // Saturn
        glm::vec3(27.0f, 0.0f, 0.0f),    // Uran
        glm::vec3(32.0f, 0.0f, 0.0f),    // Neptun
        // Add more planet positions as needed
    };

    glm::vec3 planetScales[] = {
        glm::vec3(5.0f, 5.0f, 5.0f),   // Slonce
        glm::vec3(0.25f, 0.25f, 0.25f),   // Merkury
        glm::vec3(0.45f, 0.45f, 0.45f),    // Wenus
        glm::vec3(0.5f, 0.5f, 0.5f),    // Ziemia
        glm::vec3(0.3f, 0.3f, 0.3f),    // Mars
        glm::vec3(2.5f, 2.5f, 2.5f),    // Jowisz
        glm::vec3(2.0f, 2.0f, 2.0f),    // Saturn
        glm::vec3(1.5f, 1.5f, 1.5f),    // Uran
        glm::vec3(1.5f, 1.5f, 1.5f),    // Neptun
        // Dodaj więcej współczynników skalowania dla kolejnych planet
    };
    float speed_factor = 40.0f;
    float orbitSpeeds[] = {
        0.0f,
        speed_factor * 4,   // merkury
        speed_factor * 1.62,  // wenus
        speed_factor,  // ziemia
        speed_factor * 0.53,  // mars
        speed_factor * 0.084f,  // jowisz
        speed_factor * 0.033 ,  // saturn
        speed_factor * 0.012,  // uran
        speed_factor * 0,006,  // neptun
    };

    float orbitAngles[] = {
        0.0f,   // Słońce
        0.0f,   // Merkury
        0.0f,   // Wenus
        0.0f,   // Ziemia
        0.0f,   // Mars
        0.0f,   // Jowisz
        0.0f,   // Saturn
        0.0f,   // Uran
        0.0f    // Neptun
    };

    glm::vec3 planetColors[] = {
    glm::vec3(1.0f, 1.0f, 0.0f),  // Słońce (żółty)
    glm::vec3(0.5f, 0.5f, 0.5f),  // Merkury (szary)
    glm::vec3(1.0f, 0.5f, 0.0f),  // Wenus (pomarańczowy)
    glm::vec3(0.0f, 0.0f, 1.0f),  // Ziemia (niebieski)
    glm::vec3(1.0f, 0.0f, 0.0f),  // Mars (czerwony)
    glm::vec3(1.0f, 1.0f, 1.0f),  // Jowisz (biały)
    glm::vec3(1.0f, 1.0f, 0.5f),  // Saturn (jasno żółty)
    glm::vec3(0.5f, 0.5f, 1.0f),  // Uran (jasno niebieski)
    glm::vec3(0.0f, 0.0f, 0.5f),  // Neptun (ciemno niebieski)
    // Dodaj więcej kolorów dla innych planet
    };

    glm::vec3 sunColor = glm::vec3(1.0f, 1.0f, 0.0f); // Na przykład żółty
    float glowRadius = 2.0f; // Na przykład promień 2 jednostek

    // Render loop
   // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        processInput(window);

        // Render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activate shader
        glUseProgram(shaderProgram);

        // Set lighting uniforms
        glUniform3f(glGetUniformLocation(shaderProgram, "light.position"), lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);

        // Light properties
        glUniform3f(glGetUniformLocation(shaderProgram, "light.ambient"), 0.2f, 0.2f, 0.2f);
        glUniform3f(glGetUniformLocation(shaderProgram, "light.diffuse"), 0.5f, 0.5f, 0.5f);
        glUniform3f(glGetUniformLocation(shaderProgram, "light.specular"), 1.0f, 1.0f, 1.0f);

        // Material properties
        glUniform3f(glGetUniformLocation(shaderProgram, "material.ambient"), 1.0f, 0.5f, 0.31f);
        glUniform3f(glGetUniformLocation(shaderProgram, "material.diffuse"), 1.0f, 0.5f, 0.31f);
        glUniform3f(glGetUniformLocation(shaderProgram, "material.specular"), 0.1f, 0.1f, 0.1f);
        glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), 4.0f);

        // Set sun glow properties
        glUniform3fv(glGetUniformLocation(shaderProgram, "sunColor"), 1, glm::value_ptr(sunColor));
        glUniform1f(glGetUniformLocation(shaderProgram, "glowRadius"), glowRadius);

        // View/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraFront + cameraPos, cameraUp);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

        // Update orbit angles
        for (unsigned int i = 1; i < sizeof(planetPositions) / sizeof(glm::vec3); ++i) {
            orbitAngles[i] += orbitSpeeds[i] * deltaTime;
            if (orbitAngles[i] > 360.0f) {
                orbitAngles[i] -= 360.0f;
            }
        }

        // Render the planets
        for (unsigned int i = 0; i < sizeof(planetPositions) / sizeof(glm::vec3); ++i) {
            glm::mat4 model = glm::mat4(1.0f);

            // Apply translation and rotation for orbiting
            if (i > 0) {
                float orbitRadius = glm::length(planetPositions[i]);
                float x = cos(glm::radians(orbitAngles[i])) * orbitRadius;
                float z = sin(glm::radians(orbitAngles[i])) * orbitRadius;
                model = glm::translate(model, glm::vec3(x, 0.0f, z));
            }
            else {
                model = glm::translate(model, planetPositions[i]);
            }

            model = glm::scale(model, planetScales[i]); // Apply scaling
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

            // Ustaw kolor planety
            glUniform3fv(glGetUniformLocation(shaderProgram, "planetColor"), 1, glm::value_ptr(planetColors[i]));

            // Set whether this is the Sun
            if (i == 0) {
                glUniform1i(glGetUniformLocation(shaderProgram, "isSun"), 1);
            }
            else {
                glUniform1i(glGetUniformLocation(shaderProgram, "isSun"), 0);
            }

            // Render a sphere for each planet
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        }

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}

// Process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 5.5f * deltaTime; // Adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

// GLFW: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// GLFW: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// GLFW: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (fov >= 1.0f && fov <= 45.0f)
        fov -= yoffset;
    if (fov <= 1.0f)
        fov = 1.0f;
    if (fov >= 45.0f)
        fov = 45.0f;
}

// Utility function for loading a shader
unsigned int loadShader(const char* vertexPath, const char* fragmentPath) {
    // Retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    // Ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        // Open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // Read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // Close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // Convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    // Compile shaders
    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // Vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    // Print compile errors if any
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
            << infoLog << std::endl;
    }

    // Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    // Print compile errors if any
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
            << infoLog << std::endl;
    }

    // Shader Program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertex);
    glAttachShader(shaderProgram, fragment);
    glLinkProgram(shaderProgram);
    // Print linking errors if any
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
            << infoLog << std::endl;
    }
    // Delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return shaderProgram;
}
