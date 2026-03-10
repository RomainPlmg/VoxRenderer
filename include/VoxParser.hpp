#pragma once

#include <array>
#include <glm/glm.hpp>
#include <vector>
#include <filesystem>

struct Voxel {
    glm::i8vec3 coord = glm::ivec3(0);
    uint8_t colorIndex = 0;
};

struct VoxModel {
    glm::u32vec3 size = glm::ivec3(0);
    std::vector<Voxel> voxels;
    std::array<glm::u8vec4, 256> palette;
};

struct VoxScene {
    std::vector<VoxModel> models;
};

class VoxParser {
public:
    void parse(const std::filesystem::path& path, VoxScene& scene);
private:
};
