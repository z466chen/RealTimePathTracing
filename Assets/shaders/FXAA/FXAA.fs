#version 330 core

uniform sampler2D color_tex;

in vec2 texCoord;

out vec4 fragColor;
uniform vec2 window_size;
uniform float gs_thresh;
uniform float gs_mult;
uniform float gs_reduce;
uniform float delta_max;


vec4 read_color_offset(int dx, int dy) {
    return texture(color_tex, vec2(texCoord.x+(dx/window_size.x), 1 - texCoord.y-(dy/window_size.y)));
}

void main() {
    vec3 color = read_color_offset(0,0).rgb;
    vec3 color_ul = read_color_offset(-1,1).rgb;
    vec3 color_ur = read_color_offset(1,1).rgb;
    vec3 color_dl = read_color_offset(-1,-1).rgb;
    vec3 color_dr = read_color_offset(1,-1).rgb;

    const vec3 grey_scale_factor = vec3(0.299,0.587,0.114);

    float gs = dot(color, grey_scale_factor);
    float gs_ul = dot(color_ul, grey_scale_factor);
    float gs_ur = dot(color_ur, grey_scale_factor);
    float gs_dl = dot(color_dl, grey_scale_factor);
    float gs_dr = dot(color_dr, grey_scale_factor);

    float gs_min = min(gs, min(min(gs_ul, gs_ur), min(gs_dl, gs_dr)));
    float gs_max = max(gs, max(max(gs_ul, gs_ur), max(gs_dl, gs_dr)));

    // if ((gs_max - gs_min) < gs_max*gs_thresh) {
    //     fragColor = vec4(color, 1.0f);
    //     return;
    // }

    vec2 gs_gradient = vec2(gs_dl + gs_dr - gs_ul - gs_ur, 
        gs_dl + gs_ul - gs_dr - gs_ur);

    float gs_gradient_reduce = max((gs_ul + gs_ur + gs_dl + gs_dr) * 0.25 * gs_mult, gs_reduce);

	float gs_gradient_reduce_factor = 1.0 / (min(abs(gs_gradient.x), abs(gs_gradient.y)) + gs_gradient_reduce);

    gs_gradient = clamp(gs_gradient * gs_gradient_reduce_factor, vec2(-delta_max), vec2(delta_max)) * 
        vec2(1.0/window_size.x, 1.0/window_size.y);
	
	vec3 color_sample_innerL = texture(color_tex, vec2(texCoord.x, 1 - texCoord.y) + gs_gradient * (1.0/3.0 - 0.5)).rgb;
	vec3 color_sample_innerR = texture(color_tex, vec2(texCoord.x, 1 - texCoord.y) + gs_gradient * (2.0/3.0 - 0.5)).rgb;

	vec3 color_inner = (color_sample_innerL + color_sample_innerR) * 0.5;  

	vec3 color_sample_outerL = texture(color_tex, vec2(texCoord.x, 1 - texCoord.y) + gs_gradient * (0.0/3.0 - 0.5)).rgb;
	vec3 color_sample_outerR = texture(color_tex, vec2(texCoord.x, 1 - texCoord.y) + gs_gradient * (3.0/3.0 - 0.5)).rgb;
	
    vec3 color_outer = (color_sample_outerL + color_sample_outerR) * 0.5;

	vec3 color_sample_avg = color_inner * 0.5 + color_outer * 0.5; 
	
	float gs_sample_avg = dot(color_sample_avg, grey_scale_factor);
	
    fragColor = vec4(color_sample_avg,1.0f);
    return;
	if (gs_sample_avg < gs_min || gs_sample_avg > gs_max) {
		fragColor = vec4(color_inner, 1.0); 
	} else {
		fragColor = vec4(color_sample_avg, 1.0);
	}
}