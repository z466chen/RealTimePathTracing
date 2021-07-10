// Termm--Fall 2020

#pragma once

#include <glm/glm.hpp>
#include "Ray.hpp"

class Primitive {
public:
  virtual Intersection intersect(Ray ray) { return Intersection(); };
  virtual ~Primitive();
};

class Sphere : public Primitive {
public:
  virtual ~Sphere();
};

class Cube : public Primitive {
public:
  virtual ~Cube();
};

class NonhierSphere : public Primitive {
public:
  NonhierSphere(const glm::vec3& pos, double radius)
    : m_pos(pos), m_radius(radius)
  {
  }

  virtual Intersection intersect(Ray ray);
  virtual ~NonhierSphere();
private:
  glm::vec3 m_pos;
  double m_radius;
};

class NonhierBox : public Primitive {
public:
  NonhierBox(const glm::vec3& pos, double size)
    : m_pos(pos), m_size(size)
  {
  }
  
  virtual Intersection intersect(Ray ray);
  virtual ~NonhierBox();

private:
  glm::vec3 m_pos;
  double m_size;
};
