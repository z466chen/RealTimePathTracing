#pragma once

#include <vector>
#include <memory>

#include "SceneNode.hpp"

class Splitter {
public:
    virtual ~Splitter() {};
    virtual void split(std::vector<Object *> && source, int axis,
        std::vector<Object *> &left, 
        std::vector<Object *> &right) const = 0;
};

class BVH {

    class BVHNode {
    public:
        AABB bbox;
        
        std::vector<Object *> objs;
        
        std::unique_ptr<BVHNode> left = nullptr;
        std::unique_ptr<BVHNode> right = nullptr;
    };

    static std::unique_ptr<Splitter> splitter;
    std::unique_ptr<BVHNode> root;

    static const int LeafNodePrimitiveLimit = 1;

    BVHNode *__recursiveBuild(std::vector<Object *> &&objs, int axis) const;

    std::vector<Object *> __constructObjectList(SceneNode *root);
public:
    BVH(SceneNode *root);
    BVH(std::vector<Object *> &&objs);

    double sdf(const glm::vec3 &t) const;
    Intersection intersect(const Ray &ray) const;
    AABB getAABB() const;
};