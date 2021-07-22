// Termm--Fall 2020

#include "PhongMaterial.hpp"
#include <iostream>

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

std::shared_ptr<MaterialInfo> PhongMaterial::getMaterialInfo(const glm::vec3 &t, 
	const Primitive *primitive) const {
	auto result = std::make_shared<PhongMaterialInfo>();
	result->kd = m_kd;
	result->ks = m_ks;
	result->ior = ior;
	result->shininess = m_shininess;
	result->type = this->type;
	return result;
}
