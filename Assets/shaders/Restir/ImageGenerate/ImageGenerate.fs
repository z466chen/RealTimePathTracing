#version 330

#define MAXIMUM_NUMBER 1024

uniform sampler2D emission_tex;
uniform sampler2D color_buffer;
uniform sampler2D number_buffer;

uniform sampler2D srb_tex1;
uniform sampler2D srb_tex2;
uniform sampler2D srb_tex3;
uniform sampler2D srb_tex4;
uniform sampler2D srb_tex5;
uniform sampler2D srb_tex6;
uniform sampler2D srb_tex7;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 M;

in vec2 texCoord;

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

void read_srb(out Reservior srb) {
    vec4 data1 = texture(srb_tex1, vec2(texCoord.x, 1 - texCoord.y));
    vec4 data2 = texture(srb_tex2, vec2(texCoord.x, 1 - texCoord.y));
    vec4 data3 = texture(srb_tex3, vec2(texCoord.x, 1 - texCoord.y));
    vec4 data4 = texture(srb_tex4, vec2(texCoord.x, 1 - texCoord.y));
    vec4 data5 = texture(srb_tex5, vec2(texCoord.x, 1 - texCoord.y));
    vec4 data6 = texture(srb_tex6, vec2(texCoord.x, 1 - texCoord.y));
    vec4 data7 = texture(srb_tex7, vec2(texCoord.x, 1 - texCoord.y));

    srb.z.xv = data1.rgb;
    srb.z.nv = data2.rgb;
    srb.z.xs = data3.rgb;
    srb.z.ns = data4.rgb;
    srb.z.lo = data5.rgb;
    srb.z.oid = int(data6.r);
    srb.z.mid = int(data6.g);
    srb.w = int(data6.b);
    srb.M = int(data7.r);
    srb.W = float(data7.g);
}

void main() {
    Reservior srb;
    read_srb(srb);

    vec4 current = vec4(texture(emission_tex, 
        vec2(texCoord.x, 1 - texCoord.y)).rgb + srb.z.lo, 1.0f);
    int number = int(texture(number_buffer, vec2(texCoord.x, 1 - texCoord.y)).r);
    number = min(number, MAXIMUM_NUMBER-1);
    number += 1;
    vec4 prevColor = texture(color_buffer, vec2(texCoord.x, 1 - texCoord.y)).rgba;
    color = vec4(prevColor.rgb*(float(number-1)/number) + current.rgb * (1.0f/number), 1.0f);
    M = vec4(number, 0,0,1);
}
