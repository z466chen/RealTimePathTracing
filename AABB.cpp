#include "AABB.hpp"

AABB operator+(const AABB &lhs,const AABB &rhs) {
    AABB result;
    result.lower_bound = glm::min(lhs.lower_bound, rhs.lower_bound);
    result.upper_bound = glm::max(lhs.upper_bound, rhs.upper_bound);

    return result;
}

bool AABB::isIntersect(const Ray &ray) const {
    glm::vec3 inv_direction = glm::vec3(1/ray.direction.x, 1/ray.direction.y, 1/ray.direction.z);

    glm::vec3 lower_ts = (lower_bound - ray.origin) * inv_direction; 
    glm::vec3 upper_ts = (upper_bound - ray.origin) * inv_direction;

    if (ray.direction.x < 0) {
        std::swap(lower_ts.x, upper_ts.x);
    }

    if (ray.direction.y < 0) {
        std::swap(lower_ts.y, upper_ts.y);
    }

    if (ray.direction.z < 0) {
        std::swap(lower_ts.z, upper_ts.z);
    }

    auto lowert = fmax(lower_ts.x, fmax(lower_ts.y, lower_ts.z));
    auto uppert = fmin(upper_ts.x, fmin(upper_ts.y, upper_ts.z));;

    return uppert > 0 && uppert >= lowert;
}
