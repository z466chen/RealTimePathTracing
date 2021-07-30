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

    virtual void split(std::vector<object_reference> && source, int depth,
        std::vector<object_reference> &left, 
        std::vector<object_reference> &right) const {
        int axis = depth%3;
        auto median_index = source.begin() + source.size()/2;
        std::nth_element(source.begin(), median_index, source.end(), [axis](object_reference &a, object_reference &b){
            AABB a_bound = a.first->getAABB().transform(a.second.first);
            AABB b_bound = b.first->getAABB().transform(b.second.first);

            double a_axis_mean = (a_bound.lower_bound[axis] + a_bound.upper_bound[axis]) * 0.5;
            double b_axis_mean = (b_bound.lower_bound[axis] + b_bound.upper_bound[axis]) * 0.5;
            
            return a_axis_mean > b_axis_mean;
        });

        right = std::vector<object_reference>(std::make_move_iterator(median_index), 
            std::make_move_iterator(source.end()));

        left = std::vector<object_reference>(std::make_move_iterator(source.begin()), 
            std::make_move_iterator(median_index));

        source.clear();
        return;
    };
};

class BinningSAHSplitter: public Splitter {
    int number_of_bins = 6;
public:
    virtual ~BinningSAHSplitter() {};

    virtual void split(std::vector<object_reference> && source, int depth,
        std::vector<object_reference> &left, 
        std::vector<object_reference> &right) const {
        AABB bbox = source[0].first->getAABB().transform(source[0].second.first);
        for (auto obj:source) {
            bbox = bbox + obj.first->getAABB().transform(obj.second.first);
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
                    AABB box = source[j].first->getAABB().transform(source[j].second.first);
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
            AABB box = source[i].first->getAABB().transform(source[i].second.first);
            glm::vec3 center = (box.lower_bound + box.upper_bound)*0.5;
            if (center[best_axis] < best_plane) {
                std::swap(source[i], source[reserve]);
                ++reserve;
            }
        }

        auto split_index = source.begin() + reserve;
        right = std::vector<object_reference>(std::make_move_iterator(split_index), 
            std::make_move_iterator(source.end()));

        left = std::vector<object_reference>(std::make_move_iterator(source.begin()), 
            std::make_move_iterator(split_index));

        source.clear();
        return;
    };
};

std::unique_ptr<Splitter> BVH::splitter = std::make_unique<MediumSplitter>();

BVH::BVHNode * BVH::__recursiveBuild(std::vector<object_reference> &&objs, int depth) const {

    // base case, construct leaf node
    if (objs.empty()) return nullptr;
    if (objs.size() <= LeafNodePrimitiveLimit) {
        BVHNode *leafNode = new BVHNode();
        leafNode->bbox = objs[0].first->getAABB().transform(objs[0].second.first);
        leafNode->objs.emplace_back(std::move(objs[0]));
        for (int i = 1; i < objs.size(); ++i) {
            leafNode->bbox = leafNode->bbox + objs[i].first->getAABB().transform(objs[i].second.first);
            leafNode->objs.emplace_back(std::move(objs[i]));
        }
        leafNode->left = nullptr;
        leafNode->right = nullptr;
        return leafNode;
    }

    std::vector<object_reference> right_objs;
    std::vector<object_reference> left_objs;

    splitter->split(std::move(objs), depth, left_objs, right_objs);

    BVHNode * left = __recursiveBuild(std::move(left_objs), depth+1);
    BVHNode * right = __recursiveBuild(std::move(right_objs), depth+1);

    BVHNode * current = new BVHNode();
    current->bbox = left->bbox + right->bbox;
    current->left = std::unique_ptr<BVHNode>(left);
    current->right = std::unique_ptr<BVHNode>(right);
    return current;
}

BVH::BVH(std::vector<object_reference> &&objs) {
    priority = 1;
    BVHNode *root_raw = __recursiveBuild(std::move(objs), 0);
    this->root = std::unique_ptr<BVHNode>(root_raw);
}

BVH::BVH(std::vector<object_reference> &&objs, int LeafNodePrimitiveLimit) {
    this->LeafNodePrimitiveLimit = LeafNodePrimitiveLimit;
    priority = 1;
    BVHNode *root_raw = __recursiveBuild(std::move(objs), 0);
    this->root = std::unique_ptr<BVHNode>(root_raw);
}

std::vector<object_reference> BVH::__constructObjectList(SceneNode *root) {
    std::queue<std::pair<SceneNode *, std::pair<glm::mat4, glm::mat4>>> q;
    q.emplace(root, std::make_pair(glm::mat4(1.0f), glm::mat4(1.0f)));

    std::vector<object_reference> objs;
    while(!q.empty()) {
        auto node_and_matrix = q.front();
        auto node = node_and_matrix.first;
        auto current_trans_matrix = node_and_matrix.second.first*node->trans;
        auto current_inv_trans_matrix = node->invtrans*node_and_matrix.second.second;
        q.pop();

        if (node->m_nodeType == NodeType::GeometryNode || 
            node->m_nodeType == NodeType::CSGNode) {
            objs.emplace_back(std::make_pair(node, 
                std::make_pair(current_trans_matrix, current_inv_trans_matrix)));
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
            Triangle *triangle = static_cast<Triangle *>(obj.first);
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
            transformed_ray.origin = ptrans(obj.second.second, ray.origin);
            transformed_ray.direction = vtrans(obj.second.second, ray.direction);

            Intersection temp = obj.first->intersect(transformed_ray);

            if (temp.intersects && temp.t < result.t) {
                result = temp;
                result.position = temp.t * ray.direction + ray.origin;
                result.normal = vtrans(glm::transpose(obj.second.second), temp.normal);
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


int BVH::__constructUbo(const BVH::BVHNode *node, const glm::mat4 &t_matrix) const {
    if (!node) return -1;

    if (priority == 0) {
        int id = UboConstructor::bvh_arr.size();
        UboConstructor::bvh_arr.emplace_back(UboBVH());

        UboConstructor::bvh_arr[id].bvh_aabb_1 = glm::vec2(node->bbox.lower_bound.x, node->bbox.lower_bound.y); 
        UboConstructor::bvh_arr[id].bvh_aabb_2 = glm::vec2(node->bbox.lower_bound.z, node->bbox.upper_bound.x); 
        UboConstructor::bvh_arr[id].bvh_aabb_3 = glm::vec2(node->bbox.upper_bound.y, node->bbox.upper_bound.z); 
        
        UboConstructor::bvh_arr[id].obj_id_1 = -1;
        UboConstructor::bvh_arr[id].obj_id_2 = -1;
        UboConstructor::bvh_arr[id].obj_id_3 = -1;
        UboConstructor::bvh_arr[id].obj_id_4 = -1;

        UboConstructor::bvh_arr[id].bvh_left = -1;
        UboConstructor::bvh_arr[id].bvh_right = -1;

        UboConstructor::bvh_arr[id].bvh_left = __constructUbo(node->left.get(), t_matrix);
        UboConstructor::bvh_arr[id].bvh_right = __constructUbo(node->right.get(), t_matrix);

        float *temp[4] = {
            &UboConstructor::bvh_arr[id].obj_id_1, 
            &UboConstructor::bvh_arr[id].obj_id_2, 
            &UboConstructor::bvh_arr[id].obj_id_3, 
            &UboConstructor::bvh_arr[id].obj_id_4
        };

        for (int i = 0; i < node->objs.size(); ++i) {
            *temp[i] = node->objs[i].first->construct(node->objs[i].second.first);
            UboConstructor::obj_arr[int(*temp[i])].t_matrix = node->objs[i].second.first;
            UboConstructor::obj_arr[int(*temp[i])].inv_t_matrix = node->objs[i].second.second;
        }
        return id;
    } else {
        int id = UboConstructor::bvh_mesh_arr.size();
        UboConstructor::bvh_mesh_arr.emplace_back(UboBVH());

        UboConstructor::bvh_mesh_arr[id].bvh_aabb_1 = glm::vec2(node->bbox.lower_bound.x, node->bbox.lower_bound.y); 
        UboConstructor::bvh_mesh_arr[id].bvh_aabb_2 = glm::vec2(node->bbox.lower_bound.z, node->bbox.upper_bound.x); 
        UboConstructor::bvh_mesh_arr[id].bvh_aabb_3 = glm::vec2(node->bbox.upper_bound.y, node->bbox.upper_bound.z); 
        
        UboConstructor::bvh_mesh_arr[id].obj_id_1 = -1;
        UboConstructor::bvh_mesh_arr[id].obj_id_2 = -1;
        UboConstructor::bvh_mesh_arr[id].obj_id_3 = -1;

        UboConstructor::bvh_mesh_arr[id].bvh_left = -1;
        UboConstructor::bvh_mesh_arr[id].bvh_right = -1;

        int left_id = __constructUbo(node->left.get(), t_matrix);
        UboConstructor::bvh_mesh_arr[id].bvh_left = left_id;
        int right_id = __constructUbo(node->right.get(), t_matrix);
        UboConstructor::bvh_mesh_arr[id].bvh_right = right_id;

        if (left_id >= 0 && right_id >= 0) {
            UboConstructor::bvh_mesh_arr[id].obj_id_4 = UboConstructor::bvh_arr[left_id].obj_id_4 + 
                UboConstructor::bvh_arr[right_id].obj_id_4;
        } else {
            float *temp[4] = {
            &UboConstructor::bvh_mesh_arr[id].obj_id_1, 
            &UboConstructor::bvh_mesh_arr[id].obj_id_2, 
            &UboConstructor::bvh_mesh_arr[id].obj_id_3,
            &UboConstructor::bvh_mesh_arr[id].obj_id_4
            };

            float area = 0.0f;
            for (int i = 0; i < node->objs.size(); ++i) {
                *temp[i] = node->objs[i].first->construct(t_matrix);
                area += node->objs[i].first->getArea(t_matrix);
            }
            UboConstructor::bvh_mesh_arr[id].obj_id_4 = area;
        }
        return id;
    }
}

int BVH::construct(const glm::mat4 &t_matrix) const {
    __constructUbo(root.get(), t_matrix);
}