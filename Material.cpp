// Termm--Fall 2020

#include <glm/glm.hpp>
#include "Material.hpp"


Material::Material()
{}

void Material::snell(const double &theta_i, double & theta_t, float n1, float n2) const {
	double sin_i = sin(theta_i);
	double sin_t = n1 * sin_i / n2;
	if (sin_t > 1.0f) sin_t = 1.0f;

	theta_t = asin(sin_t);
}

void Material::fresnel(const double &theta_i, 
	const double &theta_t, double &r_eff, float n1, float n2) const {
	
	double cos_i = cos(theta_i);
	double cos_t = cos(theta_t);
	double divident = n1 * cos_i + n2 * cos_t;
	double rs = pow((n1 * cos_i - n2 * cos_t) / divident, 2.0f);
	double rp = pow((n1 * cos_t - n2 * cos_i) / divident, 2.0f);
	
	r_eff = (rs + rp) * 0.5;
}

Material::~Material()
{}