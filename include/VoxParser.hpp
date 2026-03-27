#pragma once

#include <array>
#include <filesystem>
#include <fstream>
#include <vector>

#include "VoxTypes.hpp"

struct Voxel {
    glm::u8vec3 coord = glm::u8vec3(0);
    uint8_t colorIndex = 0;
};

struct VoxModel {
    glm::uvec3 size = glm::uvec3(0);
    std::vector<Voxel> voxels;
};

/////////////////////////////////////////////
// Final .vox scene data
/////////////////////////////////////////////
struct VoxScene {
    uint32_t version = 0;
    std::vector<VoxModel> models;
    std::array<glm::u8vec4, 256> palette;
    std::unordered_map<uint32_t, std::unique_ptr<VoxNode>> nodes;
    std::vector<VoxLayer> layers;
    std::array<VoxMaterialProperty, 256> materials;
};

/////////////////////////////////////////////
// .vox parser
/////////////////////////////////////////////
class VoxParser {
public:
    void parse(const std::filesystem::path &path, VoxScene &scene);

private:
    void jump(std::ifstream &file, uint32_t bytes);
    uint8_t readUint8(std::ifstream &file);
    uint32_t readUint32(std::ifstream &file);
    int32_t readInt32(std::ifstream &file);
    std::string readString(std::ifstream &file);
    void readDict(std::ifstream &file, std::unordered_map<std::string, std::string> &dict);
    std::string uint32ToStr(uint32_t value);

    void readHeader(std::ifstream &file, VoxScene &scene);
    void readChunk(std::ifstream &file, VoxScene &scene);
    void readSIZE(std::ifstream &file, uint32_t size, uint32_t nbChildren, VoxScene &scene);
    void readXYZI(std::ifstream &file, uint32_t size, uint32_t nbChildren, VoxScene &scene);
    void readRGBA(std::ifstream &file, uint32_t size, uint32_t nbChildren, VoxScene &scene);
    void readTransformNode(std::ifstream &file, uint32_t size, uint32_t nbChildren, VoxScene &scene);
    void readGroupNode(std::ifstream &file, uint32_t size, uint32_t nbChildren, VoxScene &scene);
    void readShapeNode(std::ifstream &file, uint32_t size, uint32_t nbChildren, VoxScene &scene);
    void readMaterial(std::ifstream &file, uint32_t size, uint32_t nbChildren, VoxScene &scene);
    void readLayer(std::ifstream &file, uint32_t size, uint32_t nbChildren, VoxScene &scene);
    void readRenderObject(std::ifstream &file, uint32_t size, uint32_t nbChildren, VoxScene &scene);
    void readPaletteNote(std::ifstream &file, uint32_t size, uint32_t nbChildren, VoxScene &scene);
    void readIndexMap(std::ifstream &file, uint32_t size, uint32_t nbChildren, VoxScene &scene);

    VoxNodeAttribute parseNodeAttribute(std::unordered_map<std::string, std::string> &content);
    VoxFrameAttribute parseFrameAttribute(std::unordered_map<std::string, std::string> &content);
    VoxModelAttribute parseModelAttribute(std::unordered_map<std::string, std::string> &content);
    VoxMaterialProperty parseMaterialProperty(std::unordered_map<std::string, std::string> &content);
};
