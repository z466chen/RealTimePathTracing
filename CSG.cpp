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

    const float delta = 5;
    start = (lowert > delta)? lowert:delta;
    end = (uppert > delta)? uppert:delta;
    return (uppert > 0) && (lowert < uppert);   
}

double CSGNode::__getSDFWithMaterial(const glm::vec3 &t, Material ** mat) const {

    Material *leftm;
    double left;
    auto leftNode = *children.begin();
    if (leftNode->m_nodeType == NodeType::CSGNode) {
        left = static_cast<CSGNode *>(leftNode)->__getSDFWithMaterial(t, &leftm);
    } else {
        GeometryNode *leftNodeReal = static_cast<GeometryNode *>(leftNode);
        glm::vec3 trans_t =  ptrans(leftNodeReal->inv_t_matrix, t);
        // std::cout << glm::to_string(t) << " " << glm::to_string(trans_t) << std::endl;
        left = leftNodeReal->m_primitive->sdf(trans_t);
        leftm = leftNodeReal->m_material;
    }
    
    Material *rightm;
    double right;
    auto rightNode = *(++children.begin());
    if (rightNode->m_nodeType == NodeType::CSGNode) {
        right = static_cast<CSGNode *>(rightNode)->__getSDFWithMaterial(t, &rightm);
    } else {
        GeometryNode *rightNodeReal = static_cast<GeometryNode *>(rightNode);
        glm::vec3 trans_t = ptrans(rightNodeReal->inv_t_matrix, t); 
        right = rightNodeReal->m_primitive->sdf(trans_t);
        rightm = rightNodeReal->m_material;
    }

    double r;

    switch (operation) {
    
    case CSGNodeType::INTERSECTION:
        *mat = (left > right)? rightm:leftm;
        return fmax(left, right);
        break;

    case CSGNodeType::DIFFERENCE:
        *mat = leftm;
        r = fmax(left, -right);
        return r;
        break;
    case CSGNodeType::UNION:
        *mat = (left > right)? leftm:rightm;
        return fmin(left, right);
        break;
    case CSGNodeType::SMOOTH_UNION:
        *mat = leftm;
        float k = 50.0f;
        float h = fmax(k - fabs(left - right), 0.0f) / k;
        return fmin(left, right) - pow(h, 3)*k*(1/6.0f);
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

    Material *temp;
    float x = __getSDFWithMaterial(x_plus, &temp) - __getSDFWithMaterial(x_minus, &temp);
    float y = __getSDFWithMaterial(y_plus, &temp) - __getSDFWithMaterial(y_minus, &temp);
    float z = __getSDFWithMaterial(z_plus, &temp) - __getSDFWithMaterial(z_minus, &temp);
    return glm::normalize(glm::vec3(x,y,z));
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
        Material *m = nullptr;
        double dst = __getSDFWithMaterial(t, &m);
        if (dst < EPSILON) {
            result.normal = __estimateNormal(t);
            result.material = m;
            result.position = t;
            // std::cout << glm::to_string(t) << std::endl;
            result.t = current;
            result.intersects = true;
            result.isCSG = true;
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