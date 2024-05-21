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
std::vector<float> generateFlatRingVertices(float radius, float ringWidth, int segments);
std::vector<unsigned int> generateFlatRingIndices(int segments);
void drawOrbit(float radius, int segments, glm::mat4 view, glm::mat4 projection, unsigned int shaderProgram);


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
        glm::vec3(8.0f, 0.0f, 0.0f),   // Merkury
        glm::vec3(11.0f, 0.0f, 0.0f),    // Wenus
        glm::vec3(15.0f, 0.0f, 0.0f),    // Ziemia
        glm::vec3(18.0f, 0.0f, 0.0f),    // Mars
        glm::vec3(25.0f, 0.0f, 0.0f),    // Jowisz
        glm::vec3(35.0f, 0.0f, 0.0f),    // Saturn
        glm::vec3(45.0f, 0.0f, 0.0f),    // Uran
        glm::vec3(53.0f, 0.0f, 0.0f),    // Neptun
        // Add more planet positions as needed
    };
    float size_factor = 0.6f;
    glm::vec3 planetScales[] = {
        glm::vec3(10.0f, 10.0f, 10.0f),   // Slonce
        glm::vec3(size_factor*0.45, size_factor * 0.45, size_factor * 0.45),         // Merkury
        glm::vec3(size_factor * 0.949, size_factor * 0.949, size_factor * 0.949),    // Wenus
        glm::vec3(size_factor, size_factor, size_factor),                            // Ziemia
        glm::vec3(size_factor*0.53, size_factor * 0.53, size_factor * 0.53),         // Mars
        glm::vec3(size_factor * 11.2,size_factor * 11.2, size_factor * 11.2),         // Jowisz
        glm::vec3(size_factor * 9.45, size_factor * 9.45, size_factor * 9.45),       // Saturn
        glm::vec3(size_factor * 4, size_factor * 4, size_factor * 4),                // Uran
        glm::vec3(size_factor * 3.88, size_factor * 3.88, size_factor * 3.88),       // Neptun
        // Dodaj więcej współczynników skalowania dla kolejnych planet
    };
    float speed_factor = 40.0f;
    float orbitSpeeds[] = {
        0.0f,
        speed_factor*4,   // merkury
        speed_factor * 1.62,  // wenus
        speed_factor,  // ziemia
        speed_factor * 0.53,  // mars
        speed_factor * 0.084f,  // jowisz
        speed_factor * 0.033 ,  // saturn
        speed_factor * 0.012,  // uran
        speed_factor * 0.006,  // neptun
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
    float glowRadius = 10.0f; // Na przykład promień 2 jednostek

    glm::vec3 moonPosition = glm::vec3(10.0f, 0.0f, 0.0f); // Pozycja Ziemi
    float moonOrbitRadius = 1.0f; // Promień orbity Księżyca
    float moonOrbitSpeed = speed_factor * 13.36f; // Szybkość orbity Księżyca
    float moonOrbitAngle = 0.0f;

    // Inicjalizacja płaskiego pierścienia Saturna
    std::vector<float> flatRingVertices = generateFlatRingVertices(7.5f * size_factor, 1.7f, 36);
    std::vector<unsigned int> flatRingIndices = generateFlatRingIndices(36);

    unsigned int flatRingVBO, flatRingVAO, flatRingEBO;
    glGenVertexArrays(1, &flatRingVAO);
    glGenBuffers(1, &flatRingVBO);
    glGenBuffers(1, &flatRingEBO);

    glBindVertexArray(flatRingVAO);

    glBindBuffer(GL_ARRAY_BUFFER, flatRingVBO);
    glBufferData(GL_ARRAY_BUFFER, flatRingVertices.size() * sizeof(float), flatRingVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, flatRingEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, flatRingIndices.size() * sizeof(unsigned int), flatRingIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);



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

        // Update Moon's orbit angle
        moonOrbitAngle += moonOrbitSpeed * deltaTime;
        if (moonOrbitAngle > 360.0f) {
            moonOrbitAngle -= 360.0f;
        }

        // Render the orbits
        for (unsigned int i = 1; i < sizeof(planetPositions) / sizeof(glm::vec3); ++i) {
            drawOrbit(glm::length(planetPositions[i]), 100, view, projection, shaderProgram);
        }

        // Render the planets
        for (unsigned int i = 0; i < sizeof(planetPositions) / sizeof(glm::vec3); ++i) {
            glm::mat4 model = glm::mat4(1.0f);

            // Orbita
            if (i > 0) {
                float orbitRadius = glm::length(planetPositions[i]);
                float x = cos(glm::radians(orbitAngles[i])) * orbitRadius;
                float z = sin(glm::radians(orbitAngles[i])) * orbitRadius;
                model = glm::translate(model, glm::vec3(x, 0.0f, z));
            }
            else {
                model = glm::translate(model, planetPositions[i]);
            }
            model = glm::scale(model, planetScales[i]);
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniform3fv(glGetUniformLocation(shaderProgram, "planetColor"), 1, glm::value_ptr(planetColors[i]));
            if (i == 0) {
                glUniform1i(glGetUniformLocation(shaderProgram, "isSun"), 1);
            }
            else {
                glUniform1i(glGetUniformLocation(shaderProgram, "isSun"), 0);
            }
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

            // Renderowanie pierścienia Saturna
            if (i == 6) {
                glm::mat4 ringModel = glm::mat4(1.0f);
                float saturnOrbitRadius = glm::length(planetPositions[6]);
                float saturnX = cos(glm::radians(orbitAngles[6])) * saturnOrbitRadius;
                float saturnZ = sin(glm::radians(orbitAngles[6])) * saturnOrbitRadius;
                ringModel = glm::translate(ringModel, glm::vec3(saturnX, 0.0f, saturnZ));
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(ringModel));
                glBindVertexArray(flatRingVAO);
                glDrawElements(GL_TRIANGLES, flatRingIndices.size(), GL_UNSIGNED_INT, 0);
            }

            // Renderowanie Księżyca
            if (i == 3) {
                glm::mat4 moonModel = glm::mat4(1.0f);
                float earthOrbitRadius = glm::length(planetPositions[3]);
                float earthX = cos(glm::radians(orbitAngles[3])) * earthOrbitRadius;
                float earthZ = sin(glm::radians(orbitAngles[3])) * earthOrbitRadius;
                float moonX = cos(glm::radians(moonOrbitAngle)) * moonOrbitRadius;
                float moonZ = sin(glm::radians(moonOrbitAngle)) * moonOrbitRadius;
                moonModel = glm::translate(moonModel, glm::vec3(earthX + moonX, 0.0f, earthZ + moonZ));
                moonModel = glm::scale(moonModel, glm::vec3(size_factor * 0.273f, size_factor * 0.273f, size_factor * 0.273f));
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(moonModel));
                glUniform3fv(glGetUniformLocation(shaderProgram, "planetColor"), 1, glm::value_ptr(glm::vec3(0.8f, 0.8f, 0.8f)));
                glUniform1i(glGetUniformLocation(shaderProgram, "isSun"), 0);
                glBindVertexArray(VAO);
                glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
            }
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteVertexArrays(1, &flatRingVAO);
    glDeleteBuffers(1, &flatRingVBO);
    glDeleteBuffers(1, &flatRingEBO);
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

    float cameraSpeed = 10.0f * deltaTime; // Adjust accordingly
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

std::vector<float> generateFlatRingVertices(float radius, float ringWidth, int segments) {
    std::vector<float> vertices;
    float innerRadius = radius - ringWidth / 2.0f;
    float outerRadius = radius + ringWidth / 2.0f;

    for (int i = 0; i <= segments; ++i) {
        float theta = i * 2.0f * glm::pi<float>() / segments;
        float cosTheta = cos(theta);
        float sinTheta = sin(theta);

        // Outer ring vertex
        vertices.push_back(outerRadius * cosTheta);
        vertices.push_back(0.0f); // flat ring, so y is 0
        vertices.push_back(outerRadius * sinTheta);

        // Inner ring vertex
        vertices.push_back(innerRadius * cosTheta);
        vertices.push_back(0.0f); // flat ring, so y is 0
        vertices.push_back(innerRadius * sinTheta);
    }
    return vertices;
}

std::vector<unsigned int> generateFlatRingIndices(int segments) {
    std::vector<unsigned int> indices;
    for (int i = 0; i < segments; ++i) {
        int first = i * 2;
        int second = first + 1;
        int nextFirst = (first + 2) % (2 * (segments + 1));
        int nextSecond = (second + 2) % (2 * (segments + 1));

        indices.push_back(first);
        indices.push_back(nextFirst);
        indices.push_back(second);

        indices.push_back(second);
        indices.push_back(nextFirst);
        indices.push_back(nextSecond);
    }
    return indices;
}

void drawOrbit(float radius, int segments, glm::mat4 view, glm::mat4 projection, unsigned int shaderProgram) {
    std::vector<float> vertices;
    for (int i = 0; i <= segments; ++i) {
        float theta = i * 2.0f * glm::pi<float>() / segments;
        float x = radius * cos(theta);
        float z = radius * sin(theta);
        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(z);
    }

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Use the shader program
    glUseProgram(shaderProgram);

    // Set view and projection matrices
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(shaderProgram, "planetColor"), 1, glm::value_ptr(glm::vec3(1.5f, 1.5f, 1.5f)));

    // Set the model matrix to identity
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // Draw the orbit
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_LOOP, 0, segments);

    // Clean up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
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
