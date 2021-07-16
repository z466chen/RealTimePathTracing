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
    virtual void init(Image &img) = 0;
    virtual void pick(glm::vec2 &sample, float &weight, 
        std::vector<glm::ivec2> &pixels) = 0;
    virtual size_t getNos() const = 0;
};

class SingleSampler: public Sampler {
    size_t count;
    size_t width;
    size_t height;
public:
    SingleSampler(): count{0} {}
    virtual ~SingleSampler();
    virtual void init(Image &img) {width = img.width(); height = img.height();}
    virtual size_t getNos() const { return width*height; };
    virtual void pick(glm::vec2 &sample, float &weight, 
        std::vector<glm::ivec2> &pixels);
};

class SuperSamplerGrid: public Sampler {
    size_t count;
    size_t size;
    size_t width;
    size_t height;
public:
    SuperSamplerGrid(size_t size);
    virtual ~SuperSamplerGrid();
    virtual void init(Image &img) {width = img.width(); height = img.height();}
    virtual size_t getNos() const { return  width*height*size*size; };
    virtual void pick(glm::vec2 &sample, float &weight, 
        std::vector<glm::ivec2> &pixels);
};

class SuperSamplerRandom: public Sampler {
    size_t count;
    size_t size;
    size_t width;
    size_t height;
public:
    SuperSamplerRandom(size_t size);
    virtual ~SuperSamplerRandom();
    virtual void init(Image &img) {width = img.width(); height = img.height();}
    virtual size_t getNos() const { return width*height*size*size; };
    virtual void pick(glm::vec2 &sample, float &weight, 
        std::vector<glm::ivec2> &pixels);
};

class SuperSamplerJitter: public Sampler {
    size_t count;
    size_t size;
    size_t width;
    size_t height;
public:
    SuperSamplerJitter(size_t size);
    virtual ~SuperSamplerJitter();
    virtual void init(Image &img) {width = img.width(); height = img.height();}
    virtual size_t getNos() const { return width*height*size*size; };
    virtual void pick(glm::vec2 &sample, float &weight, 
        std::vector<glm::ivec2> &pixels);
};


class SuperSamplerRotatedGrid: public Sampler {
    static const float rotate_angle;
    static const glm::mat2 rotate_matrix;

    size_t count;
    size_t size;
    size_t width;
    size_t height;
public:
    SuperSamplerRotatedGrid(size_t size);
    virtual ~SuperSamplerRotatedGrid();
    virtual void init(Image &img) {width = img.width(); height = img.height();}
    virtual size_t getNos() const { return width*height*size*size; };
    virtual void pick(glm::vec2 &sample, float &weight, 
        std::vector<glm::ivec2> &pixels);
};


class SuperSamplerQuincunx: public Sampler {
    size_t count;
    size_t width;
    size_t height;
public:
    SuperSamplerQuincunx();
    virtual ~SuperSamplerQuincunx();
    virtual void init(Image &img) {width = img.width(); height = img.height();}
    virtual size_t getNos() const { return width*height + (width+1)*(height+1); };
    virtual void pick(glm::vec2 &sample, float &weight, 
        std::vector<glm::ivec2> &pixels);
};

class SuperSamplerQMC: public Sampler {
    size_t size;
    size_t count;
    size_t width;
    size_t height;
public:
    SuperSamplerQMC(size_t size);
    virtual ~SuperSamplerQMC();
    virtual void init(Image &img) {width = img.width(); height = img.height();}
    virtual size_t getNos() const { return width*height*size*size; };
    virtual void pick(glm::vec2 &sample, float &weight, 
        std::vector<glm::ivec2> &pixels);
};
