#pragma once

#include <glm/glm.hpp>

class Camera {
    glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::vec3 camFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 view;

    float fov = 45.0f;

    float min_fov = 0.0f;
    float max_fov = 0.0f;

    float yaw = 0.0f;
    float pitch = 0.0f;

    void __calcViewMatrix();
    void __calcCamFront();
public:
    enum TRANSLATION {
        FORWARD=0, LEFT, BACKWARD, RIGHT
    };

    enum ROTATION {
        YAW=0, PITCH
    };

    Camera();
    Camera(float fov);
    Camera(float fov, glm::vec3 camPos, glm::vec3 camFront);
    Camera(float fov, glm::vec3 camPos, glm::vec3 camFront, glm::vec3 camUp);
    glm::mat4 getViewMatrix() const; 

    glm::vec3 getCamPos() const;
    glm::vec3 getCamFront() const;
    glm::vec3 getCamUp() const;
    glm::vec3 getCamEye() const;
    glm::vec3 getCamView() const;

    float getFov() const;
    float getYaw() const;
    float getPitch() const;

    void setFovLimits(float min, float max);

    float rotate(ROTATION type,float value);
    float translate(TRANSLATION type, float value);
    float zoom(float value);

    float setCamPos(glm::vec3 cameraPosition);
};