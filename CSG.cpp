#include "CSG.hpp"
#include <glm/ext.hpp>
#include <queue>
#include "general.hpp"

CSGNode::CSGNode(const std::string & name, CSGNodeType operation): 
    GeometryNode(name, nullptr, nullptr) {
    m_nodeType = NodeType::CSGNode;
    this->operation = operation;
}

bool CSGNode::__bboxIntersectionWithRay(const Ray &ray, const AABB &box,
    double &start, double &end) const {
    glm::vec3 inv_direction = glm::vec3(1/ray.direction.x, 1/ray.direction.y, 1/ray.direction.z);

    glm::vec3 lower_ts = (box.lower_bound - ray.origin) * inv_direction; 
    glm::vec3 upper_ts = (box.upper_bound - ray.origin) * inv_direction;

    if (ray.direction.x < 0) {
        std::swap(lower_ts.x, upper_ts.x);
    }

    if (ray.direction.y < 0) {
        std::swap(lower_ts.y, upper_ts.y);
    }

    if (ray.direction.z < 0) {
        std::swap(lower_ts.z, upper_ts.z);
    }

    auto lowert = fmax(lower_ts.x, fmax(lower_ts.y, lower_ts.z));
    auto uppert = fmin(upper_ts.x, fmin(upper_ts.y, upper_ts.z));;

    const float delta = 1;
    start = (lowert > delta)? lowert:delta;
    end = (uppert > delta)? uppert:delta;
    return (uppert > 0) && (lowert < uppert);   
}

std::shared_ptr<MaterialInfo> CSGNode::__getMatInfoWithDistance(const glm::vec3 &t, 
        float &dst, bool shouldCalcColor) const {

    auto leftNode = *children.begin();
    std::shared_ptr<MaterialInfo> leftInfo;
    if (leftNode->m_nodeType == NodeType::CSGNode) {
        leftInfo = static_cast<CSGNode *>(leftNode)->__getMatInfoWithDistance(t, dst, shouldCalcColor);
    } else {
        GeometryNode *leftNodeReal = static_cast<GeometryNode *>(leftNode);
        glm::vec3 trans_t =  ptrans(leftNodeReal->inv_t_matrix, t);
        // std::cout << glm::to_string(t) << " " << glm::to_string(trans_t) << std::endl;
        dst = leftNodeReal->m_primitive->sdf(trans_t);
        if (shouldCalcColor) {
            leftInfo = leftNodeReal->getMaterialInfo(t);
        }
    }
    

    std::shared_ptr<MaterialInfo> rightInfo;
    float right;
    auto rightNode = *(++children.begin());
    if (rightNode->m_nodeType == NodeType::CSGNode) {
        rightInfo = static_cast<CSGNode *>(rightNode)->__getMatInfoWithDistance(t, right,shouldCalcColor);
    } else {
        GeometryNode *rightNodeReal = static_cast<GeometryNode *>(rightNode);
        glm::vec3 trans_t = ptrans(rightNodeReal->inv_t_matrix, t); 
        right = rightNodeReal->m_primitive->sdf(trans_t);
        if (shouldCalcColor) {
            rightInfo = rightNodeReal->getMaterialInfo(t);
        }
    }

    double r;

    switch (operation) {
    
    case CSGNodeType::INTERSECTION:
        if (dst < right) {
            dst = right;
            return rightInfo;
        }
        
        return leftInfo;
        break;

    case CSGNodeType::DIFFERENCE:
        dst = fmax(dst, -right);

        return leftInfo;
        break;
    case CSGNodeType::UNION:
        if (dst > right) {
            dst = right;
            return rightInfo;
        } 
        return leftInfo;
        break;
    case CSGNodeType::SMOOTH_UNION:
        float k = 50.0f;
        float h = fmax(k - fabs(dst - right), 0.0f) / k;

        if (shouldCalcColor) {
            leftInfo->blendMaterial(rightInfo.get(), 1.0f - h);
        }

        dst = fmin(dst, right) - pow(h, 3)*k*(1/6.0f);
        return leftInfo;
        break;
    }

}

glm::vec3 CSGNode::__estimateNormal(const glm::vec3 &t) const {
    glm::vec3 x_plus = glm::vec3(t.x + EPSILON,t.y,t.z);
    glm::vec3 x_minus = glm::vec3(t.x-EPSILON,t.y,t.z);

    glm::vec3 y_plus = glm::vec3(t.x,t.y+EPSILON,t.z);
    glm::vec3 y_minus = glm::vec3(t.x,t.y-EPSILON,t.z);

    glm::vec3 z_plus = glm::vec3(t.x,t.y,t.z+EPSILON);
    glm::vec3 z_minus = glm::vec3(t.x,t.y,t.z-EPSILON);


    glm::vec3 start;
    glm::vec3 end;
    __getMatInfoWithDistance(x_plus, start.x, false);
    __getMatInfoWithDistance(x_minus, end.x, false);
    __getMatInfoWithDistance(y_plus, start.y, false);
    __getMatInfoWithDistance(y_minus, end.y, false);
    __getMatInfoWithDistance(z_plus, start.z, false);
    __getMatInfoWithDistance(z_minus, end.z, false);
    
   return glm::normalize(start - end);
}

void CSGNode::init() {
    this->t_matrix = trans;
    this->inv_t_matrix = invtrans;

    std::queue<std::pair<SceneNode *, std::pair<glm::mat4, glm::mat4>>> q;
    // q.emplace(this, std::make_pair(glm::mat4(1.0f), glm::mat4(1.0f)));

    for (auto child: this->children) { 
        q.emplace(child, std::make_pair(glm::mat4(1.0f), glm::mat4(1.0f)));
    }

    while(!q.empty()) {
        auto node_and_matrix = q.front();
        auto node = node_and_matrix.first;
        auto current_trans_matrix = node_and_matrix.second.first*node->trans;
        auto current_inv_trans_matrix = node->invtrans*node_and_matrix.second.second;
        q.pop();

        node->t_matrix = current_trans_matrix;
        node->inv_t_matrix = current_inv_trans_matrix;

        for (auto child: node->children) { 
            q.emplace(child, std::make_pair(current_trans_matrix, current_inv_trans_matrix));
        }
    }
    return;
}

Intersection CSGNode::intersect(const Ray &ray) const {
    AABB bbox = getAABB();
    double current,end;
    bool isIntersect = __bboxIntersectionWithRay(ray, bbox, current, end);
    Intersection result;

    if (!isIntersect) return result;
    glm::vec3 t = ray.origin + (float)current * ray.direction;
    // int step = 0;
    while(current < end) {
        float dst;
        result.matInfo = __getMatInfoWithDistance(t, dst);
        if (dst < EPSILON) {
            
            result.normal = __estimateNormal(t);
            result.position = t;
            result.t = current;
            result.intersects = true;
            break;
        }

        current += dst;
        t = ray.origin + (float)current * ray.direction;
        // ++step;
    }
    // std::cout << step << std::endl;
    return result;
}

AABB CSGNode::getAABB() const {
    //std::cout << "---------------------------------" << m_name << std::endl;
    auto node = (*children.begin());
    AABB result;
    if (node->m_nodeType == NodeType::GeometryNode) {
        result = node->getAABB().transform(node->t_matrix);
    //     std::cout << glm::to_string(result.lower_bound) << glm::to_string(node->getAABB().lower_bound) << std::endl;
    } else {
        result = node->getAABB();
    }
      
    // std::cout << "next name:" << (*children.begin())->m_name << std::endl;
    for (auto i = ++children.begin(); i != children.end(); ++i) {
        node = *i;
        if (node->m_nodeType == NodeType::GeometryNode) {
            result = result + node->getAABB().transform(node->t_matrix);
        } else {
            result = result + node->getAABB();
        }
    }
    return result;
}
