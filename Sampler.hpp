#pragma once
#include <vector>
#include "Image.hpp"

#include <glm/glm.hpp>

enum class SamplerType {
    SINGLE=0,
    SS_GRID,
    SS_RANDOM,
    SS_JITTER,
    SS_RG,
    SS_QUINCUNX,
    SS_QMC
};

class Sampler {
public:
    virtual ~Sampler(){};
    virtual void pickInPixel(const Image &frame,
        int w, int h, std::vector<std::pair<glm::vec2, float>> &selections) = 0;
};

class SingleSampler: public Sampler {
public:
    virtual ~SingleSampler();
    virtual void pickInPixel(const Image &frame,
        int w, int h, std::vector<std::pair<glm::vec2, float>> &selections);
};

class SuperSamplerGrid: public Sampler {
    int size;
public:
    SuperSamplerGrid(int size);
    virtual ~SuperSamplerGrid();
    virtual void pickInPixel(const Image &frame,
        int w, int h, std::vector<std::pair<glm::vec2, float>> &selections);
};

class SuperSamplerRandom: public Sampler {
    int size;
public:
    SuperSamplerRandom(int size);
    virtual ~SuperSamplerRandom();
    virtual void pickInPixel(const Image &frame,
        int w, int h, std::vector<std::pair<glm::vec2, float>> &selections);
};

class SuperSamplerJitter: public Sampler {
    int size;
public:
    SuperSamplerJitter(int size);
    virtual ~SuperSamplerJitter();
    virtual void pickInPixel(const Image &frame,
        int w, int h, std::vector<std::pair<glm::vec2, float>> &selections);
};


class SuperSamplerRotatedGrid: public Sampler {
public:
    static const float rotate_angle;
    static const glm::mat2 rotate_matrix;
    int size;

    SuperSamplerRotatedGrid(int size);
    virtual ~SuperSamplerRotatedGrid();
    virtual void pickInPixel(const Image &frame,
        int w, int h, std::vector<std::pair<glm::vec2, float>>&selections);
};


class SuperSamplerQuincunx: public Sampler {
public:
    SuperSamplerQuincunx();
    virtual ~SuperSamplerQuincunx();
    virtual void pickInPixel(const Image &frame,
        int w, int h, std::vector<std::pair<glm::vec2, float>> &selections);
};

class SuperSamplerQMC: public Sampler {
    int size;
public:
    SuperSamplerQMC(int size);
    virtual ~SuperSamplerQMC();
    virtual void pickInPixel(const Image &frame,
        int w, int h, std::vector<std::pair<glm::vec2, float>> &selections);
};
