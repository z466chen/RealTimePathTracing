// Termm--Fall 2020

#pragma once

#include "SceneNode.hpp"
#include "Primitive.hpp"
#include "Material.hpp"

class GeometryNode : public SceneNode {

public:
	GeometryNode( const std::string & name, Primitive *prim, 
		Material *mat = nullptr );

	virtual ~GeometryNode();
	void setMaterial( Material *material );

	Material *m_material;
	Primitive *m_primitive;

	std::shared_ptr<MaterialInfo> getMaterialInfo(const glm::vec3 &t) const;
	virtual Intersection intersect(const Ray &ray) const ;

	virtual AABB getAABB() const;
	virtual int construct(const glm::mat4 &t_matrix) const;
	virtual float getArea(const glm::mat4 &t_matrix) const;
};
