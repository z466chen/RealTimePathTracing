// Termm--Fall 2020

#include "Primitive.hpp"
#include "polyroots.hpp"
#include <utility>
#include <map>
#include <algorithm>
#include "defines.hpp"

Primitive::~Primitive()
{
}

Sphere::~Sphere()
{
}

Cube::~Cube()
{
}

NonhierSphere::~NonhierSphere()
{
}

NonhierBox::~NonhierBox()
{
}


Intersection NonhierSphere::intersect(Ray ray) {
    Intersection result = Intersection();

    glm::vec3 temp = ray.origin - m_pos;
    double A = glm::dot(ray.direction, ray.direction);
    double B = 2.0f*glm::dot(ray.direction, temp);
    double C = glm::dot(temp, temp) - m_radius*m_radius;

    double roots[2];
    int code = quadraticRoots(A, B, C, roots);

    if (code == 0) return result;
    double t = roots[0];
    if (code == 2) {
        if (roots[0] > roots[1]) std::swap(roots[0], roots[1]);
        t = roots[0];
        if (t < 0) t = roots[1];
    }

    if (t < 0) return result;

    result.intersects = true;
    result.t = t;
    result.position = ray.origin + (float)t*ray.direction;
    result.normal = glm::normalize(result.position - m_pos);
    result.obj = this;
}

Intersection NonhierBox::intersect(Ray ray) {
    glm::vec3 lower_bound = m_pos - glm::vec3(m_size*0.5,m_size*0.5,m_size*0.5);
    glm::vec3 upper_bound = m_pos + glm::vec3(m_size*0.5,m_size*0.5,m_size*0.5);

    glm::vec3 inv_direction = glm::vec3(1/ray.direction.x, 1/ray.direction.y, 1/ray.direction.z);

    glm::vec3 lower_ts = (lower_bound - ray.origin) * inv_direction; 
    glm::vec3 upper_ts = (upper_bound - ray.origin) * inv_direction;

    glm::vec3 normals[6] = {
        glm::vec3(-1, 0, 0),
        glm::vec3(0, -1, 0),
        glm::vec3(0, 0, -1),
        glm::vec3(1, 0, 0),
        glm::vec3(0, 1, 0),
        glm::vec3(0, 0, 1)
    };

    std::map<float,int> lowert_map = {
        {lower_ts.x, 0},
        {lower_ts.y, 1},
        {lower_ts.z, 2},
    };

    std::map<float,int> uppert_map = {
        {upper_ts.x, 3},
        {upper_ts.y, 4},
        {upper_ts.z, 5},
    };

    if (ray.direction.x < 0) {
        std::swap(lowert_map[0], uppert_map[0]);
    }

    if (ray.direction.y < 0) {
        std::swap(lowert_map[1], uppert_map[1]);
    }

    if (ray.direction.z < 0) {
        std::swap(lowert_map[2], uppert_map[2]);
    }

    auto lowert = *std::max_element(lowert_map.begin(), lowert_map.end());
    auto uppert = *std::min_element(uppert_map.begin(), uppert_map.end());

    Intersection result = Intersection();

    if (uppert.first > 0 && lowert.first < uppert.first) {
        if (lowert.first > 0) {
            result.t = lowert.first;
            result.normal = normals[lowert.second];
        } else {
            result.t = uppert.first;
            result.normal = normals[uppert.second];
        }

        result.intersects = true;
        result.position = ray.origin + (float)result.t*ray.direction;
        result.obj = this;
    }

    return result;
}

