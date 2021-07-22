#include "MaterialInfo.hpp"
#include <glm/ext.hpp>

void PhongMaterialInfo::blendMaterial(const MaterialInfo *other, double h) {
	if (other->type != type) return;
	auto temp = static_cast<const PhongMaterialInfo *>(other);
	kd = glm::lerp(kd, temp->kd, (float)h);
	ks = glm::lerp(ks, temp->ks, (float)h);
	shininess = glm::lerp((float)temp->shininess,(float)temp->shininess, (float)h);
	if (h > 0.5) {
		ior = temp->ior;
	}
}