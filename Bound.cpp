#include "Bound.hpp"
#include "general.hpp"
#include <glm/ext.hpp>

AABB operator+(const AABB &lhs,const AABB &rhs) {
    AABB result;
    result.lower_bound = vec_min(lhs.lower_bound, rhs.lower_bound);
    result.upper_bound = vec_max(lhs.upper_bound, rhs.upper_bound);

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


AABB AABB::transform(const glm::mat4 &trans) const {
    AABB result;

    glm::vec3 o = ptrans(trans, lower_bound);
    glm::vec3 x = vtrans(trans, glm::vec3(upper_bound.x - lower_bound.x,0.0f,0.0f));
    glm::vec3 y = vtrans(trans, glm::vec3(0.0f,upper_bound.y - lower_bound.y,0.0f));
    glm::vec3 z = vtrans(trans, glm::vec3(0.0f,0.0f,upper_bound.z - lower_bound.z));
    
    glm::vec3 s = o+x+y+z;
    glm::vec3 zero = glm::vec3(0.0f);
    result.lower_bound = o + vec_min(x,zero) + vec_min(y,zero)+ vec_min(z, zero);
    result.upper_bound = s - result.lower_bound + o;
    return result;
}

double AABB::sdf(const glm::vec3 &t) const {
    glm::vec3 center = (lower_bound + upper_bound)*0.5;
    glm::vec3 size = upper_bound - lower_bound;
    glm::vec3 q = glm::abs(t - center) - size*0.5;
    return glm::l2Norm(vec_max(q,glm::vec3(0.0))) + 
        fmin(fmax(q.x,fmax(q.y,q.z)),0.0);
}