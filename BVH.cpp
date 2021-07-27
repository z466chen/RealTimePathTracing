#include "BVH.hpp"
#include <algorithm>
#include <queue>
#include <climits>
#include <glm/ext.hpp>
#include "general.hpp"
#include "Triangle.hpp"
#include "CSG.hpp"
#include "UboConstructor.hpp"

class MediumSplitter: public Splitter {
public:
    virtual ~MediumSplitter() {};

    virtual void split(std::vector<Object *> && source, int depth,
        std::vector<Object *> &left, 
        std::vector<Object *> &right) const {
        int axis = depth%3;
        auto median_index = source.begin() + source.size()/2;
        std::nth_element(source.begin(), median_index, source.end(), [axis](Object * a, Object *b){
            AABB a_bound = a->getAABB().transform(a->t_matrix);
            AABB b_bound = b->getAABB().transform(b->t_matrix);

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

class BinningSAHSplitter: public Splitter {
    int number_of_bins = 6;
public:
    virtual ~BinningSAHSplitter() {};

    virtual void split(std::vector<Object *> && source, int depth,
        std::vector<Object *> &left, 
        std::vector<Object *> &right) const {
        AABB bbox = source[0]->getAABB().transform(source[0]->t_matrix);
        for (auto obj:source) {
            bbox = bbox + obj->getAABB().transform(obj->t_matrix);
        }

        glm::vec3 size = bbox.upper_bound - bbox.lower_bound;

        float sah = std::numeric_limits<float>::max();
        float best_axis;
        float best_plane; 
        for (int i = 0; i < 3; ++i) {
            float bin_size = size[i]/number_of_bins;
            for (int b = 1; b < number_of_bins; ++b) {
                
                float split_plane = bbox.lower_bound[i] + bin_size*i;

                int reserve = 0;
                float left_s = 0.0f;
                float right_s = 0.0f;
                for (int j = 0; j < source.size(); ++j) {
                    AABB box = source[j]->getAABB().transform(source[j]->t_matrix);
                    glm::vec3 size = box.upper_bound - box.lower_bound;
                    glm::vec3 center = (box.lower_bound + box.upper_bound)*0.5;
                    if (center[i] < split_plane) {
                        std::swap(source[j], source[reserve]);
                        left_s += glm::dot(size, glm::vec3(size.y, size.z, size.x));
                        ++reserve;
                    } else {
                        right_s += glm::dot(size, glm::vec3(size.y, size.z, size.x));
                    }
                }

                float total_sah = reserve * left_s + (source.size() - reserve) * right_s;
                if (total_sah < sah) {
                    sah = total_sah;
                    best_axis = i;
                    best_plane = split_plane;
                }
            }
        }

        int reserve = 0;
        for (int i = 0; i < source.size(); ++i) {
            AABB box = source[i]->getAABB().transform(source[i]->t_matrix);
            glm::vec3 center = (box.lower_bound + box.upper_bound)*0.5;
            if (center[best_axis] < best_plane) {
                std::swap(source[i], source[reserve]);
                ++reserve;
            }
        }

        auto split_index = source.begin() + reserve;
        right = std::vector<Object *>(std::make_move_iterator(split_index), 
            std::make_move_iterator(source.end()));

        left = std::vector<Object *>(std::make_move_iterator(source.begin()), 
            std::make_move_iterator(split_index));

        source.clear();
        return;
    };
};

std::unique_ptr<Splitter> BVH::splitter = std::make_unique<MediumSplitter>();

BVH::BVHNode * BVH::__recursiveBuild(std::vector<Object *> &&objs, int depth) const {

    // base case, construct leaf node
    if (objs.empty()) return nullptr;
    if (objs.size() <= LeafNodePrimitiveLimit) {
        BVHNode *leafNode = new BVHNode();
        leafNode->bbox = objs[0]->getAABB().transform(objs[0]->t_matrix);
        leafNode->objs.emplace_back(objs[0]);
        for (int i = 1; i < objs.size(); ++i) {
            leafNode->bbox = leafNode->bbox + objs[i]->getAABB().transform(objs[i]->t_matrix);
            leafNode->objs.emplace_back(objs[i]);
        }
        leafNode->left = nullptr;
        leafNode->right = nullptr;
        return leafNode;
    }

    std::vector<Object *> right_objs;
    std::vector<Object *> left_objs;

    splitter->split(std::move(objs), depth, left_objs, right_objs);

    BVHNode * left = __recursiveBuild(std::move(left_objs), depth+1);
    BVHNode * right = __recursiveBuild(std::move(right_objs), depth+1);

    BVHNode * current = new BVHNode();
    current->bbox = left->bbox + right->bbox;
    current->left = std::unique_ptr<BVHNode>(left);
    current->right = std::unique_ptr<BVHNode>(right);
    return current;
}

BVH::BVH(std::vector<Object *> &&objs) {
    priority = 1;
    BVHNode *root_raw = __recursiveBuild(std::move(objs), 0);
    this->root = std::unique_ptr<BVHNode>(root_raw);
}

std::vector<Object *> BVH::__constructObjectList(SceneNode *root) {
    std::queue<std::pair<SceneNode *, std::pair<glm::mat4, glm::mat4>>> q;
    q.emplace(root, std::make_pair(glm::mat4(1.0f), glm::mat4(1.0f)));

    std::vector<Object *> objs;
    while(!q.empty()) {
        auto node_and_matrix = q.front();
        auto node = node_and_matrix.first;
        auto current_trans_matrix = node_and_matrix.second.first*node->trans;
        auto current_inv_trans_matrix = node->invtrans*node_and_matrix.second.second;
        q.pop();

        if (node->m_nodeType == NodeType::GeometryNode || 
            node->m_nodeType == NodeType::CSGNode) {

            node->t_matrix = current_trans_matrix;
            node->inv_t_matrix = current_inv_trans_matrix;
            objs.emplace_back(node);
        }

        if (node->m_nodeType != NodeType::CSGNode) {
            for (auto child: node->children) { 
                q.emplace(child, std::make_pair(current_trans_matrix, current_inv_trans_matrix));
            }
        } else {
            static_cast<CSGNode *>(node)->init();
        }
    }
    return objs;
}

BVH::BVH(SceneNode *root): BVH(__constructObjectList(root)) {
    priority = 0;
}

double BVH::sdf(const glm::vec3 &t) const {
    std::queue<BVHNode *> queue;
    queue.emplace(root.get());

    double result = std::numeric_limits<double>::max();

    while(!queue.empty()) {
        auto node = queue.front();
        queue.pop();

        if (!node) continue;
        double boxSdf = node->bbox.sdf(t);
        if (boxSdf > 0) {
            result = fmin(result,boxSdf);
            continue;
        }

        for (auto obj: node->objs) {
            Triangle *triangle = static_cast<Triangle *>(obj);
            result = fmin(result, triangle->sdf(t));
        }

        queue.emplace(node->left.get());
        queue.emplace(node->right.get());
    }
    return result;    
}

Intersection BVH::intersect(const Ray &ray) const {
    std::queue<BVHNode *> queue;
    queue.emplace(root.get());

    Intersection result;
    result.t = std::numeric_limits<double>::max();

    while(!queue.empty()) {
        auto node = queue.front();
        queue.pop();

        if (!node || !node->bbox.isIntersect(ray)) continue;

        for (auto obj: node->objs) {
            Ray transformed_ray;
            transformed_ray.origin = ptrans(obj->inv_t_matrix, ray.origin);
            transformed_ray.direction = vtrans(obj->inv_t_matrix, ray.direction);

            Intersection temp = obj->intersect(transformed_ray);

            if (temp.intersects && temp.t < result.t) {
                result = temp;
                result.position = temp.t * ray.direction + ray.origin;
                result.normal = vtrans(glm::transpose(obj->inv_t_matrix), temp.normal);
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


int BVH::__constructUbo(const BVH::BVHNode *node) const {
    if (!node) return -1;

    UboBVH * temp_1 = nullptr;
    int id = -1;
    if (priority == 0) {
        id = UboConstructor::bvh_arr.size();
        UboConstructor::bvh_arr.emplace_back(UboBVH());
        temp_1 = &UboConstructor::bvh_arr.back();
    } else {
        id = UboConstructor::bvh_mesh_arr.size();
        UboConstructor::bvh_mesh_arr.emplace_back(UboBVH());
        temp_1 = &UboConstructor::bvh_mesh_arr.back();
    }

    auto & ubo_bvh = *temp_1;

    ubo_bvh.bvh_aabb_1 = glm::vec2(node->bbox.lower_bound.x, node->bbox.lower_bound.y); 
    ubo_bvh.bvh_aabb_2 = glm::vec2(node->bbox.lower_bound.y, node->bbox.upper_bound.x); 
    ubo_bvh.bvh_aabb_3 = glm::vec2(node->bbox.upper_bound.y, node->bbox.upper_bound.z); 
    
    ubo_bvh.obj_id_1 = -1;
    ubo_bvh.obj_id_2 = -1;
    ubo_bvh.obj_id_3 = -1;
    ubo_bvh.obj_id_4 = -1;

    ubo_bvh.bvh_left = -1;
    ubo_bvh.bvh_right = -1;

    ubo_bvh.bvh_left = __constructUbo(node->left.get());
    ubo_bvh.bvh_right = __constructUbo(node->right.get());

    float *temp[4] = {
        &ubo_bvh.obj_id_1, 
        &ubo_bvh.obj_id_2, 
        &ubo_bvh.obj_id_3, 
        &ubo_bvh.obj_id_4
    };

    for (int i = 0; i < node->objs.size(); ++i) {
        *temp[i] = node->objs[i]->construct();
    }
    return id;
}

int BVH::construct() const {
    __constructUbo(root.get());
}