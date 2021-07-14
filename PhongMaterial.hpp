// Termm--Fall 2020

#pragma once

#include <glm/glm.hpp>

#include "Material.hpp"

class PhongMaterial : public Material {
public:
  PhongMaterial(const glm::vec3& kd, const glm::vec3& ks, double shininess, MaterialType type);
  virtual ~PhongMaterial();

  glm::vec3 m_kd;
  glm::vec3 m_ks;

  double ior = 2.0;

  double m_shininess;
private:
};
