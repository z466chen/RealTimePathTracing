#pragma once
#include "Ray.hpp"
#include "Bound.hpp"
#include "defines.hpp"
// This is an interface for all the information required for collision detection

class Object {
public:
    glm::mat4 t_matrix = glm::mat4(1.0f);
    glm::mat4 inv_t_matrix = glm::mat4(1.0f);
    virtual ~Object() {};
    virtual Intersection intersect(const Ray &ray) const = 0;
    virtual AABB getAABB() const = 0;
    virtual int construct() const = 0;
};