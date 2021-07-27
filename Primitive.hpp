// Termm--Fall 2020

#pragma once

#include <glm/glm.hpp>
#include "Object.hpp"
#include <climits>

class Primitive: public Object {
public:
  virtual Intersection intersect(const Ray &ray) const { return Intersection(); };
  virtual double sdf(const glm::vec3 &t) const { return std::numeric_limits<double>::max();}; 
  virtual AABB getAABB() const { return AABB(); };
  virtual int construct() const { return -1; }
  virtual ~Primitive();
};

class NonhierSphere : public Primitive {
public:
  NonhierSphere(const glm::vec3& pos, double radius)
    :m_pos(pos), m_radius(radius)
  {
  }

  virtual Intersection intersect(const Ray &ray) const;
  virtual double sdf(const glm::vec3 &t) const;
  virtual AABB getAABB() const;
  virtual int construct() const;
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
  virtual double sdf(const glm::vec3 &t) const;
  virtual AABB getAABB() const;
  virtual int construct() const;
  virtual ~NonhierBox();

private:
  glm::vec3 m_pos;
  double m_size;
};

class Sphere : public Primitive {
  static const NonhierSphere content;
public:
  virtual Intersection intersect(const Ray &ray) const;
  virtual double sdf(const glm::vec3 &t) const;
  virtual AABB getAABB() const;
  virtual int construct() const;
  virtual ~Sphere();
};

class Cube : public Primitive {
  static const NonhierBox content;
public:
  virtual Intersection intersect(const Ray &ray) const;
  virtual double sdf(const glm::vec3 &t) const;
  virtual AABB getAABB() const;
  virtual int construct() const;
  virtual ~Cube();
};

// RoundBox Centred at (0,0,0)
class RoundBox: public Primitive {
  glm::vec3 size;
  float radius;
public:
  RoundBox(const glm::vec3 &size, float radius);
  virtual Intersection intersect(const Ray &ray) const;
  virtual double sdf(const glm::vec3 &t) const;
  virtual AABB getAABB() const;
  virtual int construct() const;
  virtual ~RoundBox();
};

// Cylinder Centred at (0,0,0)
class Cylinder: public Primitive {
  float radius;
  float height;
public:
  Cylinder(float height,float radius);
  virtual Intersection intersect(const Ray &ray) const;
  virtual double sdf(const glm::vec3 &t) const;
  virtual AABB getAABB() const;
  virtual int construct() const;
  virtual ~Cylinder();
};


// // Torus Centred at (0,0,0)
class Torus: public Primitive {
  glm::vec2 parameters;
public:
  Torus(float rx,float ry);
  virtual Intersection intersect(const Ray &ray) const;
  virtual double sdf(const glm::vec3 &t) const;
  virtual AABB getAABB() const;
  virtual int construct() const;
  virtual ~Torus();
};