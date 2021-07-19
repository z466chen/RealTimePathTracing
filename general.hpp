#pragma once

#include <glm/glm.hpp>
#include <random>
#include <memory>

inline std::unique_ptr<std::random_device> dev = 
    std::make_unique<std::random_device>();

inline std::mt19937 rng = std::mt19937((*dev)());

inline float get_random_float()
{
    // std::random_device dev;
    // std::mt19937 rng(dev());
    std::uniform_real_distribution<float> dist(0.f, 1.f); // distribution in range [1, 6]

    return dist(rng);
}

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

inline double dot2(glm::vec3 v) {
    return glm::dot(v,v);
}


