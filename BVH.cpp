#include "BVH.hpp"
#include <algorithm>
#include <queue>
#include <climits>
#include <glm/ext.hpp>

class MediumSplitter: public Splitter {
public:
    virtual ~MediumSplitter() {};

    virtual void split(std::vector<Object *> && source, int axis,
        std::vector<Object *> &left, 
        std::vector<Object *> &right) const {

        auto median_index = source.begin() + source.size()/2;
        std::nth_element(source.begin(), median_index, source.end(), [axis](Object * a, Object *b){
            AABB a_bound = a->getAABB();
            AABB b_bound = b->getAABB();

            double a_axis_mean = (a_bound.lower_bound[axis] + a_bound.upper_bound[axis]) * 0.5;
            double b_axis_mean = (b_bound.lower_bound[axis] + b_bound.upper_bound[axis]) * 0.5;
            
            return a_axis_mean > b_axis_mean;
        });

        right = std::vector<Object *>(std::make_move_iterator(median_index), 
            std::make_move_iterator(source.end()));

        left = std::vector<Object *>(std::make_move_iterator(source.begin()), 
            std::make_move_iterator(median_index));

        source.clear();
        return;
    };
};

std::unique_ptr<Splitter> BVH::splitter = std::make_unique<MediumSplitter>();

BVH::BVHNode * BVH::__recursiveBuild(std::vector<Object *> &&objs, int axis) const {
    // base case, construct leaf node
    if (objs.empty()) return nullptr;
    if (objs.size() <= LeafNodePrimitiveLimit) {
        BVHNode *leafNode = new BVHNode();
        leafNode->bbox = objs[0]->getAABB();
        leafNode->objs.emplace_back(objs[0]);
        for (int i = 1; i < objs.size(); ++i) {
            leafNode->bbox = leafNode->bbox + objs[i]->getAABB();
            leafNode->objs.emplace_back(objs[i]);
        }
        leafNode->left = nullptr;
        leafNode->right = nullptr;
        return leafNode;
    }

    std::vector<Object *> right_objs;
    std::vector<Object *> left_objs;

    splitter->split(std::move(objs), axis, left_objs, right_objs);

    BVHNode * left = __recursiveBuild(std::move(left_objs), (axis + 1)%3);
    BVHNode * right = __recursiveBuild(std::move(right_objs), (axis + 1)%3);

    BVHNode * current = new BVHNode();
    current->bbox = left->bbox + right->bbox;
    current->left = std::unique_ptr<BVHNode>(left);
    current->right = std::unique_ptr<BVHNode>(right);
    return current;
}

BVH::BVH(std::vector<Object *> &&objs) {
    difnad = 2;
    BVHNode *root_raw = __recursiveBuild(std::move(objs), 0);
    this->root = std::unique_ptr<BVHNode>(root_raw);
}

BVH::BVH(SceneNode *root) {
    // get all objs from root screen node

    std::queue<SceneNode *> q;
    q.emplace(root);

    std::vector<Object *> objs;
    while(!q.empty()) {
        auto node = q.front();
        q.pop();
        if (node->m_nodeType == NodeType::GeometryNode) {
            objs.emplace_back(node);
        }

        for (auto child: node->children) {
            q.emplace(child);
        }
    }

    BVHNode *root_raw = __recursiveBuild(std::move(objs), 0);
    this->root = std::unique_ptr<BVHNode>(root_raw);
}

Intersection BVH::intersect(const Ray &ray) {
    std::queue<BVHNode *> queue;
    queue.emplace(root.get());

    Intersection result;
    result.t = std::numeric_limits<double>::max();

    while(!queue.empty()) {
        auto node = queue.front();
        queue.pop();

        if (!node || !node->bbox.isIntersect(ray)) continue;

        for (auto obj: node->objs) {
            Intersection temp = obj->intersect(ray);
            if (temp.intersects && temp.t < result.t) {
                result = temp;
            }
        }

        queue.emplace(node->left.get());
        queue.emplace(node->right.get());
    }
    return result;
}

AABB BVH::getAABB() const {
    return root->bbox;
}
