#pragma once
#include "Material.hpp"
#include <glm/glm.hpp>

class LightMaterial : public Material {
public:
  LightMaterial(const glm::vec3& emission);
  virtual ~LightMaterial() {};

  glm::vec3 emission;
  int id;

  virtual std::shared_ptr<MaterialInfo> getMaterialInfo(const glm::vec3 &t, 
    const Primitive * primitive) const;

  virtual int construct() const;
private:
};
