#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

void Camera::__calcViewMatrix() {
    view = glm::lookAt(camPos, camPos + camFront, camUp);
}

void Camera::setFovLimits(float min, float max) {
    min_fov = min;
    max_fov = max;
}

Camera::Camera() {}
Camera::Camera(float fov):fov{fov}{}
Camera::Camera(float fov, glm::vec3 camPos, glm::vec3 camFront): fov{fov},
    camPos{camPos}, camFront{camFront} {
    pitch = (asin(camFront.y)/glm::pi<float>()) * 180.0f;
    yaw = (atan2(camFront.z, camFront.x)/glm::pi<float>()) * 180.0f;

    setFovLimits(fov*0.1, fov*2.0f);
    __calcViewMatrix();
}
Camera::Camera(float fov, glm::vec3 camPos, glm::vec3 camFront, 
    glm::vec3 camUp): camPos{camPos}, camFront{camFront}, fov{fov},
    camUp{camUp} {

    pitch = (asin(camFront.y)/glm::pi<float>()) * 180.0f;
    yaw = (atan2(camFront.z, camFront.x)/glm::pi<float>()) * 180.0f;

    setFovLimits(fov*0.1, fov*2.0f);
    __calcViewMatrix();
}

glm::mat4 Camera::getViewMatrix() const {
    return view;
}

float Camera::getFov() const {
    return fov;
}

float Camera::getYaw() const {
    return yaw;
}

float Camera::getPitch() const {
    return pitch;
}

void Camera::__calcCamFront() {
    camFront.x = cos(glm::radians(pitch))*cos(glm::radians(yaw));
    camFront.y = sin(glm::radians(pitch));
    camFront.z = cos(glm::radians(pitch))*sin(glm::radians(yaw));
}

float Camera::rotate(ROTATION type, float value) {
    switch (type) {
        case YAW:
            yaw = fmod(yaw+value, 360.0f);
            __calcCamFront();
            break;
        case PITCH:
            pitch = glm::clamp(pitch+value, -90.0f, 90.0f);
            __calcCamFront();
            break;
    }

    __calcViewMatrix();
}

float Camera::translate(TRANSLATION type, float value) {
    switch (type) {
        case FORWARD:
            camPos += value * camFront;
            break;    
        case BACKWARD:
            camPos -= value * camFront;
            break;
        case LEFT:
            camPos += value * glm::cross(camUp, camFront);
            break;  
        case RIGHT:
            camPos -= value * glm::cross(camUp, camFront);
            break;  
    }

    __calcViewMatrix();
}

glm::vec3 Camera::getCamPos() const {
    return camPos;
}

glm::vec3 Camera::getCamFront() const {
    return camFront;
}

glm::vec3 Camera::getCamUp() const {
    return camUp;
}

glm::vec3 Camera::getCamEye() const {
    return camPos;
}

glm::vec3 Camera::getCamView() const {
    return camPos + camFront;
}

float Camera::zoom(float value) {
    fov = glm::clamp(fov + value, min_fov, max_fov);
}

float Camera::setCamPos(glm::vec3 cameraPosition) {
    camPos = cameraPosition;
    __calcViewMatrix();
}
