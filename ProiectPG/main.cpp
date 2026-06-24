#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <iostream>

gps::Window myWindow;

glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

glm::vec3 lightDir;
glm::vec3 lightColor;

GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;

bool isNight = false;
int activateFog = 0;
bool presentationMode = false;
float animAngle = 0.0f;


glm::vec3 dirColorDay = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 dirColorNight = glm::vec3(0.01f, 0.01f, 0.05); 
glm::vec3 fireWorldPos = glm::vec3(3.830240f,0.514880f,1.842519f); //coordonatele focului

const int NR_POINT_LIGHTS = 1;

// atenuare + spec
float pointKc = 1.0f;
float pointKl = 0.7f;
float pointKq = 1.8f;
float pointShininess = 8.0f;
float pointSpecStrength = 0.18f;

// uniform locations
GLint pointPosLoc = -1;
GLint pointColorLoc = -1;
GLint pointKcLoc = -1, pointKlLoc = -1, pointKqLoc = -1;
GLint pointShininessLoc = -1;
GLint pointSpecStrengthLoc = -1;

std::vector<const GLchar*> skyboxFacesDay = {
    "skybox_day/px.png",
    "skybox_day/nx.png",
    "skybox_day/py.png",
    "skybox_day/ny.png",
    "skybox_day/pz.png",
    "skybox_day/nz.png"
};

std::vector<const GLchar*> skyboxFacesNight = {
    "skybox_night/px.png",
    "skybox_night/nx.png",
    "skybox_night/py.png",
    "skybox_night/ny.png",
    "skybox_night/pz.png",
    "skybox_night/nz.png"
};
glm::vec3 presentationTarget = glm::vec3(0.220238f, 0.514880f, -0.131434f);
const glm::vec3 startPos = glm::vec3(5.733179f, 0.514880f, -2.064091f);
const float startYaw = -glm::half_pi<float>();
const float startPitch = 0.0f;

gps::Camera myCamera(
    startPos,
    glm::vec3(0.0f, 0.04f, -20.0f),   
    glm::vec3(0.0f, 1.0f, 0.0f));
GLfloat cameraSpeed = 0.03f;

GLboolean pressedKeys[1024];

bool firstMouse = true;
double lastX = 512.0, lastY = 384.0;
float yaw = -glm::half_pi<float>();
float pitch = 0.0f;
float mouseSensitivity = 0.0025f;   
bool useFlatShading = false;
GLint useFlatLoc = -1;


gps::Model3D teapot;
gps::Model3D portile_cortului; 
gps::Model3D cow;
bool openGates = false;        
float gateOffset = 0.0f;

gps::Shader myBasicShader;
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

//pentru pozitia camerei 
const glm::vec3 groundCenter = glm::vec3(-0.14253f, -0.28512f, 0.32104f);
const float groundSizeX = 12.5f;
const float groundSizeZ = 12.5f;
const float groundHalfX = groundSizeX * 0.5f; 
const float groundHalfZ = groundSizeZ * 0.5f; 
const float margin = 0.20f;   
const float eyeHeight = 0.8f;   

struct BoundingBox {
    glm::vec3 min;
    glm::vec3 max;
};

std::vector<BoundingBox> solidObjects = {
  {
        glm::vec3(3.33024f, -5.0f, 1.34251f), 
        glm::vec3(4.33024f,  5.0f, 2.34251f)  
    }
};

GLenum glCheckError_(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)


void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);

    if (height == 0) height = 1;

    glViewport(0, 0, width, height);
    myWindow.setWindowDimensions({ width, height });

    projection = glm::perspective(glm::radians(45.0f),
        (float)width / (float)height,
        0.1f, 20.0f);

    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }

    if (key == GLFW_KEY_N && action == GLFW_PRESS) {
        isNight = !isNight;

        myBasicShader.useShaderProgram();

        glm::vec3 newColor = isNight ? dirColorNight : dirColorDay;
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(newColor));
      
        // schimba skybox
        if (isNight)
            mySkyBox.Load(skyboxFacesNight);
        else
            mySkyBox.Load(skyboxFacesDay);

        if (isNight)
            glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        else
            glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    }
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_K: // Modul SOLID
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;

        case GLFW_KEY_L: // Modul WIREFRAME
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;

        case GLFW_KEY_J: // Modul POLIGONAL
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            glPointSize(3.0f); 
            break;
        case GLFW_KEY_H: // Modul SMOOTH/Flat
            useFlatShading = !useFlatShading;
            myBasicShader.useShaderProgram();
            if (useFlatLoc != -1) glUniform1i(useFlatLoc, useFlatShading ? 1 : 0);
            std::cout << "Shading: " << (useFlatShading ? "FLAT" : "SMOOTH") << "\n";
            break;

        }

    }

    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        activateFog = 1;
    }
    if (key == GLFW_KEY_G && action == GLFW_PRESS) {
        activateFog = 0;
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        presentationMode = false; 
        myCamera.setPosition(startPos);
        yaw = startYaw;
        pitch = startPitch;
        myCamera.rotate(pitch, yaw);
        std::cout << "Camera resetata la punctul de plecare.\n";
    }

    if (key == GLFW_KEY_V && action == GLFW_PRESS) {
        presentationMode = !presentationMode;
        if (presentationMode) animAngle = 0.0f;
        std::cout << "Mod prezentare: " << (presentationMode ? "PORNIT" : "OPRIT") << "\n";
    }

    if (key == GLFW_KEY_O && action == GLFW_PRESS) {
        openGates = !openGates;
        std::cout << "Portile sunt: " << (openGates ? "DESCHISE" : "INCHISE") << std::endl;
    }

    if (key == GLFW_KEY_T && action == GLFW_PRESS) {
        static bool cursorEnabled = false;
        if (cursorEnabled) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        cursorEnabled = !cursorEnabled;
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
        return;
    }

    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    yaw += (float)xoffset * mouseSensitivity;
    pitch += (float)yoffset * mouseSensitivity;

    float limit = glm::radians(89.0f);
    pitch = glm::clamp(pitch, -limit, limit);

    if (yaw > glm::pi<float>())  yaw -= glm::two_pi<float>();
    if (yaw < -glm::pi<float>()) yaw += glm::two_pi<float>();

    myCamera.rotate(pitch, yaw);
}

bool checkCollision(glm::vec3 newPos) {
    float playerRadius = 0.05f; 

    for (const auto& box : solidObjects) {
        if (newPos.x + playerRadius > box.min.x && newPos.x - playerRadius < box.max.x &&
            newPos.z + playerRadius > box.min.z && newPos.z - playerRadius < box.max.z) {
            return true; 
        }
    }
    return false;
}

void processMovement()
{
    // 1) Move (WASD)
    float speed = cameraSpeed;
    if (pressedKeys[GLFW_KEY_LEFT_SHIFT]) speed *= 3.0f;
    glm::vec3 oldPos = myCamera.getPosition();
    glm::vec3 nextPos = oldPos;

    if (pressedKeys[GLFW_KEY_W]) myCamera.move(gps::MOVE_FORWARD, speed);
    if (pressedKeys[GLFW_KEY_S]) myCamera.move(gps::MOVE_BACKWARD, speed);
    if (pressedKeys[GLFW_KEY_A]) myCamera.move(gps::MOVE_LEFT, speed);
    if (pressedKeys[GLFW_KEY_D]) myCamera.move(gps::MOVE_RIGHT, speed);

    nextPos = myCamera.getPosition();
    if (checkCollision(nextPos)) {
        myCamera.setPosition(oldPos); 
    }
    float rotSpeed = 0.02f; 
    model = glm::mat4(1.0f);

    if (pressedKeys[GLFW_KEY_LEFT])  yaw -= rotSpeed;
    if (pressedKeys[GLFW_KEY_RIGHT]) yaw += rotSpeed;
    if (pressedKeys[GLFW_KEY_UP])    pitch += rotSpeed;
    if (pressedKeys[GLFW_KEY_DOWN])  pitch -= rotSpeed;

    float pitchLimit = glm::radians(89.0f);
    pitch = glm::clamp(pitch, -pitchLimit, pitchLimit);

    if (yaw > glm::pi<float>())   yaw -= glm::two_pi<float>();
    if (yaw < -glm::pi<float>())  yaw += glm::two_pi<float>();

    myCamera.rotate(pitch, yaw);
 
    glm::vec3 camPos = myCamera.getPosition();

    
    camPos.y = groundCenter.y + eyeHeight;


    camPos.x = glm::clamp(camPos.x,
        groundCenter.x - groundHalfX + margin,
        groundCenter.x + groundHalfX - margin);

    camPos.z = glm::clamp(camPos.z,
        groundCenter.z - groundHalfZ + margin,
        groundCenter.z + groundHalfZ - margin);

    myCamera.setPosition(camPos);
    glm::vec3 currentPos = myCamera.getPosition();
   
}

void initOpenGLWindow() {
    myWindow.Create(1024, 768, "OpenGL Project Core");
    for (int i = 0; i < 1024; i++) pressedKeys[i] = false;
}

void setWindowCallbacks() {
    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void initOpenGLState() {
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}

void initModels() {
    teapot.LoadModel("models/teapot/proiect.obj");
    portile_cortului.LoadModel("models/teapot/portile_cortului.obj"); 
    cow.LoadModel("models/teapot/vaca.obj");
}

void initShaders() {
    myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
    skyboxShader.loadShader("shaders/skybox.vert", "shaders/skybox.frag"); 
}

void initSkybox() {
    if (isNight)
        mySkyBox.Load(skyboxFacesNight);
    else
        mySkyBox.Load(skyboxFacesDay);

}

void initUniforms() {
    myBasicShader.useShaderProgram();

    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");
    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    useFlatLoc = glGetUniformLocation(myBasicShader.shaderProgram, "useFlat");
    if (useFlatLoc != -1) glUniform1i(useFlatLoc, useFlatShading ? 1 : 0);

   
    model = glm::mat4(1.0f);
    view = myCamera.getViewMatrix();
    projection = glm::perspective(glm::radians(45.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 200.0f);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    
    lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    lightColor = isNight ? dirColorNight : dirColorDay;
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    pointPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightPos");
    pointColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightColor");
    pointKcLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightKc");
    pointKlLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightKl");
    pointKqLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightKq");
    pointShininessLoc = glGetUniformLocation(myBasicShader.shaderProgram, "blinnShininess");
    pointSpecStrengthLoc = glGetUniformLocation(myBasicShader.shaderProgram, "blinnSpecStrength");

    std::cout << "[Fire PointLight uniforms] pos=" << pointPosLoc
        << " color=" << pointColorLoc
        << " kc=" << pointKcLoc << " kl=" << pointKlLoc << " kq=" << pointKqLoc
        << " shin=" << pointShininessLoc
        << " spec=" << pointSpecStrengthLoc << "\n";

    // parametri constanti
    if (pointKcLoc != -1) glUniform1f(pointKcLoc, pointKc);
    if (pointKlLoc != -1) glUniform1f(pointKlLoc, pointKl);
    if (pointKqLoc != -1) glUniform1f(pointKqLoc, pointKq);
    if (pointShininessLoc != -1) glUniform1f(pointShininessLoc, pointShininess);
    if (pointSpecStrengthLoc != -1) glUniform1f(pointSpecStrengthLoc, pointSpecStrength);
    glm::vec3 fireLightColor = glm::vec3(2.5f, 1.0f, 0.2f);
    glUniform3fv(pointColorLoc, 1, glm::value_ptr(fireLightColor));
        glUniform1f(pointKcLoc, pointKc);
        glUniform1f(pointKlLoc, pointKl);
        glUniform1f(pointKqLoc, pointKq);
   
}


void renderTeapot(gps::Shader shader) {
    shader.useShaderProgram();
    teapot.Draw(shader);
}

void renderScene() {
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    myBasicShader.useShaderProgram();

    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "activateFog"), activateFog);

    // animatie foc
    float t = (float)glfwGetTime(); 
    float flicker = 1.0f + sin(t * 15.0f) * 0.1f + cos(t * 25.0f) * 0.05f;
    glm::vec3 dynamicColor = glm::vec3(2.5f, 1.0f, 0.2f) * flicker;
    glUniform3fv(pointColorLoc, 1, glm::value_ptr(dynamicColor));

    float offsetX = sin(t * 20.0f) * 0.02f;
    float offsetZ = cos(t * 18.0f) * 0.02f;
    glm::vec3 dynamicFirePos = fireWorldPos + glm::vec3(offsetX, 0.0f, offsetZ);

    // prezentare animata
    if (presentationMode) {
        animAngle += 0.005f;
        float radius = 6.0f;
        float camX = sin(animAngle) * radius;
        float camZ = cos(animAngle) * radius;
        view = glm::lookAt(glm::vec3(camX, 2.0f, camZ), presentationTarget, glm::vec3(0.0f, 1.0f, 0.0f));
        myCamera.setPosition(glm::vec3(camX, 2.0f, camZ));
    }
    else {
        view = myCamera.getViewMatrix();
    }
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // lumina punctiforma
    if (pointPosLoc != -1) {
        glm::vec3 firePosEye = glm::vec3(view * glm::vec4(dynamicFirePos, 1.0f));
        glUniform3fv(pointPosLoc, 1, glm::value_ptr(firePosEye));
    }

    model = glm::mat4(1.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    renderTeapot(myBasicShader);

    // animatie porti
    if (openGates) {
        if (gateOffset < 0.15f) gateOffset += 0.002f;
    }
    else {
        if (gateOffset > 0.0f) gateOffset -= 0.002f;
    }
    glm::mat4 gateModel = glm::mat4(1.0f);
    gateModel = glm::translate(gateModel, glm::vec3(0.0f, gateOffset, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(gateModel));
    glm::mat3 gateNormal = glm::mat3(glm::inverseTranspose(view * gateModel));
    glUniformMatrix3fv(glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix"),
        1, GL_FALSE, glm::value_ptr(gateNormal));

    portile_cortului.Draw(myBasicShader);

    // animatie capra
    glm::mat4 cowModel = glm::mat4(1.0f);

    float moveZ = sin(t * 0.3f) * 0.1f;
    cowModel = glm::translate(cowModel, glm::vec3(0.0f, 0.0f, moveZ));
    float rotVaca = sin(t * 0.4f) * glm::radians(1.5f);
    cowModel = glm::rotate(cowModel, rotVaca, glm::vec3(0.0f, 1.0f, 0.0f));
    float scaleVaca = 1.0f + sin(t * 0.8f) * 0.01f;
    cowModel = glm::scale(cowModel, glm::vec3(scaleVaca));

    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(cowModel));
    glm::mat3 cowNormal = glm::mat3(glm::inverseTranspose(view * cowModel));
    glUniformMatrix3fv(glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(cowNormal));

    cow.Draw(myBasicShader);

    skyboxShader.useShaderProgram();
    mySkyBox.Draw(skyboxShader, view, projection);
}


void cleanup() {
    myWindow.Delete();
}

int main(int argc, const char* argv[]) {

    try {
        initOpenGLWindow();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
;
    initOpenGLState();
    initModels();
    initShaders();
    initUniforms();
    initSkybox();

    setWindowCallbacks();

    glCheckError();

    while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
        renderScene();

        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());

        glCheckError();
    }

    cleanup();

    return EXIT_SUCCESS;
}
