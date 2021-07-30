#pragma once

#include <vector>
#include <memory>

#include "SceneNode.hpp"
#include "UboConstructor.hpp"

typedef std::pair<Object *, std::pair<glm::mat4, glm::mat4>> object_reference;

class Splitter {
public:
    virtual ~Splitter() {};
    virtual void split(std::vector<object_reference> && source, int depth,
        std::vector<object_reference> &left, 
        std::vector<object_reference> &right) const = 0;
};

class BVH {

    class BVHNode {
    public:
        AABB bbox;
        float area = 0.0f;
        std::vector<object_reference> objs;

        std::unique_ptr<BVHNode> left = nullptr;
        std::unique_ptr<BVHNode> right = nullptr;
    };

    static std::unique_ptr<Splitter> splitter;
    std::unique_ptr<BVHNode> root;

    int LeafNodePrimitiveLimit = 4;

    BVHNode *__recursiveBuild(std::vector<object_reference> &&objs, int depth) const;

    std::vector<object_reference> __constructObjectList(SceneNode *root);

    int __constructUbo(const BVHNode *node, const glm::mat4 &t_matrix) const;

    int priority = 0;
public:
    BVH(SceneNode *root);
    BVH(std::vector<object_reference> &&objs);
    BVH(std::vector<object_reference> &&objs, int LeafNodePrimitiveLimit);
    

    double sdf(const glm::vec3 &t) const;
    Intersection intersect(const Ray &ray) const;
    AABB getAABB() const;

    int construct(const glm::mat4 &t_matrix) const;
};