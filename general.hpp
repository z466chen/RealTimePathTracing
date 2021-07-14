#pragma once

#include <glm/glm.hpp>

inline glm::vec3 vec_min(glm::vec3 lhs, glm::vec3 rhs) {
    return glm::vec3(fmin(lhs.x, rhs.x), fmin(lhs.y, rhs.y), fmin(lhs.z, rhs.z));
}

inline glm::vec3 vec_max(glm::vec3 lhs, glm::vec3 rhs) {
    return glm::vec3(fmax(lhs.x, rhs.x), fmax(lhs.y, rhs.y), fmax(lhs.z, rhs.z));
}

inline glm::vec3 to_vec3(glm::vec4 &&v) {
    return glm::vec3(v.x, v.y,v.z)/v.w;
}

inline glm::vec3 ptrans(const glm::mat4 &m, glm::vec3 p) {
    return to_vec3(m * glm::vec4(p, 1.0f));
}

inline glm::vec3 ptrans(glm::mat4 &&m, glm::vec3 p) {
    return to_vec3(m * glm::vec4(p, 1.0f));
}

inline glm::vec3 vtrans(const glm::mat4 &m, glm::vec3 v) {
    return glm::vec3(m * glm::vec4(v, 0.0f));
}

inline glm::vec3 vtrans(glm::mat4 &&m, glm::vec3 v) {
    return glm::vec3(m * glm::vec4(v, 0.0f));
}



