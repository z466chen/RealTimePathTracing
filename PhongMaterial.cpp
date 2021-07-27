// Termm--Fall 2020

#include "PhongMaterial.hpp"
#include <iostream>
#include "UboConstructor.hpp"

PhongMaterial::PhongMaterial(
	const glm::vec3& kd, const glm::vec3& ks, double shininess, MaterialType type)
	: m_kd(kd)
	, m_ks(ks)
	, m_shininess(shininess)
{
	this->type = type;
	id = UboConstructor::mat_arr.size();
	UboConstructor::mat_arr.emplace_back(UboMaterial());
	auto & ubo_mat = UboConstructor::mat_arr.back();
	ubo_mat.mat_data_1 = glm::vec2(m_kd.x, m_kd.y);
	ubo_mat.mat_data_2 = glm::vec2(m_kd.z, m_ks.x);
	ubo_mat.mat_data_3 = glm::vec2(m_kd.y, m_kd.z);
	ubo_mat.mat_data_4 = glm::vec2(ior, m_shininess);
	ubo_mat.mat_type = (int) ((type == MaterialType::DIFFUSE)? 
    UboMaterialType::PHONG_DIFFUSE:UboMaterialType::PHONG_SPECULAR);
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

int PhongMaterial::construct() const {
	return id;
}
