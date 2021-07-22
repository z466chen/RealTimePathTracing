#pragma once
#include <glm/glm.hpp>
#include "defines.hpp"

#define B 0x100
#define BM 0xff

#define N 0x1000



class PerlinNoiseGenerator {
    static bool isInitialized;
    static int p[B+B+2];
    static glm::vec3 g3[B+B+2];

    static void __init();
    
    static double noise(const glm::vec3 &t);
public:
    PerlinNoiseGenerator();
    double turbulence(const glm::vec3 &t) const;
    double getSize() const {
        return B;
    }
};