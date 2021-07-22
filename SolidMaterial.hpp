#pragma once
#include "Material.hpp"
#include "PerlinNoiseGenerator.hpp"


class SolidMaterial: public Material {
  PerlinNoiseGenerator plng;
  glm::vec3 periods;
public:
  SolidMaterial(glm::vec3 periods,double turbPower,double shininess, MaterialType type);
  virtual ~SolidMaterial();

  double ior = 2.0;

  double m_shininess;
  double m_turb_power;

  std::shared_ptr<MaterialInfo> getMaterialInfo(const glm::vec3 &t, 
    const Primitive * primitive) const;
private:
};