// Termm--Fall 2020

#include "GeometryNode.hpp"
#include <iostream>
#include <glm/ext.hpp>
#include "UboConstructor.hpp"

//---------------------------------------------------------------------------------------
GeometryNode::GeometryNode(
	const std::string & name, Primitive *prim, Material *mat )
	: SceneNode( name )
	, m_material( mat )
	, m_primitive( prim )
{
	m_nodeType = NodeType::GeometryNode;
}

GeometryNode::~GeometryNode() {
	delete m_primitive;
}

void GeometryNode::setMaterial( Material *mat )
{
	// Obviously, there's a potential memory leak here.  A good solution
	// would be to use some kind of reference counting, as in the 
	// C++ shared_ptr.  But I'm going to punt on that problem here.
	// Why?  Two reasons:
	// (a) In practice we expect the scene to be constructed exactly
	//     once.  There's no reason to believe that materials will be
	//     repeatedly overwritten in a GeometryNode.
	// (b) A ray tracer is a program in which you compute once, and 
	//     throw away all your data.  A memory leak won't build up and
	//     crash the program.

	m_material = mat;
}

std::shared_ptr<MaterialInfo> GeometryNode::getMaterialInfo(const glm::vec3 &t) const {
	return m_material->getMaterialInfo(t, m_primitive);
}

Intersection GeometryNode::intersect(const Ray &ray) const {
	Intersection result = m_primitive->intersect(ray);
	// std::cout << "gnode: " << result.intersects << std::endl;
	if (result.intersects) {
		result.matInfo = getMaterialInfo(result.position);
	}
	return result;
}

AABB GeometryNode::getAABB() const {
	std::string name = m_name;
	AABB bbox = m_primitive->getAABB();
	return bbox;
}

int GeometryNode::construct(const glm::mat4 &t_matrix) const {
	std::cout << m_name << std::endl;

	int pid = m_primitive->construct(t_matrix);
	int mid = m_material->construct();
	UboConstructor::obj_arr[pid].mat_id = mid;
	if (m_material->type == MaterialType::LIGHT) {
		UboConstructor::light_arr.emplace_back(UboLight());
		UboConstructor::light_arr.back().oid_and_area = glm::vec4(float(pid),getArea(t_matrix),0,0);
	}
	UboConstructor::obj_arr[pid].obj_data_3.x = getArea(t_matrix);

	return pid;
}

float GeometryNode::getArea(const glm::mat4 &t_matrix) const {
	return m_primitive->getArea(t_matrix);
}