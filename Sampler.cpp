#include "Sampler.hpp"
#include "general.hpp"
#include "hammersley.hpp"
#include <iostream>

SingleSampler::~SingleSampler() {}

void SingleSampler::pick(glm::vec2 &sample, float &weight, 
    std::vector<glm::ivec2> &pixels) {
    size_t x = count % width;
    size_t y = count / width;
    sample = glm::vec2(x+0.5f, y+0.5f);
    weight = 1.0f;
    pixels.emplace_back(x,y);
    ++count;
    return;
}

SuperSamplerGrid::SuperSamplerGrid(size_t size): count{0}, size{size} {}

SuperSamplerGrid::~SuperSamplerGrid() {}

void SuperSamplerGrid::pick(glm::vec2 &sample, float &weight, 
        std::vector<glm::ivec2> &pixels) {
    size_t pid = count / (size*size);
    size_t lid = count % (size*size);
    size_t x = pid % width;
    size_t y = pid / width;
    size_t i = lid / size;
    size_t j = lid % size;

    float share = 1.0f/(size);

    sample = glm::vec2(x+share*(i+0.5), y+share*(j+0.5));
    weight = 1.0f/(size*size);
    pixels.emplace_back(x, y);
    ++count;
    return;
}

SuperSamplerRandom::SuperSamplerRandom(size_t size): count{0}, size{size} {}

SuperSamplerRandom::~SuperSamplerRandom() {}

void SuperSamplerRandom::pick(glm::vec2 &sample, float &weight, 
        std::vector<glm::ivec2> &pixels) {
    size_t pid = count / (size*size);
    size_t x = pid % width;
    size_t y = pid / width;

    sample = glm::vec2(x+get_random_float(), y+get_random_float());
    weight = 1.0f/(size*size);
    pixels.emplace_back(x, y);
    ++count;
    return;
}

SuperSamplerJitter::SuperSamplerJitter(size_t size): count{0}, size{size} {}

SuperSamplerJitter::~SuperSamplerJitter() {}

void SuperSamplerJitter::pick(glm::vec2 &sample, float &weight, 
        std::vector<glm::ivec2> &pixels) {
    size_t pid = count / (size*size);
    size_t lid = count % (size*size);
    size_t x = pid % width;
    size_t y = pid / width;
    size_t i = lid / size;
    size_t j = lid % size;


    float share = 1.0f/(size);

    sample = glm::vec2(x+share*(i+get_random_float()), 
        y+share*(j+get_random_float()));
    weight = 1.0f/(size*size);
    pixels.emplace_back(x, y);
    ++count;
    return;
}

const float rotate_angle = 26.0f;
const glm::mat2 SuperSamplerRotatedGrid::rotate_matrix = 
    glm::mat2(cos(glm::radians(26.0f)), 
              sin(glm::radians(26.0f)),
             -sin(glm::radians(26.0f)), 
              cos(glm::radians(26.0f)));

SuperSamplerRotatedGrid::SuperSamplerRotatedGrid(size_t size): count{0}, size{size} {}

SuperSamplerRotatedGrid::~SuperSamplerRotatedGrid() {}

void SuperSamplerRotatedGrid::pick(glm::vec2 &sample, float &weight, 
        std::vector<glm::ivec2> &pixels) {
    size_t pid = count / (size*size);
    size_t lid = count % (size*size);
    size_t x = pid % width;
    size_t y = pid / width;
    size_t i = lid / size;
    size_t j = lid % size;

    float share = 1.0f/(size);

    glm::vec2 rotated_coord = rotate_matrix * 
        glm::vec2(share*(i+0.5) - 0.5, share*(j+0.5) - 0.5) + 
        glm::vec2(0.5f,0.5f);
    
    sample = glm::vec2(x+glm::clamp(rotated_coord.x, 0.0f, 1.0f), 
        y+glm::clamp(rotated_coord.y, 0.0f, 1.0f));
    weight = 1.0f/(size*size);
    pixels.emplace_back(x, y);
    ++count;
    return;  
}

SuperSamplerQuincunx::SuperSamplerQuincunx(): count{0} {}

SuperSamplerQuincunx::~SuperSamplerQuincunx() {}

void SuperSamplerQuincunx::pick(glm::vec2 &sample, float &weight, 
        std::vector<glm::ivec2> &pixels) {
    size_t edge_odd_index_upper_limit = ((width+1)/2)*((height+1)/2);
    size_t edge_index_upper_limit = (width+1)*(height+1);
    

    if (count < edge_index_upper_limit) {
        size_t x = (count % (width+1));
        size_t y = (count / (width+1));
        sample = glm::vec2(x,y);
        std::vector<std::pair<int, int>> indices = 
            {{-1, 0}, {1,0}, {0,-1}, {0,1}};
        for (auto & index: indices) {
            size_t real_x = x + index.first;
            size_t real_y = y + index.second;
            if (real_x > 0 && real_x < width &&
                real_y > 0 && real_y < height) {

                pixels.emplace_back(real_x, real_y);
            }
        }
        weight = 0.125;
    } else {
        size_t real_count = count - edge_index_upper_limit;
        size_t x = (real_count % width);
        size_t y = (real_count / width);
        
        sample = glm::vec2(x+0.5f,y+0.5f);
        pixels.emplace_back(x, y);
        weight = 0.5;
    }

    ++count;
    return;
}

SuperSamplerQMC::SuperSamplerQMC(size_t size): count{0}, size{size} {}

SuperSamplerQMC::~SuperSamplerQMC(){}

void SuperSamplerQMC::pick(glm::vec2 &sample, float &weight, 
        std::vector<glm::ivec2> &pixels) {
    size_t pid = count / (size*size);
    size_t lid = count % (size*size);
    size_t x = pid % width;
    size_t y = pid / width;

    double *select = hammersley(lid, 2, size*size);
    sample = glm::vec2(x+select[0], y+select[1]);
    weight = 1.0f/(size*size);
    pixels.emplace_back(x, y);
    delete [] select;
    ++count;
    return;
}

