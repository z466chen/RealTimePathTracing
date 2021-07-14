// Termm--Fall 2020

#include "Primitive.hpp"
#include "polyroots.hpp"
#include <utility>
#include <vector>
#include <algorithm>
#include <glm/ext.hpp>
#include "defines.hpp"


const NonhierSphere Sphere::content = NonhierSphere(glm::vec3(0,0,0), 1.0f);
const NonhierBox Cube::content = NonhierBox(glm::vec3(0,0,0), 1.0f);


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



Intersection NonhierSphere::intersect(const Ray &ray) const {
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
    return result;
}

AABB NonhierSphere::getAABB() const {
    AABB result;
    result.lower_bound = m_pos - glm::vec3(m_radius, m_radius, m_radius);
    result.upper_bound = m_pos + glm::vec3(m_radius, m_radius, m_radius);
    return result;
}


Intersection NonhierBox::intersect(const Ray &ray) const {
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

    std::vector<std::pair<float,int>> lowert_arr = {
        {lower_ts.x, 0},
        {lower_ts.y, 1},
        {lower_ts.z, 2},
    };

    std::vector<std::pair<float,int>> uppert_arr = {
        {upper_ts.x, 3},
        {upper_ts.y, 4},
        {upper_ts.z, 5},
    };

    if (ray.direction.x < 0) {
        auto temp = lowert_arr[0];
        lowert_arr[0] = uppert_arr[0];
        uppert_arr[0] = temp;
        // std::swap(lowert_arr[0], uppert_arr[0]);
    }

    if (ray.direction.y < 0) {
        auto temp = lowert_arr[1];
        lowert_arr[1] = uppert_arr[1];
        uppert_arr[1] = temp;
        // std::swap(lowert_arr[1], uppert_arr[1]);
    }

    if (ray.direction.z < 0) {
        auto temp = lowert_arr[2];
        lowert_arr[2] = uppert_arr[2];
        uppert_arr[2] = temp;
        // std::swap(lowert_arr[2], uppert_arr[2]);
    }

    auto lowert = *std::max_element(lowert_arr.begin(), lowert_arr.end(), 
        [](std::pair<float, int> &a, std::pair<float, int> &b){ return a.first < b.first; });
    auto uppert = *std::min_element(uppert_arr.begin(), uppert_arr.end(), 
        [](std::pair<float, int> &a, std::pair<float, int> &b){ return a.first < b.first; });

    Intersection result = Intersection();

    if (uppert.first > 0 && lowert.first <= uppert.first) {
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

AABB NonhierBox::getAABB() const {
    AABB result;
    result.lower_bound = m_pos - glm::vec3(m_size*0.5,m_size*0.5,m_size*0.5);
    result.upper_bound = m_pos + glm::vec3(m_size*0.5,m_size*0.5,m_size*0.5); 
    return result;
}


Intersection Sphere::intersect(const Ray &ray) const {
    Intersection result = content.intersect(ray);
    result.obj = this;
    return result;
}

AABB Sphere::getAABB() const {
    return content.getAABB();
} 

Intersection Cube::intersect(const Ray &ray) const {
    Intersection result = content.intersect(ray);
    result.obj = this;
    return result;
}

AABB Cube::getAABB() const {
    return content.getAABB();
}
