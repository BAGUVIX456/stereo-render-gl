#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader/shader.hpp"
#include "camera/camera.hpp"
#include "model/model.h"
#include <glm/trigonometric.hpp>
#include <iostream>
#include <vector>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const unsigned int FISH_COUNT = 1;
const float CAMERA_OFFSET = 0.0325f;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastFrame_fps = 0.0f;
int frameCount = 0;

void measure_frame_time(float currentFrame);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadCubemap(const std::vector<std::string>& faces);
void render_scene(Shader& skyboxShader, unsigned int skyboxVAO, unsigned int skyboxTexture, Shader& shaderProgram, const glm::vec3 offsets[], Model& fishy, bool isLeftEye, glm::vec3 offset);
glm::mat4 get_frustum(bool isLeftEye);
void setupFramebuffer(unsigned int& fbo, unsigned int& texture, unsigned int& rbo);
void setupQuad(unsigned int& vao, unsigned int& vbo, const float* vertices, size_t size);
void setupSkybox(unsigned int& vao, unsigned int& vbo, const float* vertices, size_t size);

int main()
{
    // Initialize GLFW and create window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH * 2, SCR_HEIGHT, "stereo stuff", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_MULTISAMPLE);
    glfwSwapInterval(0);

    // Load shaders
    Shader shaderProgram("shaders/shader.vert.glsl", "shaders/shader.frag.glsl");
    Shader skyboxShader("shaders/skybox.vert.glsl", "shaders/skybox.frag.glsl");
    Shader quadShader("shaders/quad.vert.glsl", "shaders/quad.frag.glsl");

    // Load model
    stbi_set_flip_vertically_on_load(false);
    Model fishy("./resources/fishy/fish.obj");

    // Setup skybox
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f
    };

    unsigned int skyboxVAO, skyboxVBO;
    setupSkybox(skyboxVAO, skyboxVBO, skyboxVertices, sizeof(skyboxVertices));

    // Setup quads for left and right eye
    float quadVertices_left[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         0.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         0.0f, -1.0f,  1.0f, 0.0f,
         0.0f,  1.0f,  1.0f, 1.0f
    };

    float quadVertices_right[] = {
        // positions   // texCoords
        0.0f,  1.0f,  0.0f, 1.0f,
        0.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        0.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };

    unsigned int quadVAO_left, quadVAO_right, quadVBO_left, quadVBO_right;
    setupQuad(quadVAO_left, quadVBO_left, quadVertices_left, sizeof(quadVertices_left));
    setupQuad(quadVAO_right, quadVBO_right, quadVertices_right, sizeof(quadVertices_right));

    // Setup framebuffers for left and right eye
    unsigned int frame_left, frame_right, texture_left, texture_right, rbo_left, rbo_right;
    setupFramebuffer(frame_left, texture_left, rbo_left);
    setupFramebuffer(frame_right, texture_right, rbo_right);

    // Load cubemap
    std::vector<std::string> faces = {
        "./resources/blue/right.png",
        "./resources/blue/left.png",
        "./resources/blue/top.png",
        "./resources/blue/bottom.png",
        "./resources/blue/front.png",
        "./resources/blue/back.png"
    };

    unsigned int skyboxTexture = loadCubemap(faces);

    // Fish positions
    glm::vec3 offsets[FISH_COUNT] = { glm::vec3(1.0f, 1.0f, -7.0f) };

    // Set light properties
    shaderProgram.use();
    shaderProgram.setVec3("light.position", 0.0f, 0.0f, 0.0f);
    shaderProgram.setVec3("light.ambient", 0.5f, 0.5f, 0.5f);
    shaderProgram.setVec3("light.diffuse", 0.8f, 0.8f, 0.8f);
    shaderProgram.setVec3("light.specular", 0.5f, 0.5f, 0.5f);
    shaderProgram.setFloat("material.shininess", 64);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        measure_frame_time(currentFrame);
        lastFrame = currentFrame;

        processInput(window);

        // Render to left framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, frame_left);
        render_scene(skyboxShader, skyboxVAO, skyboxTexture, shaderProgram, offsets, fishy, true, glm::vec3(-CAMERA_OFFSET, 0.0f, 0.0f));

        // Render to right framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, frame_right);
        render_scene(skyboxShader, skyboxVAO, skyboxTexture, shaderProgram, offsets, fishy, false, glm::vec3(CAMERA_OFFSET, 0.0f, 0.0f));

        // Render quads to screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        quadShader.use();
        glBindVertexArray(quadVAO_left);
        glBindTexture(GL_TEXTURE_2D, texture_left);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindVertexArray(quadVAO_right);
        glBindTexture(GL_TEXTURE_2D, texture_right);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// Function definitions
void measure_frame_time(float currentFrame) {
    frameCount++;
    if (currentFrame - lastFrame_fps >= 1.0f) {
        std::cout << "FrameTime: " << ((currentFrame - lastFrame_fps) / double(frameCount)) * 1000.0f << std::endl;
        frameCount = 0;
        lastFrame_fps = currentFrame;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

unsigned int loadCubemap(const std::vector<std::string>& faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void render_scene(Shader& skyboxShader, unsigned int skyboxVAO, unsigned int skyboxTexture, Shader& shaderProgram, const glm::vec3 offsets[], Model& fishy, bool isLeftEye, glm::vec3 offset) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = get_frustum(isLeftEye);
    glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix(glm::vec3(0.0f))));

    glDepthMask(GL_FALSE);
    skyboxShader.use();
    skyboxShader.setMat4("view", view);
    skyboxShader.setMat4("projection", projection);

    glBindVertexArray(skyboxVAO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);

    shaderProgram.use();
    view = camera.GetViewMatrix(offset);
    shaderProgram.setMat4("view", view);
    shaderProgram.setMat4("projection", projection);
    shaderProgram.setFloat("_Time", glfwGetTime());

    for (int i = 0; i < FISH_COUNT; i++) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, offsets[i]);
        shaderProgram.setMat4("model", model);
        fishy.Draw(shaderProgram);
    }
}

glm::mat4 get_frustum(bool isLeftEye) {
    float fov = glm::radians(camera.Zoom);
    float aspect_ratio = (float)SCR_WIDTH/(float)SCR_HEIGHT;
    float near = 0.5f;
    float far = 100.0f;

    float top = near * tan(fov / 2.0f);
    float right = aspect_ratio * top;
    float left;
    if (isLeftEye) {
        left = -right + CAMERA_OFFSET;
        right = right + CAMERA_OFFSET;
    } else {
        left = -right - CAMERA_OFFSET;
        right = right - CAMERA_OFFSET;
    }

    return glm::frustum(left, right, -top, top, near, far);
}

void setupFramebuffer(unsigned int& fbo, unsigned int& texture, unsigned int& rbo) {
    glGenFramebuffers(1, &fbo);
    glGenTextures(1, &texture);
    glGenRenderbuffers(1, &rbo);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setupQuad(unsigned int& vao, unsigned int& vbo, const float* vertices, size_t size) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void setupSkybox(unsigned int& vao, unsigned int& vbo, const float* vertices, size_t size) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}
