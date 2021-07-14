// Termm--Fall 2020

#pragma once

#include <glm/glm.hpp>
#include "Object.hpp"

class Primitive: public Object {
public:
  virtual Intersection intersect(const Ray &ray) const { return Intersection(); };
  virtual AABB getAABB() const { return AABB(); };
  virtual ~Primitive();
};

class NonhierSphere : public Primitive {
public:
  NonhierSphere(const glm::vec3& pos, double radius)
    :m_pos(pos), m_radius(radius)
  {
  }

  virtual Intersection intersect(const Ray &ray) const;
  virtual AABB getAABB() const;
  virtual ~NonhierSphere();
private:
  glm::vec3 m_pos;
  double m_radius;
};

class NonhierBox : public Primitive {
public:
  NonhierBox(const glm::vec3& pos, double size)
    :m_pos(pos), m_size(size)
  {
  }
  
  virtual Intersection intersect(const Ray &ray) const;
  virtual AABB getAABB() const;
  virtual ~NonhierBox();

private:
  glm::vec3 m_pos;
  double m_size;
};

class Sphere : public Primitive {
  static const NonhierSphere content;
public:
  virtual Intersection intersect(const Ray &ray) const;
  virtual AABB getAABB() const;
  virtual ~Sphere();
};

class Cube : public Primitive {
  static const NonhierBox content;
public:
  virtual Intersection intersect(const Ray &ray) const;
  virtual AABB getAABB() const;
  virtual ~Cube();
};