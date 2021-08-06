#version 330

#define FLT_MAX 3.402823466e+38
#define FLT_MIN -3.402823466e+38
#define EPSILON 0.01
#define M_PI 3.14159265358979323846

#define RUSSIAN_ROULETTE 0.8
#define SPP 1

#define B_SIZE 0x100
#define BM 0xff
#define PN 0x1000
#define setup(i,b0,b1,r0,r1) temp = t[i] + PN; b0 = (int(temp)) & BM;b1 = (b0+1) & BM;r0 = temp - int(temp);r1 = r0 - 1.0f;
#define s_curve(t) ( t * t * (3. - 2. * t) )
#define lerp(t, a, b) ( a + t * (b - a) )

#define BVH_STACK_SIZE 50
#define BVH_MESH_STACK_SIZE 50
#define CSG_STACK_SIZE 20
#define CAST_RAY_STACK_SIZE 50
#define STACK_SIZE 200

#define OBJ_TEXTURE_SIZE {OBJ_TEXTURE_SIZE}
#define VERT_TEXTURE_SIZE {VERT_TEXTURE_SIZE}
#define ELEM_TEXTURE_SIZE {ELEM_TEXTURE_SIZE}
#define BVH_TEXTURE_SIZE {BVH_TEXTURE_SIZE}
#define BVH_MESH_TEXTURE_SIZE {BVH_MESH_TEXTURE_SIZE}

// #define OBJ_TEXTURE_SIZE 128
// #define VERT_TEXTURE_SIZE 128
// #define ELEM_TEXTURE_SIZE 128
// #define BVH_TEXTURE_SIZE 1
// #define BVH_MESH_TEXTURE_SIZE 1

// A non-const zero to prevent inline functions
// #define ZERO (min(int(gl_FragCoord.x),0))

#define ZERO 0

uniform sampler2D obj_tex;
uniform sampler2D vert_tex;
uniform sampler2D elem_tex;
// uniform sampler2D perlin_tex;
uniform sampler2D bvh_tex;
uniform sampler2D bvh_mesh_tex;
uniform sampler2D bump_map_tex;
// uniform sampler2D bg_tex;
// uniform sampler2D indata1;
// uniform sampler2D indata2;
// uniform sampler2D indata3;


struct MaterialNode {
    vec2 mat_data_1; // 0
    vec2 mat_data_2; // 8
    vec2 mat_data_3; // 16
    vec2 mat_data_4; // 24
    float mat_type; // 32
        // 36
};

struct BVHNode {
    vec2 bvh_aabb_1; // 0
    vec2 bvh_aabb_2; // 8
    vec2 bvh_aabb_3; // 16
    float bvh_left; // 24
    float bvh_right; // 28
    float obj_id_1; // 32
    float obj_id_2; // 36
    float obj_id_3; // 40
    float obj_id_4; // 44
                // 48
};

struct LightNode {
    vec4 oid_and_area;
};

struct Ray {
    vec3 ro;
    vec3 rd;
};

struct MaterialInfo {
    vec3 kd;
    vec3 ks;
    float shininess;
    float ior;
    int m_type;
    bool isEmission;
};

struct Intersection {
    vec3 normal;
    vec3 position;
    float t;
    MaterialInfo m;
    bool intersects;
    int id;
};

struct AABB {
    vec3 lower_bound;
    vec3 upper_bound;
};

struct Object{
    mat4 t_matrix; // 0
    mat4 inv_t_matrix; // 64
    AABB bbox; // 128
    vec2 obj_data_1; // 152
    vec2 obj_data_2; // 160
    vec2 obj_data_3; // 168
    int obj_type; // 176
    int mat_id; // 180
                // 184
};

struct LocalBVHMeshNode {
    AABB bbox;
    float area;
    int left_id;
    int right_id;
    int obj_ids[3];
};

struct LocalBVHNode {
    AABB bbox;
    int left_id;
    int right_id;
    int obj_ids[4];
};

struct PerlinNoiseNode {
    vec3 g;
    float p;
};

layout (std140) uniform Material {
    MaterialNode mat_node [100];
};

layout (std140) uniform BVH {
    BVHNode bvh_node[1024];
};

layout (std140) uniform BVHMesh {
    BVHNode bvh_mesh_node[1024];
};

layout (std140) uniform PerlinNoise {
    PerlinNoiseNode perlin_node[B_SIZE+B_SIZE+2];
};

layout (std140) uniform Camera {
    mat4 viewMatrix; // 0
    vec3 eyePos; // 64
    float fov; // 76
            // 80
};

layout (std140) uniform Light {
    LightNode light_node[1024];
};

uniform vec3 ambient;
uniform int num_of_lights;
uniform float total_light_area;
uniform vec4 seed;
uniform mat4 perspective;
uniform vec2 window_size;

// input/outputs
in vec2 texCoord;

layout (location = 0) out vec4 outdata1;
layout (location = 1) out vec4 outdata2;
layout (location = 2) out vec4 outdata3;
layout (location = 3) out vec4 outdata4;
layout (location = 4) out vec4 outdata5;
layout (location = 5) out vec4 outdata6;
layout (location = 6) out vec4 outdata7;

struct Sample {
    vec3 xv, nv, xs, ns, lo, color;
    int oid, mid;
};

struct GBufferData {
    vec3 position;
    vec3 normal;
    int id;
};

void pack_output(in Sample result) {
    outdata1 = vec4(result.xv.rgb, 1.0f);
    outdata2 = vec4(result.nv.rgb, 1.0f);
    outdata3 = vec4(result.xs.rgb, 1.0f);
    outdata4 = vec4(result.ns.rgb, 1.0f);
    outdata5 = vec4(result.lo.rgb, 1.0f);
    outdata6 = vec4(float(result.oid), float(result.mid), 0.0f, 1.0f);
    outdata7 = vec4(result.color.rgb, 1.0f);
}

// void read_input(out GBufferData data) {
//     data.position = texture(indata1, vec2(texCoord.x, 1 - texCoord.y)).rgb;
//     data.normal = texture(indata2, vec2(texCoord.x, 1 - texCoord.y)).rgb;
//     data.id = int(texture(indata3, vec2(texCoord.x, 1 - texCoord.y)).r) - 1;
// }

float _stack[STACK_SIZE];
int _top = -1;

bool is_stack_empty() {
    return _top == -1;
}

void stackClear() {
    _top = -1;
}

bool stackEmpty() {
    return (_top == -1);
}

float stackTop() {
    return _stack[_top];
}

void stackPush(float f) {
    ++_top;
    _stack[_top] = f;
}

void stackPop(out float f) {
    f = _stack[_top];
    --_top;
}

void stackPush(int f) {
    ++_top;
    _stack[_top] = float(f);
}

void stackPop(out int f) {
    f = int(_stack[_top]);
    --_top;
}

void stackPush(in vec3 v) {
    stackPush(v.x);
    stackPush(v.y);
    stackPush(v.z);
}

void stackPop(out vec3 v) {
    stackPop(v.z);
    stackPop(v.y);
    stackPop(v.x);
}

void stackPush(in Ray ray) {
    stackPush(ray.ro);
    stackPush(ray.rd);
}

void stackPop(out Ray ray) {
    stackPop(ray.rd);
    stackPop(ray.ro);
}

float rand_seed_y = 0.0f;

float rand_core(in vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

float get_random_float() {
    float a = rand_core(vec2(texCoord.x, seed.x));
    float b = rand_core(vec2(seed.y, texCoord.y));
    float c = rand_core(vec2(rand_seed_y,seed.z));
    rand_seed_y += 10;
    float d = rand_core(vec2(seed.w, a));
    float e = rand_core(vec2(b, c));
    float f = rand_core(vec2(d, e));
    return f;
}


vec3 vtrans(in mat4 m, vec3 v) {
    return (m * vec4(v, 0.0f)).xyz;
}

vec3 ptrans(in mat4 m, vec3 p) {
    vec4 temp = m * vec4(p, 1.0f);
    return temp.xyz / temp.w;
}

float dot2(vec3 t) {
    return dot(t,t);
}

void snell(in float theta_i, out float theta_t, float n1, float n2) {
    float sin_i = sin(theta_i);
    float sin_t = n1 * sin_i / n2;
    sin_t = clamp(sin_t, 0.0f, 1.0f);
    theta_t = asin(sin_t);
}

void fresnel(in float theta_i, in float theta_t, 
    out float r_eff, float n1, float n2) {
    
    float cos_i = cos(theta_i);
    float cos_t = cos(theta_t);
    float divident = n1 * cos_i + n2 * cos_t;
    float rs = pow((n1 * cos_i - n2 * cos_t) / divident, 2.0f);
    float rp = pow((n1 * cos_t - n2 * cos_i) / divident, 2.0f);
    
    r_eff = clamp((rs + rp) * 0.5f, 0.0f, 1.0f);
}

vec4 getVec4FromTexture(in sampler2D texture, int offset, int tex_size) {
    float a = texelFetch(texture, ivec2(offset%tex_size,offset/tex_size), 0).r;
    float b = texelFetch(texture, ivec2((offset+1)%tex_size,(offset+1)/tex_size), 0).r;
    float c = texelFetch(texture, ivec2((offset+2)%tex_size,(offset+2)/tex_size), 0).r;
    float d = texelFetch(texture, ivec2((offset+3)%tex_size,(offset+3)/tex_size), 0).r;
    return vec4(a,b,c,d);
}

vec3 getVec3FromTexture(in sampler2D texture, int offset, int tex_size) {
    float a = texelFetch(texture, ivec2(offset%tex_size,offset/tex_size), 0).r;
    float b = texelFetch(texture, ivec2((offset+1)%tex_size,(offset+1)/tex_size), 0).r;
    float c = texelFetch(texture, ivec2((offset+2)%tex_size,(offset+2)/tex_size), 0).r;
    return vec3(a,b,c);
}

vec2 getVec2FromTexture(in sampler2D texture, int offset, int tex_size) {
    float a = texelFetch(texture, ivec2(offset%tex_size,offset/tex_size), 0).r;
    float b = texelFetch(texture, ivec2((offset+1)%tex_size,(offset+1)/tex_size), 0).r;
    return vec2(a,b);
}

float getFloatFromTexture(in sampler2D texture, int offset, int tex_size) {
    return texelFetch(texture, ivec2(offset%tex_size,offset/tex_size), 0).r;
}

vec3 getVec3FromTexture3Packed(in sampler2D texture, int offset, int tex_size) {
    return texelFetch(texture, ivec2(offset%tex_size,offset/tex_size), 0).rgb;
}

mat4 getMat4FromTexture(in sampler2D texture, int offset, int tex_size) {
    vec4 c1 = getVec4FromTexture(texture, offset, tex_size);
    vec4 c2 = getVec4FromTexture(texture, offset+4, tex_size);
    vec4 c3 = getVec4FromTexture(texture, offset+8, tex_size);
    vec4 c4 = getVec4FromTexture(texture, offset+12, tex_size);
    return mat4(c1,c2,c3,c4);
}

LocalBVHMeshNode getLocalBVHMeshNodeFromTexture(int id) {
    LocalBVHMeshNode node;
    node.bbox.lower_bound = getVec3FromTexture(bvh_mesh_tex, 12*id,BVH_MESH_TEXTURE_SIZE);
    node.bbox.upper_bound = getVec3FromTexture(bvh_mesh_tex, 12*id+3,BVH_MESH_TEXTURE_SIZE);
    node.left_id  = int(getFloatFromTexture(bvh_mesh_tex, 12*id+6,BVH_MESH_TEXTURE_SIZE));
    node.right_id = int(getFloatFromTexture(bvh_mesh_tex, 12*id+7,BVH_MESH_TEXTURE_SIZE));
    node.obj_ids[0] = int(getFloatFromTexture(bvh_mesh_tex, 12*id+8,BVH_MESH_TEXTURE_SIZE));
    node.obj_ids[1] = int(getFloatFromTexture(bvh_mesh_tex, 12*id+9,BVH_MESH_TEXTURE_SIZE));
    node.obj_ids[2] = int(getFloatFromTexture(bvh_mesh_tex, 12*id+10,BVH_MESH_TEXTURE_SIZE));
    node.area = getFloatFromTexture(bvh_mesh_tex, 12*id+11,BVH_MESH_TEXTURE_SIZE);
    return node;
}

LocalBVHNode getLocalBVHNodeFromTexture(int id) {
    LocalBVHNode node;
    node.bbox.lower_bound = getVec3FromTexture(bvh_tex, 12*id,BVH_TEXTURE_SIZE);
    node.bbox.upper_bound = getVec3FromTexture(bvh_tex, 12*id+3,BVH_TEXTURE_SIZE);
    node.left_id  = int(getFloatFromTexture(bvh_tex, 12*id+6,BVH_TEXTURE_SIZE));
    node.right_id = int(getFloatFromTexture(bvh_tex, 12*id+7,BVH_TEXTURE_SIZE));
    node.obj_ids[0] = int(getFloatFromTexture(bvh_tex, 12*id+8,BVH_TEXTURE_SIZE));
    node.obj_ids[1] = int(getFloatFromTexture(bvh_tex, 12*id+9,BVH_TEXTURE_SIZE));
    node.obj_ids[2] = int(getFloatFromTexture(bvh_tex, 12*id+10,BVH_TEXTURE_SIZE));
    node.obj_ids[3] = int(getFloatFromTexture(bvh_tex, 12*id+11,BVH_TEXTURE_SIZE));
    return node;
}

LocalBVHMeshNode getLocalBVHMeshNodeFromUniform(int id) {
    BVHNode node = bvh_mesh_node[id];
    LocalBVHMeshNode localNode;
    localNode.bbox.lower_bound = vec3(node.bvh_aabb_1.xy, node.bvh_aabb_2.x);
    localNode.bbox.upper_bound = vec3(node.bvh_aabb_2.y, node.bvh_aabb_3.xy);
    localNode.obj_ids[0] = int(node.obj_id_1);
    localNode.obj_ids[1] = int(node.obj_id_2);
    localNode.obj_ids[2] = int(node.obj_id_3);
    localNode.area = node.obj_id_4;
    localNode.left_id  = int(node.bvh_left);
    localNode.right_id = int(node.bvh_right);
    return localNode;
}

LocalBVHNode getLocalBVHNodeFromUniform(int id) {
    BVHNode node = bvh_node[id];
    LocalBVHNode localNode;
    localNode.bbox.lower_bound = vec3(node.bvh_aabb_1.xy, node.bvh_aabb_2.x);
    localNode.bbox.upper_bound = vec3(node.bvh_aabb_2.y, node.bvh_aabb_3.xy);
    localNode.obj_ids[0] = int(node.obj_id_1);
    localNode.obj_ids[1] = int(node.obj_id_2);
    localNode.obj_ids[2] = int(node.obj_id_3);
    localNode.obj_ids[3] = int(node.obj_id_4);
    localNode.left_id  = int(node.bvh_left);
    localNode.right_id = int(node.bvh_right);
    return localNode;
}

void getTriangle(int eid, int vert_offset, out vec3 v1, out vec3 v2, out vec3 v3) {
                
    vec3 temp = getVec3FromTexture3Packed(elem_tex, eid,ELEM_TEXTURE_SIZE);
    int i1 = int(temp.x) + vert_offset;
    int i2 = int(temp.y) + vert_offset;
    int i3 = int(temp.z) + vert_offset;
    
    v1 = getVec3FromTexture3Packed(vert_tex, i1,VERT_TEXTURE_SIZE);
    v2 = getVec3FromTexture3Packed(vert_tex, i2,VERT_TEXTURE_SIZE);
    v3 = getVec3FromTexture3Packed(vert_tex, i3,VERT_TEXTURE_SIZE);
}

Object getObject(int id) {
    Object obj;
    obj.t_matrix = getMat4FromTexture(obj_tex, 46*id, OBJ_TEXTURE_SIZE);
    obj.inv_t_matrix = getMat4FromTexture(obj_tex, 46*id+16, OBJ_TEXTURE_SIZE);
    obj.bbox.lower_bound = getVec3FromTexture(obj_tex, 46*id+32, OBJ_TEXTURE_SIZE);
    obj.bbox.upper_bound = getVec3FromTexture(obj_tex, 46*id+35, OBJ_TEXTURE_SIZE);
    obj.obj_data_1 = getVec2FromTexture(obj_tex, 46*id+38, OBJ_TEXTURE_SIZE);
    obj.obj_data_2 = getVec2FromTexture(obj_tex, 46*id+40, OBJ_TEXTURE_SIZE);
    obj.obj_data_3 = getVec2FromTexture(obj_tex, 46*id+42, OBJ_TEXTURE_SIZE);
    obj.obj_type = int(getFloatFromTexture(obj_tex, 46*id+44, OBJ_TEXTURE_SIZE));
    obj.mat_id = int(getFloatFromTexture(obj_tex, 46*id+45, OBJ_TEXTURE_SIZE));
    return obj;
}

void bboxInterval(in Ray ray, in AABB bbox, out float start, out float end) {
    vec3 inv_direction = 1/ray.rd;

    vec3 lower_ts = (bbox.lower_bound - ray.ro) * inv_direction; 
    vec3 upper_ts = (bbox.upper_bound - ray.ro) * inv_direction;

    if (ray.rd.x < 0) {
        float temp = lower_ts.x;
        lower_ts.x = upper_ts.x;
        upper_ts.x = temp;
    }

    if (ray.rd.y < 0) {
        float temp = lower_ts.y;
        lower_ts.y = upper_ts.y;
        upper_ts.y = temp;
    }

    if (ray.rd.z < 0) {
        float temp = lower_ts.z;
        lower_ts.z = upper_ts.z;
        upper_ts.z = temp;
    }


    start = max(max(lower_ts.x, max(lower_ts.y, lower_ts.z)), 0.0f);
    end = min(upper_ts.x, min(upper_ts.y, upper_ts.z));
}

bool isIntersect(in Ray ray,in AABB bbox) {

    float lowert;
    float uppert;
    bboxInterval(ray, bbox, lowert, uppert);
    return uppert >= 0 && uppert >= lowert;
}

Intersection traverseTriangle(in Ray ray, in vec3 v0, in vec3 v1, in vec3 v2) {
    Intersection result;
    result.intersects = false;

    vec3 v1v0 = v1 - v0;
    vec3 v2v0 = v2 - v0;
    vec3 rov0 = ray.ro - v0;
    vec3  n = cross( v1v0, v2v0 );
    vec3  q = cross( rov0, ray.rd );
    float d = 1.0/dot( ray.rd, n );
    float u = d*dot( -q, v2v0 );
    float v = d*dot(  q, v1v0 );
    float t = d*dot( -n, rov0 );
    if( u<0.0 || u>1.0 || v<0.0 || (u+v)>1.0 ) t = -1.0;
    
    if (t < EPSILON) return result;
    result.intersects = true;
    result.t = t;
    result.position = result.t * ray.rd + ray.ro;
    result.normal = normalize(n);
    return result;
}

vec3 toWorld(in vec3 a, in vec3 N){
    vec3 B, C;
    if (abs(N.x) > abs(N.y)){
        float invLen = 1.0f / sqrt(N.x * N.x + N.z * N.z);
        C = vec3(N.z * invLen, 0.0f, -N.x *invLen);
    } else {
        float invLen = 1.0f / sqrt(N.y * N.y + N.z * N.z);
        C = vec3(0.0f, N.z * invLen, -N.y *invLen);
    }
    B = cross(C, N);
    return a.x * B + a.y * C + a.z * N;
}

Intersection bvhMeshTraverse(in Ray ray, int mesh_bvh_id, int vert_offset) {
    Intersection result;
    result.intersects = false;
    result.t = FLT_MAX;

    int previous = _top;
    stackPush(mesh_bvh_id);
    while(previous < _top) {
        int id;
        stackPop(id);

        if (id < 0) {
            continue;
        } 
        
        LocalBVHMeshNode localNode;
        
        if (id < 1024) {
            localNode = getLocalBVHMeshNodeFromUniform(id);
        } else {
            localNode = getLocalBVHMeshNodeFromTexture(id-1024);
        }

        AABB bbox = localNode.bbox;

        float lower_t = 0;
        float upper_t = 0;
        bboxInterval(ray, localNode.bbox, lower_t, upper_t);
        if (upper_t < 0 || lower_t > upper_t || lower_t > result.t) {
            continue;
        }

        int oid = localNode.obj_ids[0];
        if (oid >= 0) {
            
            vec3 v1, v2, v3;
            getTriangle(oid, vert_offset, v1, v2, v3);

            Intersection temp = traverseTriangle(ray, v1, v2, v3);
            if (temp.intersects && temp.t < result.t) {
                result = temp;
            }
            
        }
        stackPush(localNode.left_id);
        stackPush(localNode.right_id);
        
    }
    return result;
}

void sampleBVHMesh(int mesh_bvh_id, int vert_offset, 
     in mat4 t_matrix, float area, float sample, out vec3 sampleCoord, out vec3 sampleNormal) {

    LocalBVHMeshNode localNode;
        
    if (mesh_bvh_id < 1024) {
        localNode = getLocalBVHMeshNodeFromUniform(mesh_bvh_id);
    } else {
        localNode = getLocalBVHMeshNodeFromTexture(mesh_bvh_id-1024);
    }

    if (localNode.left_id < 0) {
        int oid = localNode.obj_ids[0];
        if (oid >= 0) {

            vec3 v1, v2, v3;
            getTriangle(oid, vert_offset, v1, v2, v3);

            v1 = ptrans(t_matrix,v1);
            v2 = ptrans(t_matrix,v2);
            v3 = ptrans(t_matrix,v3);
            
            vec3 n = cross(v2 - v1, v3 - v1);
            float tarea = length(n) * 0.5f;   
            // found!
            float x = sqrt(get_random_float());
            float y = get_random_float();
            sampleCoord = v1 * (1.0f - x) + v2 * (x * (1.0f - y)) + v3 * (x * y);
            sampleNormal = n;
            return;
        }
    }


    float prev_sample = sqrt(get_random_float())*area;
    int left_id = localNode.left_id;
    int right_id = localNode.right_id;

    for (int i = 0; i < 20; ++i) {
        
        LocalBVHMeshNode leftNode;
        if (left_id < 1024) {
            leftNode = getLocalBVHMeshNodeFromUniform(left_id);
        } else {
            leftNode = getLocalBVHMeshNodeFromTexture(left_id-1024);
        }

        LocalBVHMeshNode rightNode;
        if (right_id < 1024) {
            rightNode = getLocalBVHMeshNodeFromUniform(right_id);
        } else {
            rightNode = getLocalBVHMeshNodeFromTexture(right_id-1024);
        }

        if (leftNode.area >= prev_sample && leftNode.left_id >= 0) {
            prev_sample = prev_sample;
            left_id = leftNode.left_id;
            right_id = leftNode.right_id;
        } else if (leftNode.area >= prev_sample && leftNode.left_id < 0) {
            int oid = leftNode.obj_ids[0];
            if (oid >= 0) {
            
                vec3 v1, v2, v3;
                getTriangle(oid, vert_offset, v1, v2, v3);

                v1 = ptrans(t_matrix,v1);
                v2 = ptrans(t_matrix,v2);
                v3 = ptrans(t_matrix,v3);

                vec3 n = cross(v2 - v1, v3 - v1);
                float tarea = length(n) * 0.5f;   
                // found!
                float x = sqrt(get_random_float());
                float y = get_random_float();
                sampleCoord = v1 * (1.0f - x) + v2 * (x * (1.0f - y)) + v3 * (x * y);
                sampleNormal = n;

                return;
            }
        } else if (leftNode.area < prev_sample && rightNode.left_id < 0) {
            int oid = rightNode.obj_ids[0];
            if (oid >= 0) {
            
                vec3 v1, v2, v3;
                getTriangle(oid, vert_offset, v1, v2, v3);

                v1 = ptrans(t_matrix,v1);
                v2 = ptrans(t_matrix,v2);
                v3 = ptrans(t_matrix,v3);

                vec3 n = cross(v2 - v1, v3 - v1);
                float tarea = length(n) * 0.5f;   
                // found!
                float x = sqrt(get_random_float());
                float y = get_random_float();
                sampleCoord = v1 * (1.0f - x) + v2 * (x * (1.0f - y)) + v3 * (x * y);
                sampleNormal = n;
                return;
            }
        } else {
            prev_sample = prev_sample - leftNode.area;
            left_id = rightNode.left_id;
            right_id = rightNode.right_id;
        }
    }
}

// CSG parse module
float noise(in vec3 t) {
    int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
	float rx0, rx1, ry0, ry1, rz0, rz1, sy, sz, a, b, c, d, temp, u, v;
    vec3 q;
	int i, j;

	setup(0, bx0,bx1, rx0,rx1);
	setup(1, by0,by1, ry0,ry1);
	setup(2, bz0,bz1, rz0,rz1);


    
	i = int(perlin_node[bx0].p);
	j = int(perlin_node[bx1].p);

	b00 = int(perlin_node[i + by0].p);
	b10 = int(perlin_node[j + by0].p);
	b01 = int(perlin_node[i + by1].p);
	b11 = int(perlin_node[j + by1].p);

	temp = s_curve(rx0);
	sy = s_curve(ry0);
	sz = s_curve(rz0);

    #define at3(rx,ry,rz) ( rx * q[0] + ry * q[1] + rz * q[2] )


	q = perlin_node[b00 + bz0].g ; u = at3(rx0,ry0,rz0);
	q = perlin_node[b10 + bz0].g ; v = at3(rx1,ry0,rz0);
	a = lerp(temp, u, v);

	q = perlin_node[b01 + bz0].g ; u = at3(rx0,ry1,rz0);
	q = perlin_node[b11 + bz0].g ; v = at3(rx1,ry1,rz0);
	b = lerp(temp, u, v);

	c = lerp(sy, a, b);

	q = perlin_node[b00 + bz1].g ; u = at3(rx0,ry0,rz1);
	q = perlin_node[b10 + bz1].g ; v = at3(rx1,ry0,rz1);
	a = lerp(temp, u, v);

	q = perlin_node[b01 + bz1].g ; u = at3(rx0,ry1,rz1);
	q = perlin_node[b11 + bz1].g ; v = at3(rx1,ry1,rz1);
	b = lerp(temp, u, v);

	d = lerp(sy, a, b);

	return lerp(sz, c, d);    
}

float turbulence(in vec3 t) {
    float size = 32;
    float value = 0.0; 
    float initialSize = size;

    while(size >= 1)
    {
        vec3 temp = vec3(t.x / size, t.y / size, t.z / size);
        value += noise(temp) * size;
        size /= 2.0;
    }

    return value/initialSize; 
}

MaterialInfo getMaterialInfo(in Object obj, in vec3 t) {
    MaterialInfo info;

    if (obj.mat_id < 0) return info;
    MaterialNode node = mat_node[obj.mat_id];
    info.m_type = int(node.mat_type);
    switch(int(node.mat_type)) {
        case 0: {
            // DIFFUSE
            info.kd = vec3(node.mat_data_1.xy, node.mat_data_2.x);
            info.ks = vec3(node.mat_data_2.y, node.mat_data_3.xy);
            info.ior = node.mat_data_4.x;
            info.shininess = node.mat_data_4.y;
            info.isEmission = false;
            break;
        }
        // case 1: {
        //     info.kd = vec3(node.mat_data_1.xy, node.mat_data_2.x);
        //     info.ks = vec3(node.mat_data_2.y, node.mat_data_3.xy);
        //     info.ior = node.mat_data_4.x;
        //     info.shininess = node.mat_data_4.y;
        //     info.isEmission = false;
        //     break;
        // }
        case 2: {
            AABB bbox = obj.bbox;
            vec3 size = max(bbox.upper_bound - bbox.lower_bound, EPSILON);
            vec3 local_coord = t - bbox.lower_bound;
            vec3 periods = vec3(node.mat_data_1.xy, node.mat_data_2.x);
            float turb_power = node.mat_data_3.y;
            float seed = dot(local_coord, vec3(periods.x/size.x, periods.y/size.y, periods.z/size.z)) + 
                turb_power * (turbulence(local_coord));
            info.kd = vec3(abs(sin(seed*M_PI))); 
            info.ks = vec3(abs(sin(seed*M_PI)));
            info.ior = node.mat_data_2.y;
            info.shininess = node.mat_data_3.x;
            info.isEmission = false;
            break;
        }
        // case 3: {
        //     info.kd = vec3(node.mat_data_1.xy, node.mat_data_2.x);
        //     info.ks = vec3(node.mat_data_2.y, node.mat_data_3.xy);
        //     info.ior = node.mat_data_4.x;
        //     info.shininess = node.mat_data_4.y;
        //     info.isEmission = false;
        //     break;
        // }
        // Microfacet
        case 4: {
            info.ks = vec3(node.mat_data_1.xy, node.mat_data_2.x);
            info.shininess = node.mat_data_2.y;
            info.isEmission = false;
            break;
        }
        // Light
        case 5: {
            info.kd = vec3(node.mat_data_1.xy, node.mat_data_2.x);
            info.isEmission = true;
            break;
        }
    }
    return info;
}

void bump_map_perturb(in Object obj, inout Intersection result) {
    // calculate texture coords
    vec3 center = (obj.bbox.upper_bound + obj.bbox.lower_bound)*0.5;
    vec3 x_axis = normalize(cross(result.normal, vec3(0,1,0)));
    float dy = (result.position.y - center.y)/(obj.bbox.upper_bound.y - obj.bbox.lower_bound.y);
    float dx = dot(x_axis, result.position - center)/abs(dot(x_axis, obj.bbox.upper_bound - obj.bbox.lower_bound));
    dy = dy + 0.5f;
    dx = dx + 0.5f;
    // vec3 b = texture(bump_map_perturb, vec2(dx, dy)).rgb;
    // b = normalize(b*2.0f - 1.0f);

    float delta = 0.015625;
    vec3 bxplus = texture(bump_map_tex, vec2(dx+delta, dy)).rgb;
    bxplus = normalize(bxplus*2.0f - 1.0f);
    
    vec3 bxminus = texture(bump_map_tex, vec2(dx-delta, dy)).rgb;
    bxminus = normalize(bxminus*2.0f - 1.0f);
    
    vec3 byplus = texture(bump_map_tex, vec2(dx, dy+delta)).rgb;
    byplus = normalize(byplus*2.0f - 1.0f);
    
    vec3 byminus = texture(bump_map_tex, vec2(dx, dy-delta)).rgb;
    byminus = normalize(byminus*2.0f - 1.0f);
    
    float bu = (bxplus - bxminus).x*32;
    float bv = (byplus - byminus).y*32;

    vec3 D = bu*vec3(1,0,0) - bv*vec3(0,1,0);
    result.normal = normalize(result.normal + D);
}

Intersection objTraverse(int oid, in Object obj, in Ray ray) {
    Intersection result;
    result.intersects = false;

    vec3 normalizedRd = normalize(ray.rd);
    float rdlength = length(ray.rd);

    switch(obj.obj_type) {
        case 0: {
            
            vec3 ce = vec3(obj.obj_data_1, obj.obj_data_2.x);
            float ra = obj.obj_data_2.y;
            vec3 oc = ray.ro - ce;
            float b = dot( oc, normalizedRd );
            float c = dot( oc, oc ) - ra*ra;
            float h = b*b - c;
            if( h<0.0 ) return result; // no intersection
            h = sqrt( h );
            if (-b-h > 0.0f) {
                result.intersects = true;
                result.t = (-b-h)/rdlength;
                result.position = ray.ro + result.t * ray.rd;
                result.normal = normalize(result.position - ce);
            } else if (-b+h > 0.0f) {
                result.intersects = true;
                result.t = (-b+h)/rdlength;
                result.position = ray.ro + result.t * ray.rd;
                result.normal = normalize(result.position - ce);
            }
            result.m = getMaterialInfo(obj, result.position);
            break;
        }
        case 1: {
            vec3 pos = vec3(obj.obj_data_1, obj.obj_data_2.x);
            float boxSize = obj.obj_data_2.y;
            vec3 m = 1.0/normalizedRd; // can precompute if traversing a set of aligned boxes
            vec3 n = m*(ray.ro - pos);   // can precompute if traversing a set of aligned boxes
            vec3 k = abs(m)*boxSize;
            vec3 t1 = -n - k;
            vec3 t2 = -n + k;
            float tN = max( max( t1.x, t1.y ), t1.z );
            float tF = min( min( t2.x, t2.y ), t2.z );
            if( tN>tF || tF<0.0) return result; // no intersection
            result.intersects = true;
            result.normal = -sign(normalizedRd)*step(t1.yzx,t1.xyz)*step(t1.zxy,t1.xyz);
            if (tN > 0.0f) {
                result.t = tN/rdlength;
                result.position = ray.rd * result.t + ray.ro;
            } else {
                result.t = tF/rdlength;
                result.position = ray.rd * result.t + ray.ro;
            }
            result.m = getMaterialInfo(obj, result.position);
            break;
        }
        case 2: {
            // bounding box
            vec3 ro = ray.ro;
            vec3 rd = normalizedRd;
            vec3 size = vec3(obj.obj_data_1, obj.obj_data_2.x);
            float rad = obj.obj_data_2.y;
            vec3 m = 1.0/rd;
            vec3 n = m*ro;
            vec3 k = abs(m)*(size+rad);
            vec3 t1 = -n - k;
            vec3 t2 = -n + k;
            float tN = max( max( t1.x, t1.y ), t1.z );
            float tF = min( min( t2.x, t2.y ), t2.z );
            if( tN>tF || tF<0.0) return result;
            float t = tN;

            // convert to first octant
            vec3 pos = ro+t*rd;
            vec3 s = sign(pos);
            ro  *= s;
            rd  *= s;
            pos *= s;
                
            // faces
            pos -= size;
            pos = max( pos.xyz, pos.yzx );
            if( min(min(pos.x,pos.y),pos.z) < 0.0 ) {
                if (t < 0) return result;
                result.intersects = true;
                result.t = t/rdlength;
                result.position = ray.rd * result.t + ray.ro;
                result.normal = sign(result.position)*normalize(
                    max(abs(result.position)- size,0.0f));
            } else {

                // some precomputation
                vec3 oc = ro - size;
                vec3 dd = rd*rd;
                vec3 oo = oc*oc;
                vec3 od = oc*rd;
                float ra2 = rad*rad;

                t = 1e20;        

                // corner
                {
                float b = od.x + od.y + od.z;
                float c = oo.x + oo.y + oo.z - ra2;
                float h = b*b - c;
                if( h>0.0 ) t = -b-sqrt(h);
                }
                // edge X
                {
                float a = dd.y + dd.z;
                float b = od.y + od.z;
                float c = oo.y + oo.z - ra2;
                float h = b*b - a*c;
                if( h>0.0 )
                {
                    h = (-b-sqrt(h))/a;
                    if( h>0.0 && h<t && abs(ro.x+rd.x*h)<size.x ) t = h;
                }
                }
                // edge Y
                {
                float a = dd.z + dd.x;
                float b = od.z + od.x;
                float c = oo.z + oo.x - ra2;
                float h = b*b - a*c;
                if( h>0.0 )
                {
                    h = (-b-sqrt(h))/a;
                    if( h>0.0 && h<t && abs(ro.y+rd.y*h)<size.y ) t = h;
                }
                }
                // edge Z
                {
                float a = dd.x + dd.y;
                float b = od.x + od.y;
                float c = oo.x + oo.y - ra2;
                float h = b*b - a*c;
                if( h>0.0 )
                {
                    h = (-b-sqrt(h))/a;
                    if( h>0.0 && h<t && abs(ro.z+rd.z*h)<size.z ) t = h;
                }
                }

                if( t>1e19 || t < 0) return result;
                
                result.intersects = true;
                result.t = t/rdlength;
                result.position = ray.rd * result.t + ray.ro;
                result.normal = sign(result.position)*normalize(
                    max(abs(result.position)- size,0));
            }
            result.m = getMaterialInfo(obj, result.position);
            break;
        }
        case 3: {
            float t_min = FLT_MIN;
            float t_max = FLT_MAX;
            int min_code = -1;
            int max_code = -1;

            vec3 ro = ray.ro;
            vec3 rd = normalizedRd;
            float height = obj.obj_data_1.y;
            float radius = obj.obj_data_1.x;

            // side surface
            {    
                float A = (rd.x * rd.x) + (rd.z * rd.z);
                float B = 2*(rd.x*(ro.x) + rd.z*(ro.z));
                float C = (ro.x) * (ro.x) + (ro.z) * (ro.z) - (radius*radius);
                float H = B*B - 4*A*C;

                if (A > 0.0001 && H > 0) {
                    float temp = abs(sqrt(H)/(2*A));
                    float root_min = -B/(2*A) - temp;
                    float root_max = -B/(2*A) + temp;
                    vec3 pos = ro + root_min * rd;
                    if (pos.y >= -height && pos.y <= height) {
                        t_min = root_min;
                        min_code = 0;
                    }
                        
                    pos = ro + root_max * rd;
                    if (pos.y >= -height && pos.y <= height) {
                        t_max = root_max;
                        max_code = 0;
                    }
                }
            }

            // upper and lower surface
            {
                if (rd.y != 0) {
                    float t1;
                    float t2;
                    int i1;
                    int i2;

                    if (rd.y > 0) {
                        t1 = (-height - ro.y) / rd.y;
                        t2 = (height - ro.y) / rd.y;
                        i1 = 1;
                        i2 = 2;
                    } else {
                        t1 = (height - ro.y) / rd.y;
                        t2 = (-height - ro.y) / rd.y;
                        i1 = 2;
                        i2 = 1;
                    }

                    vec3 p = t1 * rd + ro;
                    if (p.x*p.x + p.z*p.z <= radius*radius && t1 > t_min) {
                        t_min = t1;
                        min_code = i1;
                    }
                    p = t2 * rd + ro;

                    if (p.x*p.x + p.z*p.z <= radius*radius && t2 < t_max) {
                        t_max = t2;
                        max_code = i2;
                    }
                }
            }

            if (t_max < 0 || t_min > t_max || t_max > 1e19) return result;

            result.intersects = true;
            result.t = t_min;
            int code = min_code;
            if (t_min < 0) {
                result.t = t_max;
                code = max_code;
            }
            result.t = result.t/rdlength;
            result.position = ray.ro + result.t*ray.rd;
            
            if (code == 0) {
                result.normal = normalize(vec3(result.position.x, 0.0f, result.position.z));
            } else if (code == 1) {
                result.normal = vec3(0.0f, -1.0f, 0.0f);
            } else {
                result.normal = vec3(0.0f, 1.0f, 0.0f);
            }
            result.m = getMaterialInfo(obj, result.position);
            break;
        }
        case 4: {
            vec3 ro = ray.ro;
            vec3 rd = normalizedRd;
            vec2 tor = obj.obj_data_1;
            float po = 1.0;
            float Ra2 = tor.x*tor.x;
            float ra2 = tor.y*tor.y;
            float m = dot(ro,ro);
            float n = dot(ro,rd);
            float k = (m + Ra2 - ra2)/2.0;
            float k3 = n;
            float k2 = n*n - Ra2*dot(rd.xy,rd.xy) + k;
            float k1 = n*k - Ra2*dot(rd.xy,ro.xy);
            float k0 = k*k - Ra2*dot(ro.xy,ro.xy);
            
            if( abs(k3*(k3*k3-k2)+k1) < 0.05 )
            {
                po = -1.0;
                float tmp=k1; k1=k3; k3=tmp;
                k0 = 1.0/k0;
                k1 = k1*k0;
                k2 = k2*k0;
                k3 = k3*k0;
            }
            
            float c2 = k2*2.0 - 3.0*k3*k3;
            float c1 = k3*(k3*k3-k2)+k1;
            float c0 = k3*(k3*(c2+2.0*k2)-8.0*k1)+4.0*k0;
            c2 /= 3.0;
            c1 *= 2.0;
            c0 /= 3.0;
            float Q = c2*c2 + c0;
            float R = c2*c2*c2 - 3.0*c2*c0 + c1*c1;
            float h = R*R - Q*Q*Q;
            
            if( h>=0.0 )  
            {
                h = sqrt(h);
                float v = sign(R+h)*pow(abs(R+h),1.0/3.0); // cube root
                float u = sign(R-h)*pow(abs(R-h),1.0/3.0); // cube root
                vec2 s = vec2( (v+u)+4.0*c2, (v-u)*sqrt(3.0));
                float y = sqrt(0.5*(length(s)+s.x));
                float x = 0.5*s.y/y;
                float r = 2.0*c1/(x*x+y*y);
                float t1 =  x - r - k3; t1 = (po<0.0)?2.0/t1:t1;
                float t2 = -x - r - k3; t2 = (po<0.0)?2.0/t2:t2;
                float t = 1e20;
                if( t1>0.0 ) t=t1;
                if( t2>0.0 ) t=min(t,t2);
                if (t < 1e19 && t > 0) {
                    //found !
                    result.intersects = true;
                    result.t = t/rdlength;
                    result.position = ray.rd * result.t + ray.ro;
                    result.normal = normalize( result.position*(
                        dot(result.position,result.position)-
                        tor.y*tor.y - tor.x*tor.x*vec3(1.0,1.0,-1.0)));
                }
            } else {
            
                float sQ = sqrt(Q);
                float w = sQ*cos( acos(-R/(sQ*Q)) / 3.0 );
                float d2 = -(w+c2); if( d2<0.0 ) return result;
                float d1 = sqrt(d2);
                float h1 = sqrt(w - 2.0*c2 + c1/d1);
                float h2 = sqrt(w - 2.0*c2 - c1/d1);
                float t1 = -d1 - h1 - k3; t1 = (po<0.0)?2.0/t1:t1;
                float t2 = -d1 + h1 - k3; t2 = (po<0.0)?2.0/t2:t2;
                float t3 =  d1 - h2 - k3; t3 = (po<0.0)?2.0/t3:t3;
                float t4 =  d1 + h2 - k3; t4 = (po<0.0)?2.0/t4:t4;
                float t = 1e20;
                if( t1>0.0 ) t=t1;
                if( t2>0.0 ) t=min(t,t2);
                if( t3>0.0 ) t=min(t,t3);
                if( t4>0.0 ) t=min(t,t4);
                //found !
                if (t < 1e19 && t > 0) {
                    result.intersects = true;
                    result.t = t/rdlength;
                    result.position = ray.rd * result.t + ray.ro;
                    result.normal = normalize( result.position*(
                        dot(result.position,result.position)-
                        tor.y*tor.y - tor.x*tor.x*vec3(1.0,1.0,-1.0)));
                }
            }
            result.m = getMaterialInfo(obj, result.position);
            break;
        }
        case 5: {
            int bvh_mesh_id = int(obj.obj_data_1.x);
            int vertex_offset = int(obj.obj_data_1.y);
            result = bvhMeshTraverse(ray, bvh_mesh_id, vertex_offset);
            result.m = getMaterialInfo(obj, result.position);

            if (oid == 0 || oid == 3) {
                bump_map_perturb(obj, result);
            }
            break;
        }
        case 6: {
            // RAY_MARCHING CODE 


            // AABB bbox = obj.bbox;

            // float current,end;
            // bboxInterval(ray, bbox, current, end);

            // if (end <= 0 || current >= end) return result;

            // float delta = 2;
            // current = max(current, delta);
            // end = max(end, delta);
            // vec3 t = ray.ro + current * ray.rd;

            // // Object csg_cache[CSG_STACK_SIZE];
            // // int left_cache[CSG_STACK_SIZE];
            // // int right_cache[CSG_STACK_SIZE];

            // int maxStep = 100;
            // int currentStep = 0;
            // while(current < end+ZERO && currentStep < maxStep) {
            //     // loadObjects(obj);
            //     float dst;
            //     result.m = sdfWithMatInfo(oid,t,dst,true);
            //     if (dst < EPSILON) {
                    
            //         result.normal = estimateNormal(oid, t);
            //         result.position = t;
            //         result.t = current;
            //         result.intersects = true;
            //         break;
            //     }

            //     current += dst;
            //     t = ray.ro + current * ray.rd;
            //     ++currentStep;
            // }     


            // if (result.intersects) {
            //     result.intersects = false;
            //     vec3 ce = vec3(0, 0,0);
            //     float ra = 100;
            //     vec3 oc = ray.ro - ce;
            //     float b = dot( oc, normalizedRd );
            //     float c = dot( oc, oc ) - ra*ra;
            //     float h = b*b - c;
            //     if( h<0.0 ) return result; // no intersection
            //     h = sqrt( h );
            //     if (-b-h > 0.0f) {
            //         result.intersects = true;
            //         result.t = (-b-h)/rdlength;
            //         result.position = ray.ro + result.t * ray.rd;
            //         result.normal = normalize(result.position - ce);
            //     } else if (-b+h > 0.0f) {
            //         result.intersects = true;
            //         result.t = (-b+h)/rdlength;
            //         result.position = ray.ro + result.t * ray.rd;
            //         result.normal = normalize(result.position - ce);
            //     }
            // }
            
            break;
        }
        default: {
            break;
        }
    }
    return result;
}

Intersection bvhTraverse(in Ray ray) {
    Intersection result;
    result.intersects = false;
    result.t = FLT_MAX;

    stackPush(0);

    while(!is_stack_empty()) {
        int id;
        stackPop(id);

        if (id < 0) {
            continue;
        } 
        
        LocalBVHNode localNode;
        
        if (id < 1024) {
            localNode = getLocalBVHNodeFromUniform(id);
        } else {
            localNode = getLocalBVHNodeFromTexture(id-1024);
        }


        float lower_t = 0;
        float upper_t = 0;
        bboxInterval(ray, localNode.bbox, lower_t, upper_t);
        if (upper_t < 0 || lower_t > upper_t || lower_t > result.t) {
            continue;
        }

        for (int i = 0; i < 1; ++i) {
            int oid = localNode.obj_ids[i];
            if (oid >= 0) {
                
                Object obj = getObject(oid);
                
                Ray transformed_ray;
                transformed_ray.ro = ptrans(obj.inv_t_matrix, ray.ro);
                transformed_ray.rd = vtrans(obj.inv_t_matrix, ray.rd);

                Intersection temp = objTraverse(oid, obj, transformed_ray);

                if (temp.intersects && temp.t < result.t) {
                    result = temp;
                    result.position = ptrans(obj.t_matrix, result.position);
                    result.normal = vtrans(transpose(obj.inv_t_matrix), temp.normal);
                    result.id = oid;
                } 
            }
        }

        stackPush(localNode.left_id);
        stackPush(localNode.right_id);
    }
    return result;
}

void sampleLight(out vec3 sampleCoord, out vec3 sampleNormal, 
    out vec3 sampleEmit, out float pdf) {
    float p = get_random_float() * total_light_area;
    float emit_area_sum = 0;
    for(int i = ZERO; i < num_of_lights; ++i) {
        int loid = int(light_node[i].oid_and_area.x);
        
        emit_area_sum += light_node[i].oid_and_area.y;
        if (p <= emit_area_sum) {
            Object obj = getObject(loid);

            sampleEmit = vec3(mat_node[obj.mat_id].mat_data_1.xy, 
                mat_node[obj.mat_id].mat_data_2.x);
            // // Sample point and pdf from object
            float area = light_node[i].oid_and_area.y;
            switch(obj.obj_type) {
                case 0: {
                    vec3 pos = vec3(obj.obj_data_1.xy, obj.obj_data_2.x);
                    float radius = obj.obj_data_2.y;

                    float theta = 2.0f * M_PI * get_random_float();
                    float phi = M_PI * get_random_float();

                    vec3 dir = vec3(cos(phi), sin(phi)*cos(theta), sin(phi)*cos(theta));
                    sampleCoord = pos + radius * dir;
                    sampleNormal = dir;
                    break;
                }
                case 1: {
                    vec3 pos = vec3(obj.obj_data_1.xy, obj.obj_data_2.x);
                    float size = obj.obj_data_2.y;
                    
                    int f = int(get_random_float() * 6);
                    float x = get_random_float();
                    float y = get_random_float();

                    int c = f%3;

                    float coords[3];
                    float normal[3];
                    coords[c] = (2*float((f/3)&1) - 1.0f) * size;
                    coords[(c+1)%3] = (2*x - 1)*size;
                    coords[(c+2)%3] = (2*y - 1)*size;
                    normal[c] = 2*float((f/3)&1) - 1.0f;
                    normal[(c+1)%3] = 0;
                    normal[(c+2)%3] = 0;
                    sampleCoord = vec3(coords[0], coords[1], coords[2]);
                    sampleNormal = vec3(normal[0], normal[1], normal[2]);
                    break;
                }
                case 2: {
                    vec3 size = vec3(obj.obj_data_1.xy, obj.obj_data_2.x);
                    float radius = obj.obj_data_2.y;
                      

                    break;
                }
                case 3: {
                    float height = obj.obj_data_1.x;
                    float radius = obj.obj_data_1.y;

                    float temp = get_random_float()*(4*radius*height + 2*radius*radius);
                    int code = int(dot(step(vec2(4*radius*height, 
                        4*radius*height+radius*radius), vec2(temp)), vec2(1)));
                    
                    if (code == 0) {
                        float h = get_random_float()*height;
                        float theta = 2*M_PI*get_random_float();
                        sampleCoord = vec3(radius*cos(theta),h,radius*sin(theta));   
                        sampleNormal = vec3(radius*cos(theta),0,radius*sin(theta));
                    } else if (code == 1) {
                        float r = get_random_float()*radius;
                        float theta = 2*M_PI*get_random_float();
                        sampleCoord = vec3(r*cos(theta),height,r*sin(theta));   
                        sampleNormal = vec3(0,1,0);
                    } else {
                        float r = get_random_float()*radius;
                        float theta = 2*M_PI*get_random_float();
                        sampleCoord = vec3(r*cos(theta),-height,r*sin(theta));   
                        sampleNormal = vec3(0,-1,0);
                    }
                    break;
                }
                case 4: {
                    float b = obj.obj_data_1.x;
                    float a = obj.obj_data_1.y;
                    
                    float theta = 2*M_PI*get_random_float();
                    float phi = 2*M_PI*get_random_float();
                    sampleCoord = vec3((a*cos(theta)+b)*cos(phi),
                        (a*cos(theta)+b)*sin(phi),a*sin(theta));
                    sampleNormal = normalize(sampleCoord*(
                        dot(sampleCoord,sampleCoord)-
                        a*a - b*b*vec3(1.0,1.0,-1.0)));
                    break;
                }
                case 5: {
                    int bvh_id = int(obj.obj_data_1.x);
                    int voffset = int(obj.obj_data_1.y);
                    sampleBVHMesh(bvh_id, voffset, obj.t_matrix, area, 
                        p - (emit_area_sum - area), sampleCoord, sampleNormal);
                    break;
                }        
            }
            
            pdf = 1.0f/area;
            break;
        }
    }
}

vec3 evalMaterial(in MaterialInfo m, in vec3 wi, in vec3 wo, in vec3 N) {
    switch(m.m_type) {
        case 0:
        case 1:
        case 2:
        case 3: {
            float cosalpha = abs(dot(N, wo));
            return (cosalpha > 0.0f)? m.kd/M_PI:vec3(0.0);
            break;
        }
        case 4: {
            float cosalpha_i = abs(dot(N, wi));
            float cosalpha = abs(dot(N, wo));
            if (dot(N, wo) > EPSILON && dot(N, wi) > EPSILON) {
                vec3 h = normalize(wi + wo);

                float cosalpha_h = abs(dot(N, h));

                float D = (m.shininess*m.shininess)/(M_PI*pow(
                    cosalpha_h*cosalpha_h*(m.shininess*m.shininess - 1), 2));

                float k_direct = pow(m.shininess+1,2)/8;
                float k_ibl = pow(m.shininess,2)/2;
                float k = k_direct;
                float G = ((cosalpha_i/(cosalpha_i*(1 - k) + k)) *
                    (cosalpha/(cosalpha*(1 - k) + k)));
                vec3 F = m.ks + (vec3(1.0f) - m.ks)*(1 - pow(dot(h, wi), 5));
                return ((G*F)/max(4*cosalpha_i*cosalpha, EPSILON));
            }
            break;
        }
    }
    return vec3(0);
}

vec3 sampleMaterial(in MaterialInfo m, in vec3 wi, vec3 N) {
    switch(m.m_type) {
        case 0:
        case 1:
        case 2:
        case 3: {
            float x_1 = get_random_float();
            float x_2 = get_random_float();
            float z = abs(1.0f - 2.0f * x_1);
            float r = sqrt(1.0f - z * z);
            float phi = 2 * M_PI * x_2;
            vec3 localRay = vec3(r*cos(phi), r*sin(phi), z);
            return toWorld(localRay, N);
            break;
        }
        case 4: {
            float x_1 = get_random_float();
            float x_2 = get_random_float();
            float theta_m = acos(sqrt((1 - x_1)/(x_1*(m.shininess*m.shininess - 1)+1)));
            float phi = 2*M_PI*x_2;
            vec3 h = vec3(sin(theta_m)*sin(phi), sin(theta_m)*cos(phi),cos(theta_m));
            vec3 world_h = normalize(toWorld(h, N));
            return 2*dot(world_h, wi)*world_h - wi;
            break;
        }
    }    
    return vec3(0);
}

float pdfMaterial(in MaterialInfo m, in vec3 wi, in vec3 wo, in vec3 N) {
    switch(m.m_type) {
        case 0:
        case 1:
        case 2:
        case 3: {
            return (dot(wo, N) > 0.0f)? 0.5f/M_PI:EPSILON;
            break;
        }
        case 4: {
            vec3 h = normalize(wo+wi);
            return abs(dot(h, N))/max(4*abs(dot(h, wo)), EPSILON);
            break;
        }
    }    
    return 0.0f;
}

vec3 castRay(in Ray primaryRay, out Intersection sample_point) {
    vec3 result = vec3(0.0f);

    Ray ray = primaryRay;
    vec3 factor = vec3(1.0f);
    int depth = 0;

    for (int i = 0; i < 7; ++i) {

        // if (depth > 0) {
        //     continue;
        // }
        
        Intersection current = bvhTraverse(ray);
        if (i == 0) {
            sample_point = current;
        }

        if (current.intersects) {
            if (current.m.isEmission) {
                result = min(result + factor*min(current.m.kd, 1.0f), 1.0f);
                break;
            }

            vec3 wo = normalize(-ray.rd);
            // Direct lighting calculation
            float pdf;
            vec3 sampleCoord;
            vec3 sampleNormal;
            vec3 sampleEmit;
            sampleLight(sampleCoord, sampleNormal, sampleEmit, pdf);
            // sampleCoord = vec3(250,548.7,250);
            // sampleEmit = vec3(23.9174, 19.2832,15.5404);
            // sampleNormal = vec3(0, -1, 0);
            
            vec3 dl_vec = sampleCoord - current.position;
            
            vec3 N = normalize(current.normal);
            vec3 ws = normalize(dl_vec);
            float dl_distance = length(dl_vec);
            vec3 NN = normalize(sampleNormal);

            vec3 p = current.position;

            Ray sampledRay = Ray(current.position, ws);
            Intersection dl_intersection = bvhTraverse(sampledRay);

            vec3 l_dir = vec3(0);    
            if (dl_intersection.t > (dl_distance - EPSILON)) {
                l_dir = ((sampleEmit * evalMaterial(current.m,ws, wo, N) * dot(NN, 
                    -ws) * dot(N, ws))/ (dl_distance*dl_distance*pdf));
            }

            // l_dir = ((sampleEmit * vec3(1.0, 1.0, 1.0) * 1.0f)/ (1*1*pdf));
    
            result = min(result + factor*min(l_dir, 1.0f), 1.0f);

            float random = get_random_float();

            if (random > RUSSIAN_ROULETTE) {
                break;
            }

            vec3 wi = normalize(sampleMaterial(current.m, wo, N));
            
            vec3 delta;
            if (dot(current.normal, wi) > 0) {
                delta = current.normal * EPSILON;
            } else {
                delta = -current.normal * EPSILON;
            }
            Ray nextRay = Ray(p+delta,wi);

            ray = nextRay;
            depth = depth + 1;
            factor = (factor * evalMaterial(current.m,wi, wo, N) * 
                dot(N, wi))/(RUSSIAN_ROULETTE * 
                pdfMaterial(current.m,wi, wo, N));
            if (length(factor) < 0.1) break;
        } else {
            break;
        }
    }
    return result;
}

void get_indirect_sample(in Ray ray, in Intersection current, out Sample result) {
    vec3 wo = normalize(-ray.rd);

    // Direct lighting calculation
    float pdf;
    vec3 sampleCoord;
    vec3 sampleNormal;
    vec3 sampleEmit;
    sampleLight(sampleCoord, sampleNormal, sampleEmit, pdf);
    // sampleCoord = vec3(250,548.7,250);
    // sampleEmit = vec3(23.9174, 19.2832,15.5404);
    // sampleNormal = vec3(0, -1, 0);
    
    vec3 dl_vec = sampleCoord - current.position;
    
    vec3 N = normalize(current.normal);
    vec3 ws = normalize(dl_vec);
    float dl_distance = length(dl_vec);
    vec3 NN = normalize(sampleNormal);

    vec3 p = current.position;

    Ray sampledRay = Ray(current.position, ws);
    Intersection dl_intersection = bvhTraverse(sampledRay);

    vec3 l_dir = vec3(0);    
    if (dl_intersection.t > (dl_distance - EPSILON)) {
        l_dir = ((sampleEmit * evalMaterial(current.m,ws, wo, N) * max(dot(NN, 
            -ws), 0.0f) * max(dot(N, ws), 0.0f))/ (dl_distance*dl_distance*pdf));
    }

    result.color = l_dir;

    vec3 wi = normalize(sampleMaterial(current.m, wo, N));
    
    vec3 delta;
    if (dot(current.normal, wi) > 0) {
        delta = current.normal * EPSILON;
    } else {
        delta = -current.normal * EPSILON;
    }

    Ray nextRay = Ray(p+delta,wi);
    
    result.xv = current.position;
    result.nv = current.normal;
    Intersection sample_point;
    result.lo = castRay(nextRay, sample_point);
    result.lo = (result.lo * evalMaterial(current.m,wi, wo, N) * 
                max(dot(N, wi), 0.0f))/pdfMaterial(current.m,wi, wo, N);
   
    // float total_w = length(result.lo) + length(l_dir);
    // if (total_w < 0.001) {
    //     result.oid = -1;
    //     return;
    // }

    // float random = get_random_float();
    // if (random <1) {
    //     result.lo = l_dir;  
    //     result.oid = dl_intersection.id;
    //     result.xs =  dl_intersection.position;
    //     result.ns = dl_intersection.normal;
    // } else {
    //     result.oid = sample_point.id;
    //     result.xs = sample_point.position;
    //     result.ns = sample_point.normal;
    // }
    result.oid = sample_point.id;
    result.xs = sample_point.position;
    result.ns = sample_point.normal;
    return;
}

void main() {
    // float w_size = tan(radians(fov*0.5));
    // float h_ratio = w_size/window_size.y;
    // float w_ratio = w_size/window_size.x;

    // vec3 dir = normalize(vtrans(transpose(viewMatrix), 
    //     vec3((gl_FragCoord.x*2 - window_size.x)*h_ratio,
    //          (gl_FragCoord.y*2 - window_size.y)*w_ratio,
    //          -1.0f)));

    // Ray primaryRay = Ray(eyePos, dir);

    // vec3 color = vec3(0);
    // for (int i = 0; i < SPP; ++i) {
    //     color += castRay(primaryRay);
    // }
    // color = color * (1.0f/SPP);
    // fragColor = vec4(color, 1.0f);

    // if (length(color) < EPSILON) {
    //     fragColor = texture(bg_tex, texCoord);
    // }


    float w_size = tan(radians(fov));
    float h_ratio = w_size/window_size.y;
    float w_ratio = w_size/window_size.x;

    vec3 dir =  normalize(vtrans(transpose(viewMatrix), 
        vec3((gl_FragCoord.x*2 - window_size.x)*h_ratio,
             (gl_FragCoord.y*2 - window_size.y)*w_ratio,
             -1.0f)));
    Ray primaryRay = Ray(eyePos, dir);

    Intersection current = bvhTraverse(primaryRay);

    Sample result;

    result.mid = current.id;
    if (current.intersects && current.m.isEmission) {
        result.color = min(current.m.kd, 1.0f);
        result.oid = -1;
    } else if (!current.intersects) {
        result.oid = -1;
        result.color = vec3(0,0,0);
    } else {
        get_indirect_sample(primaryRay,current,result);
    }
    pack_output(result);
}
