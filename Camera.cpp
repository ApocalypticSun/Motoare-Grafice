#include "Camera.h"
#include <cmath>

static constexpr float kBaseMoveDivisor = 5.0f;     
static constexpr float kSpeedNormal = 0.1f;         
static constexpr float kSpeedBoost = 0.2f;         
static constexpr float kMaxPitchDeg = 85.0f;        

Camera::Camera(int width, int height, glm::vec3 position)
    : width(width), height(height), Position(position)
{
}

void Camera::Matrix(float FOVdeg, float nearPlane, float farPlane, Shader& shader, const char* uniform)
{
    const glm::mat4 view = glm::lookAt(Position, Position + Orientation, Up);
    const glm::mat4 proj = glm::perspective(glm::radians(FOVdeg), float(width) / float(height), nearPlane, farPlane);

    glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(proj * view));
}

void Camera::Inputs(GLFWwindow* window)
{
    const glm::vec3 forward = glm::normalize(Orientation);
    const glm::vec3 right = glm::normalize(glm::cross(forward, Up));
    const glm::vec3 up = glm::normalize(Up);

    const float move = speed / kBaseMoveDivisor;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)            Position += move * forward;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)            Position -= move * forward;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)            Position -= move * right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)            Position += move * right;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)        Position += move * up;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) Position -= move * up;

    speed = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ? kSpeedBoost : kSpeedNormal;

    const bool isLmbDown = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    if (!isLmbDown)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstClick = true;
        return;
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    const float centerX = float(width) * 0.5f;
    const float centerY = float(height) * 0.5f;

    if (firstClick)
    {
        glfwSetCursorPos(window, centerX, centerY);
        firstClick = false;
    }

    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    const float deltaY = float(mouseY) - centerY;
    const float deltaX = float(mouseX) - centerX;

    const float rotX = sensitivity * (deltaY / float(height));
    const float rotY = sensitivity * (deltaX / float(width));


    const glm::vec3 pitchAxis = glm::normalize(glm::cross(Orientation, Up));
    const glm::vec3 pitched = glm::rotate(Orientation, glm::radians(-rotX), pitchAxis);

    if (std::abs(glm::angle(pitched, Up) - glm::radians(90.0f)) <= glm::radians(kMaxPitchDeg))
        Orientation = pitched;
    Orientation = glm::normalize(glm::rotate(Orientation, glm::radians(-rotY), Up));

    glfwSetCursorPos(window, centerX, centerY);
}
