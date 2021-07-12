#pragma once
#include <glm/glm.hpp>

#include "Ray.hpp"

class AABB {
public:
    glm::vec3 lower_bound;
    glm::vec3 upper_bound;

    friend AABB operator+(const AABB &lhs,const AABB &rhs);

    bool isIntersect(const Ray & ray) const;
};
