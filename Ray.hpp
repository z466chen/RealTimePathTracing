#pragma once

#include <glm/glm.hpp>

#include <iostream>
class Material;
class Primitive;

class Ray {
public:
    glm::vec3 origin;
    glm::vec3 direction;

    Ray(){};
    Ray(glm::vec3 origin, glm::vec3 direction):
        origin{origin},direction{direction} {}
};

class Intersection {
public:
    const Primitive *obj = nullptr;
    const Material * material = nullptr;
    glm::vec3 normal;
    glm::vec3 position;
    double t = 0.0;
    bool intersects = false;

    Intersection() {
        // std::cout << intersects << std::endl;
    };
};