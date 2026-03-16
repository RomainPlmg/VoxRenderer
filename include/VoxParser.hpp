#pragma once

#include <array>
#include <filesystem>
#include <fstream>
#include <glm/glm.hpp>
#include <optional>
#include <vector>

/////////////////////////////////////////////
// Basic version of the .vox format
/////////////////////////////////////////////
struct Voxel {
    glm::u8vec3 coord = glm::u8vec3(0);
    uint8_t colorIndex = 0;
};

struct VoxModel {
    glm::uvec3 size = glm::uvec3(0);
    std::vector<Voxel> voxels;
};

/////////////////////////////////////////////
// Nodes
/////////////////////////////////////////////
struct VoxNodeAttribute {
    std::string name;
    bool hidden;
};

struct VoxNode {
    virtual ~VoxNode() = default;

    VoxNodeAttribute nodeAttribute;
};

struct VoxFrameAttribute {
    std::optional<uint8_t> rotation;
    glm::ivec3 translation;
    std::optional<uint32_t> frameIdx;
};

struct VoxModelAttribute {
    std::optional<uint32_t> frameIdx;
};

struct VoxTransformNode : VoxNode {
    uint32_t childId;
    int32_t _reservedId;
    uint32_t layerId;
    uint32_t nbFrames;
    std::vector<VoxFrameAttribute> frameAttributes;
};

struct VoxGroupNode : VoxNode {
    uint32_t nbChildren;
    std::vector<uint32_t> childrenIdx;
};

struct VoxShapeNode : VoxNode {
    uint32_t nbModels;
    std::vector<uint32_t> modelId;
    std::vector<VoxModelAttribute> modelAttributes;
};

/////////////////////////////////////////////
// Material
/////////////////////////////////////////////

enum class VoxMatType {
    Diffuse,
    Metal,
    Glass,
    Emit,
};

struct VoxMaterialProperty {
    std::optional<VoxMatType> type;
    std::optional<float> weight;
    std::optional<float> rough;
    std::optional<float> spec;
    std::optional<float> ior;
    std::optional<float> att;
    std::optional<float> flux;
    std::optional<bool> plastic;
};

struct VoxMaterial {
    uint32_t materialId = 0;
    VoxMaterialProperty property;
};

/////////////////////////////////////////////
// Layer
/////////////////////////////////////////////

struct VoxLayerAttribute {
    std::string name;
    bool hidden;
};

struct VoxLayerAttributes {
    std::unordered_map<std::string, VoxLayerAttributes> data;
};

struct VoxLayer {
    uint32_t layerId = 0;
    VoxLayerAttributes attributes;
    int32_t _reservedId = 0;
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
    std::vector<VoxMaterial> materials;
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
