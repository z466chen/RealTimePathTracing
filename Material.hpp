// Termm--Fall 2020

#pragma once

// List of supported material type
enum class MaterialType {
  DIFFUSE = 0,
  SPECULAR
};

class Material {
public:
  MaterialType type;
  virtual ~Material();

  void snell(const double & theta_i, double & theta_t, float n1, float n2) const;
  void fresnel(const double &theta_i, 
    const double &theta_t, double &r_eff, float n1, float n2) const;
protected:
  Material();
};
