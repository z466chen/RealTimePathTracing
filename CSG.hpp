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

    std::vector<std::pair<glm::mat4, glm::mat4>> t_matrices;

    bool __bboxIntersectionWithRay(const Ray &ray, const AABB &box,
        double &start, double &end) const;
    std::shared_ptr<MaterialInfo> __getMatInfoWithDistance(const glm::vec3 &t, 
        float &dst, bool shouldCalcColor = true) const;
    glm::vec3 __estimateNormal(const glm::vec3 &t) const;

public:
    CSGNode( const std::string & name, CSGNodeType operation);

    void init();

    virtual Intersection intersect(const Ray &ray) const;
    
    virtual AABB getAABB() const;

    virtual int construct(const glm::mat4 &t_matrix) const;

    virtual float getArea(const glm::mat4 &t_matrix) const;
};