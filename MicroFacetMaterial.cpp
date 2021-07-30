#include "MicroFacetMaterial.hpp"
#include "UboConstructor.hpp"


MicroFacetMaterial::MicroFacetMaterial(const glm::vec3 &f0, float h_alpha): 
    f0{f0}, h_alpha{h_alpha} {
	id = UboConstructor::mat_arr.size();
	UboConstructor::mat_arr.emplace_back(UboMaterial());
	UboConstructor::mat_arr[id].mat_data_1 = glm::vec2(f0.x, f0.y);
	UboConstructor::mat_arr[id].mat_data_2 = glm::vec2(f0.z, h_alpha);
	UboConstructor::mat_arr[id].mat_type = (int) UboMaterialType::MICROFACET;
}

std::shared_ptr<MaterialInfo> MicroFacetMaterial::getMaterialInfo(const glm::vec3 &t, 
    const Primitive * primitive) const {
    return nullptr;
}

int MicroFacetMaterial::construct() const {
    return id;
}