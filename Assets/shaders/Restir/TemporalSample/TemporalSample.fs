#version 330

#define M_PI 3.14159265358979323846
#define EPSILON 0.01
#define MAXIMUM_RESERVIOR_DEPTH 1000

uniform sampler2D isb_tex1;
uniform sampler2D isb_tex2;
uniform sampler2D isb_tex3;
uniform sampler2D isb_tex4;
uniform sampler2D isb_tex5;
uniform sampler2D isb_tex6;

uniform sampler2D trb_tex1;
uniform sampler2D trb_tex2;
uniform sampler2D trb_tex3;
uniform sampler2D trb_tex4;
uniform sampler2D trb_tex5;
uniform sampler2D trb_tex6;
uniform sampler2D trb_tex7;

layout (location = 0) out vec4 outdata1;
layout (location = 1) out vec4 outdata2;
layout (location = 2) out vec4 outdata3;
layout (location = 3) out vec4 outdata4;
layout (location = 4) out vec4 outdata5;
layout (location = 5) out vec4 outdata6;
layout (location = 6) out vec4 outdata7;


in vec2 texCoord;
uniform vec4 seed = vec4(-1,-1,-1,-1);
uniform bool initialized = false;

struct Sample {
    vec3 xv, nv, xs, ns, lo;
    int oid, mid;
};

struct Reservior {
    Sample z;
    float w;
    int M;
    float W;
};

void read_isb(out Sample sample) {
    vec4 data1 = texture(isb_tex1, vec2(texCoord.x, 1 - texCoord.y));
    vec4 data2 = texture(isb_tex2, vec2(texCoord.x, 1 - texCoord.y));
    vec4 data3 = texture(isb_tex3, vec2(texCoord.x, 1 - texCoord.y));
    vec4 data4 = texture(isb_tex4, vec2(texCoord.x, 1 - texCoord.y));
    vec4 data5 = texture(isb_tex5, vec2(texCoord.x, 1 - texCoord.y));
    vec4 data6 = texture(isb_tex6, vec2(texCoord.x, 1 - texCoord.y));

    sample.xv = data1.rgb;
    sample.nv = data2.rgb;
    sample.xs = data3.rgb;
    sample.ns = data4.rgb;
    sample.lo = data5.rgb;
    sample.oid = int(data6.r);
    sample.mid = int(data6.g);
}

void read_trb(out Reservior trb) {
    vec4 data1 = texture(trb_tex1, vec2(texCoord.x, 1 - texCoord.y));
    vec4 data2 = texture(trb_tex2, vec2(texCoord.x, 1 - texCoord.y));
    vec4 data3 = texture(trb_tex3, vec2(texCoord.x, 1 - texCoord.y));
    vec4 data4 = texture(trb_tex4, vec2(texCoord.x, 1 - texCoord.y));
    vec4 data5 = texture(trb_tex5, vec2(texCoord.x, 1 - texCoord.y));
    vec4 data6 = texture(trb_tex6, vec2(texCoord.x, 1 - texCoord.y));
    vec4 data7 = texture(trb_tex7, vec2(texCoord.x, 1 - texCoord.y));

    trb.z.xv = data1.rgb;
    trb.z.nv = data2.rgb;
    trb.z.xs = data3.rgb;
    trb.z.ns = data4.rgb;
    trb.z.lo = data5.rgb;
    trb.z.oid = int(data6.r);
    trb.z.mid = int(data6.g);
    trb.w = int(data6.b);
    trb.M = int(data7.r);
    trb.W = float(data7.g);
}

void pack_output(in Reservior trb) {
    outdata1 = vec4(trb.z.xv.rgb, 1.0f);
    outdata2 = vec4(trb.z.nv.rgb, 1.0f);
    outdata3 = vec4(trb.z.xs.rgb, 1.0f);
    outdata4 = vec4(trb.z.ns.rgb, 1.0f);
    outdata5 = vec4(trb.z.lo.rgb, 1.0f);
    outdata6 = vec4(trb.z.oid,trb.z.mid,trb.w,1.0f);
    outdata7 = vec4(trb.M,trb.W,0,1.0f);
}

float rand_seed_y = 1.0f;

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

void update(inout Reservior R, in Sample S, float w) {
    R.w += w;
    R.M += 1;
    float random = get_random_float();
    if (random <= w/R.w) {
        R.z = S;
    }
}

void main() {
    Sample S;
    read_isb(S);
    Reservior R;

    read_trb(R);

    // if (S.oid < 0) {
    //     pack_output(R);
    //     return;
    // }
    
    float w = length(S.lo);
    if (S.oid < 0) w = 0.00001;
    R.M = min(R.M, MAXIMUM_RESERVIOR_DEPTH-1);
    update(R, S, w);
    R.W = R.w/(R.M*length(R.z.lo));
    pack_output(R);
}