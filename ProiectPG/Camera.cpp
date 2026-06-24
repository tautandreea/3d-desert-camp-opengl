#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>a.hpp"

namespace gps {

    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraUpDirection = glm::normalize(cameraUp);
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection));
    }


    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition,
            cameraPosition + cameraFrontDirection,
            cameraUpDirection);
    }

    void Camera::move(MOVE_DIRECTION direction, float speed) {
        switch (direction) {
        case MOVE_FORWARD:
            cameraPosition += cameraFrontDirection * speed;
            break;
        case MOVE_BACKWARD:
            cameraPosition -= cameraFrontDirection * speed;
            break;
        case MOVE_RIGHT:
            cameraPosition += cameraRightDirection * speed;
            break;
        case MOVE_LEFT:
            cameraPosition -= cameraRightDirection * speed;
            break;
        }
    }


    void Camera::rotate(float pitch, float yaw) {
        // pitch/yaw sunt deja in RADIANI
        glm::vec3 front;
        front.x = cos(yaw) * cos(pitch);
        front.y = sin(pitch);
        front.z = sin(yaw) * cos(pitch);

        cameraFrontDirection = glm::normalize(front);

        // IMPORTANT: folosim up-ul camerei (normalizat) ca in proiectul mare
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
    }

    glm::vec3 Camera::getPosition() {
        return cameraPosition;
    }

    void Camera::setPosition(glm::vec3 position) {
        cameraPosition = position;
    }


}
