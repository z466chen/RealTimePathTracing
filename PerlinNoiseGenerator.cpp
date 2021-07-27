#include "PerlinNoiseGenerator.hpp"
#include "general.hpp"
#include "UboConstructor.hpp"
#include <iostream>

#define s_curve(t) ( t * t * (3. - 2. * t) )

#define lerp(t, a, b) ( a + t * (b - a) )

#define setup(i,b0,b1,r0,r1)\
	temp = t[i] + N;\
	b0 = ((int)temp) & BM;\
	b1 = (b0+1) & BM;\
	r0 = temp - (int)temp;\
	r1 = r0 - 1.;

bool PerlinNoiseGenerator::isInitialized = false;
int PerlinNoiseGenerator::p[B+B+2] = {};
glm::vec3 PerlinNoiseGenerator::g3[B+B+2] = {};

PerlinNoiseGenerator::PerlinNoiseGenerator() {
    if (!isInitialized) {
        __init();
		for (int i = 0; i < B+B+2; ++i) {
			UboConstructor::perlin_arr.emplace_back(UboPerlinNoise());
			auto &ubo_perlin = UboConstructor::perlin_arr.back();
			ubo_perlin.p = p[i];
			ubo_perlin.g = g3[i];
		}
        isInitialized = true;
    } 
}

double PerlinNoiseGenerator::noise(const glm::vec3 &t) {
	int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
	float rx0, rx1, ry0, ry1, rz0, rz1, sy, sz, a, b, c, d, temp, u, v;
    glm::vec3 q;
	register int i, j;

	setup(0, bx0,bx1, rx0,rx1);
	setup(1, by0,by1, ry0,ry1);
	setup(2, bz0,bz1, rz0,rz1);

	i = p[ bx0 ];
	j = p[ bx1 ];

	b00 = p[ i + by0 ];
	b10 = p[ j + by0 ];
	b01 = p[ i + by1 ];
	b11 = p[ j + by1 ];

	temp = s_curve(rx0);
	sy = s_curve(ry0);
	sz = s_curve(rz0);

    #define at3(rx,ry,rz) ( rx * q[0] + ry * q[1] + rz * q[2] )


	q = g3[ b00 + bz0 ] ; u = at3(rx0,ry0,rz0);
	q = g3[ b10 + bz0 ] ; v = at3(rx1,ry0,rz0);
	a = lerp(temp, u, v);

	q = g3[ b01 + bz0 ] ; u = at3(rx0,ry1,rz0);
	q = g3[ b11 + bz0 ] ; v = at3(rx1,ry1,rz0);
	b = lerp(temp, u, v);

	c = lerp(sy, a, b);

	q = g3[ b00 + bz1 ] ; u = at3(rx0,ry0,rz1);
	q = g3[ b10 + bz1 ] ; v = at3(rx1,ry0,rz1);
	a = lerp(temp, u, v);

	q = g3[ b01 + bz1 ] ; u = at3(rx0,ry1,rz1);
	q = g3[ b11 + bz1 ] ; v = at3(rx1,ry1,rz1);
	b = lerp(temp, u, v);

	d = lerp(sy, a, b);

	return lerp(sz, c, d);
}

void PerlinNoiseGenerator::__init() {
	int i, j, k;

	for (i = 0 ; i < B ; i++) {
		p[i] = i;
        g3[i] = glm::vec3(get_random_float() * 2.0f - 1.0f,
            get_random_float() * 2.0f - 1.0f,
            get_random_float() * 2.0f - 1.0f); 
		g3[i] = glm::normalize(g3[i]);
	}

	while (--i) {
        std::swap(p[i], p[(int) (get_random_float() * B)]);
	}

	for (i = 0 ; i < B + 2 ; i++) {
		p[B + i] = p[i];

        g3[B + i] = g3[i];
	}
}

double PerlinNoiseGenerator::turbulence(const glm::vec3 &t) const {
    double size = 32;
    double value = 0.0, initialSize = size;

    while(size >= 1)
    {
        glm::vec3 temp = glm::vec3(t.x / size, t.y / size, t.z / size);
        value += noise(temp) * size;
        size /= 2.0;
    }

    return value/initialSize;
}