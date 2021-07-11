// Termm--Fall 2020

#include "PhongMaterial.hpp"

PhongMaterial::PhongMaterial(
	const glm::vec3& kd, const glm::vec3& ks, double shininess, MaterialType type)
	: m_kd(kd)
	, m_ks(ks)
	, m_shininess(shininess)
{
	this->type = type;
}

PhongMaterial::~PhongMaterial()
{}
