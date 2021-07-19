#pragma once

#include <vector>
#include <memory>

#include "GeometryNode.hpp"

enum class CSGNodeType {
    UNION=0,
    INTERSECTION,
    DIFFERENCE,
    SMOOTH_UNION
};

class CSGNode: public GeometryNode {
    CSGNodeType operation;

    bool __bboxIntersectionWithRay(const Ray &ray, const AABB &box,
        double &start, double &end) const;
    double __getSDFWithMaterial(const glm::vec3 &t, Material ** mat) const;
    glm::vec3 __estimateNormal(const glm::vec3 &t) const;

public:
    CSGNode( const std::string & name, CSGNodeType operation);

    void init();

    virtual Intersection intersect(const Ray &ray) const;
    
    virtual AABB getAABB() const;
};