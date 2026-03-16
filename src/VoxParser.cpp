#include <fstream>

#include "Logger.hpp"
#include "VoxParser.hpp"

void VoxParser::parse(const std::filesystem::path &path, VoxScene &scene) {
    std::ifstream file(path, std::ios_base::binary);
    if (!file) {
        LOG_ERROR("\'{}\': File not found.", path.string());
        return;
    }

    LOG_INFO("Read file \'{}\'.", std::filesystem::absolute(path).string());
    readHeader(file, scene);
    LOG_INFO("File version: {}.", scene.version);

    // Read MAIN chunk
    uint32_t chunkId = readUint32(file);
    jump(file, 4);
    uint32_t nbChildrenChunks = readUint32(file);

    LOG_DEBUG("Read chunk \'{}\'.", uint32ToStr(chunkId));

    auto endPos = file.tellg() + static_cast<std::streamoff>(nbChildrenChunks);
    while (file.tellg() < endPos) {
        readChunk(file, scene);
    }
}

/////////////////////////////////////////////
// Utility functions
/////////////////////////////////////////////

void VoxParser::jump(std::ifstream &file, uint32_t bytes) { file.seekg(bytes, std::ios::cur); }

uint8_t VoxParser::readUint8(std::ifstream &file) {
    uint8_t d;
    file.read(reinterpret_cast<char *>(&d), 1);

    return d;
}

uint32_t VoxParser::readUint32(std::ifstream &file) {
    uint32_t d;
    file.read(reinterpret_cast<char *>(&d), 4);

    return d;
}

int32_t VoxParser::readInt32(std::ifstream &file) {
    int32_t d;
    file.read(reinterpret_cast<char *>(&d), 4);

    return d;
}

std::string VoxParser::readString(std::ifstream &file) {
    uint32_t buffSize = readUint32(file);
    std::string buffer;
    for (size_t i = 0; i < buffSize; i++) {
        buffer.push_back(static_cast<char>(readUint8(file)));
    }

    return buffer;
}

void VoxParser::readDict(std::ifstream &file, std::unordered_map<std::string, std::string> &dict) {
    uint32_t dictSize = readUint32(file);

    for (size_t i = 0; i < dictSize; i++) {
        auto key = readString(file);
        dict[key] = readString(file);
    }
}

std::string VoxParser::uint32ToStr(uint32_t value) {
    char buffer[5];

    buffer[0] = static_cast<char>(value & 0xFF);
    buffer[1] = static_cast<char>((value >> 8) & 0xFF);
    buffer[2] = static_cast<char>((value >> 16) & 0xFF);
    buffer[3] = static_cast<char>((value >> 24) & 0xFF);
    buffer[4] = '\0';

    return std::string(buffer);
}

/////////////////////////////////////////////
// Parser functions
/////////////////////////////////////////////

void VoxParser::readHeader(std::ifstream &file, VoxScene &scene) {
    jump(file, 4);
    scene.version = readUint32(file);
}

void VoxParser::readChunk(std::ifstream &file, VoxScene &scene) {
    uint32_t chunkId = readUint32(file);
    uint32_t chunkContentLength = readUint32(file);
    uint32_t nbChildrenChunks = readUint32(file);

    LOG_DEBUG("Read chunk \'{}\'.", uint32ToStr(chunkId));
    switch (chunkId) {
        case 0x455A4953: // SIZE
            readSIZE(file, chunkContentLength, nbChildrenChunks, scene);
            break;
        case 0x495A5958: // XYZI
            readXYZI(file, chunkContentLength, nbChildrenChunks, scene);
            break;
        case 0x41424752: // RGBA
            readRGBA(file, chunkContentLength, nbChildrenChunks, scene);
            break;
        case 0x4E52546E: // nTRN
            readTransformNode(file, chunkContentLength, nbChildrenChunks, scene);
            break;
        case 0x5052476E: // nGRP
            readGroupNode(file, chunkContentLength, nbChildrenChunks, scene);
            break;
        case 0x5048536E: // nSHP
            readShapeNode(file, chunkContentLength, nbChildrenChunks, scene);
            break;
        case 0x4C54414D: // MATL
            readMaterial(file, chunkContentLength, nbChildrenChunks, scene);
            break;
        case 0x5259414C: // LAYR
            readLayer(file, chunkContentLength, nbChildrenChunks, scene);
            break;
        default:
            LOG_WARN("Unknown chunk ID \'{}\', skip...", uint32ToStr(chunkId));
            jump(file, chunkContentLength + nbChildrenChunks);
            break;
    }
}

void VoxParser::readSIZE(std::ifstream &file, uint32_t size, uint32_t nbChildren, VoxScene &scene) {
    auto before = file.tellg();

    auto &model = scene.models.emplace_back(VoxModel());
    model.size.x = readUint32(file);
    model.size.y = readUint32(file);
    model.size.z = readUint32(file);

    LOG_INFO("Model of size {}x{}x{}.", model.size.x, model.size.y, model.size.z);

    auto after = file.tellg();
    // Safe check, never reach (normally)
    assert(after - before == size);
}

void VoxParser::readXYZI(std::ifstream &file, uint32_t size, uint32_t nbChildren, VoxScene &scene) {
    auto before = file.tellg();

    uint32_t nbVoxels = readUint32(file);
    for (uint32_t i = 0; i < nbVoxels; i++) {
        auto &voxel = scene.models.back().voxels.emplace_back(Voxel());

        // MagicaVoxel uses inversed z axis the vertical axis
        voxel.coord.x = readUint8(file);
        voxel.coord.z = readUint8(file);
        voxel.coord.y = readUint8(file);
        voxel.colorIndex = readUint8(file);

        // LOG_TRACE("Create voxel at {}|{}|{} -> color idx: {}", voxel.coord.x, voxel.coord.y, voxel.coord.z,
        //           voxel.colorIndex);
    }
    LOG_INFO("Model contains {} voxels", scene.models.back().voxels.size());

    auto after = file.tellg();
    // Safe check, never reach (normally)
    assert(after - before == size);
}

void VoxParser::readRGBA(std::ifstream &file, uint32_t size, uint32_t nbChildren, VoxScene &scene) {
    auto before = file.tellg();

    scene.palette[0] = glm::u8vec4(0); // palette[0] reserved for empty voxels
    for (uint32_t i = 0; i < 255; i++) {
        scene.palette[i + 1].r = readUint8(file);
        scene.palette[i + 1].g = readUint8(file);
        scene.palette[i + 1].b = readUint8(file);
        scene.palette[i + 1].a = readUint8(file);

        // LOG_TRACE("Color at palette idx {} -> rgba({}, {}, {}, {})", i + 1, color.r, color.g, color.b, color.a);
    }
    jump(file, 4);

    auto after = file.tellg();
    // Safe check, never reach (normally)
    assert(after - before == size);
}

void VoxParser::readTransformNode(std::ifstream &file, uint32_t size, uint32_t nbChildren, VoxScene &scene) {
    auto before = file.tellg();

    auto nodeId = readUint32(file);

    scene.nodes[nodeId] = std::make_unique<VoxTransformNode>();
    auto &ptr = scene.nodes[nodeId];
    auto &transform = dynamic_cast<VoxTransformNode &>(*ptr);

    std::unordered_map<std::string, std::string> nodeAttributesStr;
    readDict(file, nodeAttributesStr);
    if (nodeAttributesStr.count("_name"))
        transform.nodeAttribute.name = nodeAttributesStr["_name"];
    if (nodeAttributesStr.count("_hidden"))
        transform.nodeAttribute.hidden = (nodeAttributesStr["_hidden"] == "1");


    transform.childId = readUint32(file);
    transform._reservedId = readInt32(file);

    if (transform._reservedId != -1) {
        LOG_ERROR("Transform node \'{}\' must have a reserved id = -1.", nodeId);
    }

    transform.layerId = readUint32(file);
    transform.nbFrames = readUint32(file);

    for (size_t i = 0; i < transform.nbFrames; i++) {
        std::unordered_map<std::string, std::string> frameAttributesStr;
        readDict(file, frameAttributesStr);

        transform.frameAttributes.push_back(parseFrameAttribute(frameAttributesStr));
    }

    auto after = file.tellg();
    // Safe check, never reach (normally)
    assert(after - before == size);
}

void VoxParser::readGroupNode(std::ifstream &file, uint32_t size, uint32_t nbChildren, VoxScene &scene) {
    auto before = file.tellg();

    auto nodeId = readUint32(file);

    scene.nodes[nodeId] = std::make_unique<VoxGroupNode>();
    auto &ptr = scene.nodes[nodeId];
    VoxGroupNode &group = dynamic_cast<VoxGroupNode &>(*ptr);

    std::unordered_map<std::string, std::string> nodeAttributesStr;
    readDict(file, nodeAttributesStr);
    if (nodeAttributesStr.count("_name"))
        group.nodeAttribute.name = nodeAttributesStr["_name"];
    if (nodeAttributesStr.count("_hidden"))
        group.nodeAttribute.hidden = (nodeAttributesStr["_hidden"] == "1");

    group.nbChildren = readUint32(file);

    for (size_t i = 0; i < group.nbChildren; i++) {
        group.childrenIdx.push_back(readUint32(file));
    }

    auto after = file.tellg();
    // Safe check, never reach (normally)
    assert(after - before == size);
}

void VoxParser::readShapeNode(std::ifstream &file, uint32_t size, uint32_t nbChildren, VoxScene &scene) {
    auto before = file.tellg();

    auto nodeId = readUint32(file);

    scene.nodes[nodeId] = std::make_unique<VoxShapeNode>();
    auto &ptr = scene.nodes[nodeId];
    VoxShapeNode &shape = dynamic_cast<VoxShapeNode &>(*ptr);

    std::unordered_map<std::string, std::string> nodeAttributesStr;
    readDict(file, nodeAttributesStr);
    if (nodeAttributesStr.count("_name"))
        shape.nodeAttribute.name = nodeAttributesStr["_name"];
    if (nodeAttributesStr.count("_hidden"))
        shape.nodeAttribute.hidden = (nodeAttributesStr["_hidden"] == "1");

    shape.nbModels = readUint32(file);
    if (shape.nbModels <= 0)
        LOG_ERROR("Shape node \'{}\' must have at least 1 model.", nodeId);

    for (size_t i = 0; i < shape.nbModels; i++) {
        shape.modelId.push_back(readUint32(file));
        std::unordered_map<std::string, std::string> modelAttributeStr;
        readDict(file, modelAttributeStr);

        shape.modelAttributes.push_back(parseModelAttribute(modelAttributeStr));
    }

    auto after = file.tellg();
    // Safe check, never reach (normally)
    assert(after - before == size);
}

void VoxParser::readMaterial(std::ifstream &file, uint32_t size, uint32_t nbChildren, VoxScene &scene) {
    auto before = file.tellg();

    auto &material = scene.materials.emplace_back(VoxMaterial());
    material.materialId = readInt32(file);

    std::unordered_map<std::string, std::string> materialPropertyStr;
    readDict(file, materialPropertyStr);

    material.property = parseMaterialProperty(materialPropertyStr);

    auto after = file.tellg();
    // Safe check, never reach (normally)
    assert(after - before == size);
}

void VoxParser::readLayer(std::ifstream &file, uint32_t size, uint32_t nbChildren, VoxScene &scene) {
    auto before = file.tellg();

    auto &layer = scene.layers.emplace_back(VoxLayer());
    layer.layerId = readUint32(file);

    std::unordered_map<std::string, std::string> layerAttributesStr;
    readDict(file, layerAttributesStr);

    // TODO: Parse the layerAttributesStr

    layer._reservedId = readUint32(file);

    if (layer._reservedId != -1) {
        LOG_ERROR("Layer chunk \'{}\' must have a reserved id = -1.", layer.layerId);
    }

    auto after = file.tellg();
    // Safe check, never reach (normally)
    assert(after - before == size);
}

void VoxParser::readRenderObject(std::ifstream &file, uint32_t size, uint32_t nbChildren, VoxScene &scene) {}

void VoxParser::readPaletteNote(std::ifstream &file, uint32_t size, uint32_t nbChildren, VoxScene &scene) {}

void VoxParser::readIndexMap(std::ifstream &file, uint32_t size, uint32_t nbChildren, VoxScene &scene) {}

VoxNodeAttribute VoxParser::parseNodeAttribute(std::unordered_map<std::string, std::string> &content) {
    VoxNodeAttribute attr{};

    if (content.contains("_name"))
        attr.name = content["_name"];
    if (content.contains("_hidden"))
        attr.hidden = (content["_hidden"] == "1");

    return attr;
}

VoxFrameAttribute VoxParser::parseFrameAttribute(std::unordered_map<std::string, std::string> &content) {
    VoxFrameAttribute frame{};
    if (content.contains("_r"))
        frame.rotation = static_cast<int8_t>(std::stoi(content["_r"]));
    if (content.contains("_t")) {
        std::istringstream ss(content["_t"]);
        ss >> frame.translation.x >> frame.translation.y >> frame.translation.z;
    }
    if (content.contains("_f"))
        frame.frameIdx = std::stoi(content["_f"]);

    return frame;
}

VoxModelAttribute VoxParser::parseModelAttribute(std::unordered_map<std::string, std::string> &content) {
    VoxModelAttribute model{};

    if (content.contains("_f"))
        model.frameIdx = std::stoi(content["_f"]);

    return model;
}

VoxMaterialProperty VoxParser::parseMaterialProperty(std::unordered_map<std::string, std::string> &content) {
    VoxMaterialProperty property{};

    if (content.contains("_type")) {
        if (content["_type"] == "_diffuse")
            property.type = VoxMatType::Diffuse;
        if (content["_type"] == "_metal")
            property.type = VoxMatType::Metal;
        if (content["_type"] == "_glass")
            property.type = VoxMatType::Glass;
        if (content["_type"] == "_emit")
            property.type = VoxMatType::Emit;
    }
    if (content.contains("_weight"))
        property.weight = std::stof(content["_weight"]);
    if (content.contains("_rough"))
        property.rough = std::stof(content["_rough"]);
    if (content.contains("_spec"))
        property.spec = std::stof(content["_spec"]);
    if (content.contains("_ior"))
        property.ior = std::stof(content["_ior"]);
    if (content.contains("_att"))
        property.att = std::stof(content["_att"]);
    if (content.contains("_flux"))
        property.flux = std::stof(content["_flux"]);

    property.plastic = content.contains("_plastic");

    return property;
}
