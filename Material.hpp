// Termm--Fall 2020

#pragma once

#include <memory>
#include "MaterialInfo.hpp"
class Primitive;

class Material {
public:
  MaterialType type;

  virtual std::shared_ptr<MaterialInfo> getMaterialInfo(const glm::vec3 & type, 
    const Primitive *primitive) const { return std::make_shared<MaterialInfo>(); };
  virtual ~Material();

  virtual int construct() const { return -1; };


protected:
  Material();
};
