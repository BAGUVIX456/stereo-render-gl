#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "stb_image.h"

#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader/shader.hpp"
#include "camera/camera.hpp"
#include "model/model.h"

#include <iostream>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const unsigned int FISH_COUNT = 1;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastFrame_fps = 0.0f;
int frameCount = 0;

void measure_frame_time(float currentFrame) {
    frameCount++;

    if (currentFrame - lastFrame_fps >= 1.0f) {
        cout << "FrameTime: " << ((currentFrame - lastFrame_fps)/double(frameCount))*1000.0f << endl;
        frameCount = 0;
        lastFrame_fps = currentFrame;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void processInput(GLFWwindow *window)
{
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

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
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

int main()
{
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

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_MULTISAMPLE);
    glfwSwapInterval(0);

    Shader shaderProgram("shaders/shader.vert.glsl", "shaders/shader.frag.glsl");
    Shader skyboxShader("shaders/skybox.vert.glsl", "shaders/skybox.frag.glsl");
    Shader quadShader("shaders/quad.vert.glsl", "shaders/quad.frag.glsl");

    stbi_set_flip_vertically_on_load(false);
    Model fishy("/home/dhanvith/OpenGL/boids/resources/fishy/fish.obj");

    float skyboxVertices[] = 
    {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

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

    unsigned int frame_left, frame_right;
    glGenFramebuffers(1, &frame_left);
    glGenFramebuffers(1, &frame_right);

    unsigned int texture_left, texture_right;
    glGenTextures(1, &texture_left);
    glGenTextures(1, &texture_right);

    unsigned int rbo_left, rbo_right;
    glGenRenderbuffers(1, &rbo_left);
    glGenRenderbuffers(1, &rbo_right);

    glBindFramebuffer(GL_FRAMEBUFFER, frame_left);
    glBindTexture(GL_TEXTURE_2D, texture_left);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_left);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_left, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_left);

    glBindFramebuffer(GL_FRAMEBUFFER, frame_right);
    glBindTexture(GL_TEXTURE_2D, texture_right);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_right);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_right, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_right);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    unsigned int quadVAO_left, quadVAO_right, quadVBO_left, quadVBO_right;
    glGenVertexArrays(1, &quadVAO_left);
    glGenVertexArrays(1, &quadVAO_right);
    glGenBuffers(1, &quadVBO_left);
    glGenBuffers(1, &quadVBO_right);

    glBindVertexArray(quadVAO_left);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO_left);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices_left), quadVertices_left, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

    glBindVertexArray(quadVAO_right);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO_right);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices_right), quadVertices_right, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

    vector<std::string> faces
    {
        "/home/dhanvith/OpenGL/boids/resources/blue/right.png",
        "/home/dhanvith/OpenGL/boids/resources/blue/left.png",
        "/home/dhanvith/OpenGL/boids/resources/blue/top.png",
        "/home/dhanvith/OpenGL/boids/resources/blue/bottom.png",
        "/home/dhanvith/OpenGL/boids/resources/blue/front.png",
        "/home/dhanvith/OpenGL/boids/resources/blue/back.png"
    };

    unsigned int skyboxTexture = loadCubemap(faces);

    glm::vec3 offsets[FISH_COUNT];

    offsets[0] = glm::vec3(1.0f, 1.0f, -7.0f);
    // offsets[1] = glm::vec3(3.0f, 0.0f, -5.0f);

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

        glBindFramebuffer(GL_FRAMEBUFFER, frame_left);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.01f, 100.0f);
        glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));

        glDepthMask(GL_FALSE);
        skyboxShader.use();
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);

        glBindVertexArray(skyboxVAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthMask(GL_TRUE);

        shaderProgram.use();

        view = camera.GetViewMatrix();
        shaderProgram.setMat4("view", view);
        shaderProgram.setMat4("projection", projection);
        shaderProgram.setFloat("_Time", glfwGetTime());
        
        for(int i=0; i<FISH_COUNT; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, offsets[i]);
            shaderProgram.setMat4("model", model);

            fishy.Draw(shaderProgram);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, frame_right);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.01f, 100.0f);
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));

        glDepthMask(GL_FALSE);
        skyboxShader.use();
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);

        glBindVertexArray(skyboxVAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthMask(GL_TRUE);

        shaderProgram.use();

        view = camera.GetViewMatrix();
        shaderProgram.setMat4("view", view);
        shaderProgram.setMat4("projection", projection);
        shaderProgram.setFloat("_Time", glfwGetTime());
        
        for(int i=0; i<FISH_COUNT; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, offsets[i]);
            shaderProgram.setMat4("model", model);

            fishy.Draw(shaderProgram);
        }

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
