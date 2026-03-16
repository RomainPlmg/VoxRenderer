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
    glm::i8vec3 coord = glm::ivec3(0);
    uint8_t colorIndex = 0;
};

struct VoxModel {
    glm::u32vec3 size = glm::ivec3(0);
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
    std::optional<glm::uvec3> translation;
    std::optional<uint32_t> frameIdx;
};

struct VoxFrameAttributes {
    std::unordered_map<std::string, VoxFrameAttribute> data;
};

struct VoxModelAttribute {
    uint32_t frameIdx;
};

struct VoxModelAttributes {
    std::unordered_map<std::string, VoxModelAttribute> data;
};

struct VoxTransformNode : public VoxNode {
    uint32_t childId;
    int32_t _reservedId;
    uint32_t layerId;
    uint32_t nbFrames;
    VoxFrameAttributes frameAttributes;
};

struct VoxGroupNode : public VoxNode {
    uint32_t nbChildren;
    std::vector<uint32_t> childrenIdx;
};

struct VoxShapeNode : public VoxNode {
    uint32_t nbModels;
    std::vector<uint32_t> modelId;
    std::vector<VoxModelAttributes> modelAttributes;
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
    VoxMatType type;
    float weight;
    float rough;
    float spec;
    float ior;
    float att;
    bool plastic;
};

struct VoxMaterialProperties {
    std::unordered_map<std::string, VoxMaterialProperty> data;
};

struct VoxMaterial {
    uint32_t materialId;
    VoxMaterialProperties materialProperties;
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
    uint32_t layerId;
    VoxLayerAttributes attributes;
    int32_t _reservedId;
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
    void readSIZE(std::ifstream &file, uint32_t size, uint32_t nbChilren, VoxScene &scene);
    void readXYZI(std::ifstream &file, uint32_t size, uint32_t nbChilren, VoxScene &scene);
    void readRGBA(std::ifstream &file, uint32_t size, uint32_t nbChilren, VoxScene &scene);
    void readTransformNode(std::ifstream &file, uint32_t size, uint32_t nbChilren, VoxScene &scene);
    void readGroupNode(std::ifstream &file, uint32_t size, uint32_t nbChilren, VoxScene &scene);
    void readShapeNode(std::ifstream &file, uint32_t size, uint32_t nbChilren, VoxScene &scene);
    void readMaterial(std::ifstream &file, uint32_t size, uint32_t nbChilren, VoxScene &scene);
    void readLayer(std::ifstream &file, uint32_t size, uint32_t nbChilren, VoxScene &scene);
    void readRenderObject(std::ifstream &file, uint32_t size, uint32_t nbChilren, VoxScene &scene);
    void readPaletteNote(std::ifstream &file, uint32_t size, uint32_t nbChilren, VoxScene &scene);
    void readIndexMap(std::ifstream &file, uint32_t size, uint32_t nbChilren, VoxScene &scene);

    VoxNodeAttribute parseNodeAttributes(std::string_view content);
};
