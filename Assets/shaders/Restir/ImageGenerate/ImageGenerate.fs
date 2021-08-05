#version 330

uniform sampler2D emission_tex;

uniform sampler2D srb_tex1;
uniform sampler2D srb_tex2;
uniform sampler2D srb_tex3;
uniform sampler2D srb_tex4;
uniform sampler2D srb_tex5;
uniform sampler2D srb_tex6;
uniform sampler2D srb_tex7;

out vec4 fragColor;

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

    fragColor = vec4(texture(emission_tex, 
        vec2(texCoord.x, 1 - texCoord.y)).rgb + srb.z.lo, 1.0f);
}
