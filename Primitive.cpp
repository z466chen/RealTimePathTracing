// Termm--Fall 2020

#include "Primitive.hpp"
#include "polyroots.hpp"
#include <utility>
#include <vector>
#include <algorithm>
#include <glm/ext.hpp>
#include "general.hpp"


const NonhierSphere Sphere::content = NonhierSphere(glm::vec3(0,0,0), 1.0f);
const NonhierBox Cube::content = NonhierBox(glm::vec3(0,0,0), 1.0f);


Primitive::~Primitive()
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
    // result.obj = this;
    return result;
}

double NonhierSphere::sdf(const glm::vec3 &t) const {
    double result = glm::l2Norm(t - m_pos) - m_radius;
    // std::cout << "Sphere: "<< result << std::endl;
    return result;
}

AABB NonhierSphere::getAABB() const {
    AABB result;
    result.lower_bound = m_pos - glm::vec3(m_radius, m_radius, m_radius);
    result.upper_bound = m_pos + glm::vec3(m_radius, m_radius, m_radius);
    return result;
}

NonhierSphere::~NonhierSphere()
{
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
        // result.obj = this;

    }
    
    return result;
}

double NonhierBox::sdf(const glm::vec3 &t) const {
    glm::vec3 q = abs(t - m_pos) - glm::vec3(m_size)*0.5;
    double result = glm::l2Norm(vec_max(q,glm::vec3(0.0))) + 
        fmin(fmax(q.x,fmax(q.y,q.z)),0.0);
    // std::cout << "box:" << result << " " << 
    //     glm::to_string(t - m_pos) << " "  << glm::l2Norm(vec_max(q,glm::vec3(0.0))) << " "<< fmin(fmax(q.x,fmax(q.y,q.z)),0.0) << std::endl;
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
    // result.obj = this;
    return result;
}

NonhierBox::~NonhierBox()
{
}


double Sphere::sdf(const glm::vec3 &t) const {
    return content.sdf(t);
}

AABB Sphere::getAABB() const {
    return content.getAABB();
}

Intersection Cube::intersect(const Ray &ray) const {
    Intersection result = content.intersect(ray);
    // result.obj = this;
    return result;
}

Sphere::~Sphere()
{
}

double Cube::sdf(const glm::vec3 &t) const {
    return content.sdf(t);
}

AABB Cube::getAABB() const {
    return content.getAABB();
} 

Cube::~Cube()
{
}

RoundBox::RoundBox(const glm::vec3 &size, float radius):size{size},radius{radius} {}


Intersection RoundBox::intersect(const Ray &ray) const {
    Intersection result;

    // std::cout << "hello" << std::endl;
    glm::vec3 ro = ray.origin;
    glm::vec3 rd = glm::normalize(ray.direction);
    float dnorm = glm::l2Norm(ray.direction);
    // bounding box
    glm::vec3 m = glm::vec3(1.0/ray.direction.x, 1.0/ray.direction.y, 1.0/ray.direction.z);
    glm::vec3 n = m*ro;
    glm::vec3 k = abs(m)*(size+radius);
    glm::vec3 t1 = -n - k;
    glm::vec3 t2 = -n + k;
    float tN = fmax( fmax( t1.x, t1.y ), t1.z );
    float tF = fmin( fmin( t2.x, t2.y ), t2.z );
    if( tN>tF || tF<0.0) return result;
    float t = tN;

    // convert to first octant
    glm::vec3 pos = ro+t*rd;
    
    glm::vec3 s = sign(pos);
    ro  = ro*s;
    rd  = rd*s;
    pos = pos*s;
        
    // faces
    pos -= size;
    // std::cout << glm::to_string(pos) << std::endl;
    pos = vec_max( pos, glm::vec3(pos.y, pos.z, pos.x) );
    if( fmin(fmin(pos.x,pos.y),pos.z) < 0.0 ) {
        if (t < 0) return result;
        result.intersects = true;
        result.t = t/dnorm;
        result.position = ray.direction * t + ray.origin;
        result.normal = sign(result.position)*glm::normalize(
            vec_max(glm::abs(result.position)- size,glm::vec3(0,0,0)));
    
        
        return result;
    };

    // std::cout << "hello1" << std::endl;

    //some precomputation
    glm::vec3 oc = ro - size;
    glm::vec3 dd = rd*rd;
    glm::vec3 oo = oc*oc;
    glm::vec3 od = oc*rd;
    float ra2 = radius*radius;

    t = 1e20;        

    // corner
    {
    float b = od.x + od.y + od.z;
	float c = oo.x + oo.y + oo.z - ra2;
	float h = b*b - c;
	if( h>0.0 ) t = -b-sqrt(h);
    }

    // edge X
    {
    float a = dd.y + dd.z;
    float b = od.y + od.z;
    float c = oo.y + oo.z - ra2;
    float h = b*b - a*c;
    if( h>0.0 )
    {
        h = (-b-sqrt(h))/a;
        if( h>0.0 && h<t && abs(ro.x+rd.x*h)<size.x ) t = h;
    }
    }
    // edge Y
    {
    float a = dd.z + dd.x;
    float b = od.z + od.x;
    float c = oo.z + oo.x - ra2;
    float h = b*b - a*c;
    if( h>0.0 )
    {
        h = (-b-sqrt(h))/a;
        if( h>0.0 && h<t && abs(ro.y+rd.y*h)<size.y ) t = h;
    }
    }
    // edge Z
    {
    float a = dd.x + dd.y;
    float b = od.x + od.y;
    float c = oo.x + oo.y - ra2;
    float h = b*b - a*c;
    if( h>0.0 )
    {
        h = (-b-sqrt(h))/a;
        if( h>0.0 && h<t && abs(ro.z+rd.z*h)<size.z ) t = h;
    }
    }

    if( t> 1e19  || t < EPSILON) return result;
    
    result.intersects = true;
    result.t = t/dnorm;
    result.position = ray.direction * t + ray.origin;
    result.normal = sign(result.position)*glm::normalize(
         vec_max(glm::abs(result.position)- size,glm::vec3(0,0,0)));
	return result;
}

double RoundBox::sdf(const glm::vec3 &t) const {
    glm::vec3 q = abs(t) - size;
    return glm::l2Norm(vec_max(q,glm::vec3(0.0f))) + fmin(fmax(q.x,fmax(q.y,q.z)),0.0) - radius;
}

AABB RoundBox::getAABB() const {
    AABB result;
    result.lower_bound = -1*(size + radius);
    result.upper_bound = size + radius;
    return result;
}

RoundBox::~RoundBox() {

}


Cylinder::Cylinder(float height, float radius):height{height},radius{radius} {}


Intersection Cylinder::intersect(const Ray &ray) const {
    Intersection result;

    std::vector<std::pair<float, int>> t_min;
    std::vector<std::pair<float, int>> t_max;

    // side surface
    {
        float A = (ray.direction.x * ray.direction.x) + (ray.direction.z * ray.direction.z);
        float B = 2*(ray.direction.x*(ray.origin.x) + ray.direction.z*(ray.origin.z));
        float C = (ray.origin.x) * (ray.origin.x) + (ray.origin.z) * (ray.origin.z) - (radius*radius);
        
        double roots[2];
        int code = quadraticRoots(A, B, C, roots);

        if (code == 1) {
            glm::vec3 pos = ray.origin + roots[0] * ray.direction;
            if (pos.y >= -height && pos.y <= height)
                t_min.emplace_back(roots[0], 0);
        } else if (code == 2) {
            if (roots[0] > roots[1]) {
                std::swap(roots[0], roots[1]);
            } 
            glm::vec3 pos = ray.origin + roots[0] * ray.direction;
            if (pos.y >= -height && pos.y <= height)
                t_min.emplace_back(roots[0], 0);
            
            pos = ray.origin + roots[1] * ray.direction;
            if (pos.y >= -height && pos.y <= height)
                t_max.emplace_back(roots[1], 0);
        }
    }

    // upper and lower surface
    {
        if (ray.direction.y != 0) {
            float t1 = (-height - ray.origin.y) / ray.direction.y;
            float t2 = (height - ray.origin.y) / ray.direction.y;

            int i1 = 1;
            int i2 = 2;

            if (t1 > t2) {
                std::swap(t1, t2);
                std::swap(i1, i2);
            }

            glm::vec3 p = t1 * ray.direction + ray.origin;
            if (p.x*p.x + p.z*p.z <= radius*radius) {
                t_min.emplace_back(t1, i1);
            }
            p = t2 * ray.direction + ray.origin;

            if (p.x*p.x + p.z*p.z <= radius*radius) {
                t_max.emplace_back(t2, i2);
            }
        }
    }


    std::pair<float, int> t_lower = {-1, 0};
    if (t_min.size() > 0) {
        t_lower = *std::max_element(t_min.begin(), t_min.end(),
            [](std::pair<float, int> &a, std::pair<float, int> &b){  return a.first < b.first; });
    }

    std::pair<float, int> t_upper = {-1, 0};
    if (t_max.size() > 0) {
        t_upper = *std::min_element(t_max.begin(), t_max.end(),
            [](std::pair<float, int> &a, std::pair<float, int> &b){ return a.first < b.first; });
    }

    if (t_upper.first < 0 || t_lower.first > t_upper.first) return result;

    result.intersects = true;
    int i = t_lower.second;
    if (t_lower.first < 0) {
        result.t = t_upper.first;
        i = t_upper.second;
    }
    result.position = ray.origin + (float)result.t*ray.direction;

    
    if (i == 0) {
        result.normal = glm::normalize(glm::vec3(result.position.x, 0.0f, result.position.z));
    } else if (i == 1) {
        result.normal = glm::vec3(0.0f, -1.0f, 0.0f);
    } else {
        result.normal = glm::vec3(0.0f, 1.0f, 0.0f);
    }
    
    return result;
}

double Cylinder::sdf(const glm::vec3 &t) const {
    glm::vec3 d = glm::vec3(abs(glm::vec2(t.y,
        glm::l2Norm(glm::vec3(t.x,0.0f,t.z)))) - glm::vec2(height,radius), 0.0f);
    return fmin(fmax(d.x,d.y),0.0) + glm::l2Norm(vec_max(d, glm::vec3(0.0f)));
}

AABB Cylinder::getAABB() const {
    AABB result;
    result.lower_bound = glm::vec3(-radius, -height, -radius);
    result.upper_bound = glm::vec3(radius, height, radius);
    return result;
}

Cylinder::~Cylinder() {

}