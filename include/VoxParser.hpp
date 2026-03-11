#pragma once

#include <array>
#include <filesystem>
#include <glm/glm.hpp>
#include <vector>

struct Voxel {
    glm::i8vec3 coord = glm::ivec3(0);
    uint8_t colorIndex = 0;
};

struct VoxModel {
    glm::u32vec3 size = glm::ivec3(0);
    std::vector<Voxel> voxels;
};

struct VoxScene {
    uint32_t version = 0;
    std::vector<VoxModel> models;
    std::array<glm::u8vec4, 256> palette;
};

class VoxParser {
public:
    void parse(const std::filesystem::path &path, VoxScene &scene);

private:
    void readHeader(std::ifstream &file, VoxScene &scene);
    void readChunk(std::ifstream &file, VoxScene &scene);
    void readSIZE(std::ifstream &file, uint32_t size, uint32_t nbChilren, VoxScene &scene);
    void readXYZI(std::ifstream &file, uint32_t size, uint32_t nbChilren, VoxScene &scene);
    void readRGBA(std::ifstream &file, uint32_t size, uint32_t nbChilren, VoxScene &scene);
};
