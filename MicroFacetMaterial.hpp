#pragma once
#include "Material.hpp"

class MicroFacetMaterial : public Material {
public:
  MicroFacetMaterial(const glm::vec3 &f0, float h_alpha);
  virtual ~MicroFacetMaterial(){};

  glm::vec3 f0;

  double ior = 2.0;

  double h_alpha;
  
  int id;

  virtual std::shared_ptr<MaterialInfo> getMaterialInfo(const glm::vec3 &t, 
    const Primitive * primitive) const;

  virtual int construct() const;
private:
};
