#include "SolidMaterial.hpp"
#include "Primitive.hpp"
#include "UboConstructor.hpp"

SolidMaterial::SolidMaterial(glm::vec3 periods,double turbPower,double shininess, 
    MaterialType type):periods{periods},plng{},m_turb_power{turbPower}, m_shininess{shininess} {
    this->type = type;
    
    id = UboConstructor::mat_arr.size();
    UboConstructor::mat_arr.emplace_back(UboMaterial());
    auto &ubo_mat = UboConstructor::mat_arr.back();
    ubo_mat.mat_data_1 = glm::vec2(periods.x, periods.y);
    ubo_mat.mat_data_2 = glm::vec2(periods.z, ior);
    ubo_mat.mat_data_3 = glm::vec2(m_shininess, m_turb_power);
    ubo_mat.mat_type = (int) ((type == MaterialType::DIFFUSE)? 
    UboMaterialType::SOLID_DIFFUSE:UboMaterialType::SOLID_SPECULAR);
} 

SolidMaterial::~SolidMaterial() {};

std::shared_ptr<MaterialInfo> SolidMaterial::getMaterialInfo(
    const glm::vec3 &t, 
    const Primitive * primitive) const {
    std::shared_ptr<PhongMaterialInfo> result = std::make_shared<PhongMaterialInfo>();

    AABB bbox = primitive->getAABB();
    glm::vec3 size = glm::max(bbox.upper_bound - bbox.lower_bound, (float)EPSILON);
    glm::vec3 local_coord = t - bbox.lower_bound;
    double seed = glm::dot(local_coord, glm::vec3(periods.x/size.x, periods.y/size.y, periods.z/size.z)) + 
        m_turb_power * (plng.turbulence(local_coord));
    result->kd = glm::vec3(fabs(sin(seed*M_PI))); 
    result->ks = glm::vec3(fabs(sin(seed*M_PI)));
    result->ior = ior;
    result->shininess = m_shininess;
    return result;
}

int SolidMaterial::construct() const {
   return id;
}