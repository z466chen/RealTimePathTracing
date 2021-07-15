#include "Sampler.hpp"
#include "general.hpp"
#include "hammersley.hpp"
#include <iostream>

SingleSampler::~SingleSampler() {}

void SingleSampler::pickInPixel(const Image &frame,
    int w, int h, std::vector<std::pair<glm::vec2, float>> &selections) {
    selections.emplace_back(glm::vec2(0.5,0.5), 1.0f);
    return;
}

SuperSamplerGrid::SuperSamplerGrid(int size): size{size} {}

SuperSamplerGrid::~SuperSamplerGrid() {}

void SuperSamplerGrid::pickInPixel(const Image &frame,
    int w, int h, std::vector<std::pair<glm::vec2, float>> &selections) {

    float share = 1.0f/(size);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            selections.emplace_back(glm::vec2(share*(i+0.5), 
                share*(j+0.5)), 1.0f/(size*size));
        }
    }
    return;
}

SuperSamplerRandom::SuperSamplerRandom(int size): size{size} {}

SuperSamplerRandom::~SuperSamplerRandom() {}

void SuperSamplerRandom::pickInPixel(const Image &frame,
    int w, int h, std::vector<std::pair<glm::vec2, float>> &selections) {
    for (int i = 0; i < size*size; ++i) {
        selections.emplace_back(glm::vec2(get_random_float(), 
            get_random_float()), 1.0f/(size*size));
    }
}

SuperSamplerJitter::SuperSamplerJitter(int size): size{size} {}

SuperSamplerJitter::~SuperSamplerJitter() {}

void SuperSamplerJitter::pickInPixel(const Image &frame,
    int w, int h, std::vector<std::pair<glm::vec2, float>> &selections) {

    float share = 1.0f/(size);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            selections.emplace_back(glm::vec2(share*(i+get_random_float()), 
                share*(j+get_random_float())), 1.0f/(size*size));
        }
    }    
}

const float rotate_angle = 26.0f;
const glm::mat2 SuperSamplerRotatedGrid::rotate_matrix = 
    glm::mat2(cos(glm::radians(26.0f)), 
              sin(glm::radians(26.0f)),
             -sin(glm::radians(26.0f)), 
              cos(glm::radians(26.0f)));

SuperSamplerRotatedGrid::SuperSamplerRotatedGrid(int size): size{size} {}

SuperSamplerRotatedGrid::~SuperSamplerRotatedGrid() {}

void SuperSamplerRotatedGrid::pickInPixel(const Image &frame,
    int w, int h, std::vector<std::pair<glm::vec2, float>> &selections) {

    float share = 1.0f/(size);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            glm::vec2 rotated_coord = rotate_matrix * 
                glm::vec2(share*(i+0.5) - 0.5, share*(j+0.5) - 0.5) + 
                glm::vec2(0.5f,0.5f);
            selections.emplace_back(glm::vec2(glm::clamp(rotated_coord.x, 0.0f, 1.0f), 
                glm::clamp(rotated_coord.y, 0.0f, 1.0f)),1.0f/(size*size));
        }
    }    
}

SuperSamplerQuincunx::SuperSamplerQuincunx(){}

SuperSamplerQuincunx::~SuperSamplerQuincunx() {}

void SuperSamplerQuincunx::pickInPixel(const Image &frame,
    int w, int h, std::vector<std::pair<glm::vec2, float>> &selections) {

    selections.emplace_back(glm::vec2(0.5,0.5), 0.5);
    selections.emplace_back(glm::vec2(1.0,1.0), 0.125);
    
    if (w == 0 && h == 0) {
        selections.emplace_back(glm::vec2(0,0), 0.125);
        selections.emplace_back(glm::vec2(0.0,1.0), 0.125);
        selections.emplace_back(glm::vec2(1.0,0.0), 0.125);
    } else if (w == 0) {
        selections.emplace_back(glm::vec2(1.0,0.0), 0.125);
    } else if (h == 0) {
        selections.emplace_back(glm::vec2(0.0,1.0), 0.125);
    } 
    return;
}

SuperSamplerQMC::SuperSamplerQMC(int size): size{size} {}

SuperSamplerQMC::~SuperSamplerQMC(){}

void SuperSamplerQMC::pickInPixel(const Image &frame,
        int w, int h, std::vector<std::pair<glm::vec2, float>> &selections) {

    for (int i = 0; i < 2; ++i) {
        double *select = hammersley(i, 2, size*size);
        selections.emplace_back(glm::vec2(select[0], select[1]), 1.0f/(2));
        delete [] select;
    }
}
