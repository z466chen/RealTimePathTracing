#pragma once

#include <glm/glm.hpp>

// List of supported material type
enum class MaterialType {
  DIFFUSE = 0,
  SPECULAR,
  LIGHT
};

class MaterialInfo {
public:
    MaterialType type;

    void snell(const double &theta_i, double & theta_t, float n1, float n2) const {
        double sin_i = sin(theta_i);
        double sin_t = n1 * sin_i / n2;
        sin_t = glm::clamp((float)sin_t, 0.0f, 1.0f);

        theta_t = asin(sin_t);
    }

    void fresnel(const double &theta_i, 
        const double &theta_t, double &r_eff, float n1, float n2) const {
        
        double cos_i = cos(theta_i);
        double cos_t = cos(theta_t);
        double divident = n1 * cos_i + n2 * cos_t;
        double rs = pow((n1 * cos_i - n2 * cos_t) / divident, 2.0f);
        double rp = pow((n1 * cos_t - n2 * cos_i) / divident, 2.0f);
        
        r_eff = glm::clamp((float)((rs + rp) * 0.5), 0.0f, 1.0f);
    }

    virtual void blendMaterial(const MaterialInfo *other, double h) {};
};

class PhongMaterialInfo: public MaterialInfo {
public:
  glm::vec3 kd;
  glm::vec3 ks;
  double shininess;
  double ior;

  virtual void blendMaterial(const MaterialInfo *other, double h);
};