#include "LightMaterial.hpp"
#include "UboConstructor.hpp"

LightMaterial::LightMaterial(const glm::vec3 &emission): emission{emission} {
    this->type = MaterialType::LIGHT;
	id = UboConstructor::mat_arr.size();
	UboConstructor::mat_arr.emplace_back(UboMaterial());
	UboConstructor::mat_arr[id].mat_data_1 = glm::vec2(emission.x, emission.y);
	UboConstructor::mat_arr[id].mat_data_2 = glm::vec2(emission.z, 0.0f);
	UboConstructor::mat_arr[id].mat_type = (int) (UboMaterialType::LIGHT);
}

std::shared_ptr<MaterialInfo> LightMaterial::getMaterialInfo(const glm::vec3 &t, 
    const Primitive * primitive) const {
        return nullptr;
}

int LightMaterial::construct() const {
    return id;
}