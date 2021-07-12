#pragma once
#include "Ray.hpp"
#include "AABB.hpp"
// This is an interface for all the information required for collision detection

class Object {
public:
    virtual ~Object() {};
    virtual Intersection intersect(const Ray &ray) = 0;
    virtual AABB getAABB() const = 0;
};