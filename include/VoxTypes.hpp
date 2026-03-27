#pragma once

#include <glm/glm.hpp>
#include <optional>
#include <string>


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
    Diffuse = 0,
    Metal = 1,
    Glass = 2,
    Emit = 3,
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
