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
    char chunkId[4];
    uint32_t chunkContentLength = 0;
    uint32_t nbChildrenChunks = 0;
    file.read(chunkId, 4);
    file.read(reinterpret_cast<char *>(&chunkContentLength), 4);
    file.read(reinterpret_cast<char *>(&nbChildrenChunks), 4);

    LOG_DEBUG("Read chunk \'{}\'.", std::string_view(chunkId, 4));

    auto endPos = file.tellg() + static_cast<std::streamoff>(nbChildrenChunks);
    while (file.tellg() < endPos) {
        readChunk(file, scene);
    }
}

void VoxParser::readHeader(std::ifstream &file, VoxScene &scene) {
    file.seekg(4, std::ios::cur); // Skip the "VOX " header
    file.read(reinterpret_cast<char *>(&scene.version), 4);
}

void VoxParser::readChunk(std::ifstream &file, VoxScene &scene) {
    char chunkId[4];
    uint32_t chunkContentLength = 0;
    uint32_t nbChildrenChunks = 0;
    file.read(chunkId, 4);
    file.read(reinterpret_cast<char *>(&chunkContentLength), 4);
    file.read(reinterpret_cast<char *>(&nbChildrenChunks), 4);

    LOG_DEBUG("Read chunk \'{}\'.", std::string_view(chunkId, 4));
    if (std::string_view(chunkId, 4) == "SIZE")
        readSIZE(file, chunkContentLength, nbChildrenChunks, scene);
    else if (std::string_view(chunkId, 4) == "XYZI")
        readXYZI(file, chunkContentLength, nbChildrenChunks, scene);
    else if (std::string_view(chunkId, 4) == "RGBA")
        readRGBA(file, chunkContentLength, nbChildrenChunks, scene);
    else {
        LOG_WARN("Unknown chunk ID \'{}\', skip...", std::string_view(chunkId, 4));
        file.seekg(chunkContentLength + nbChildrenChunks, std::ios::cur);
    }
}

void VoxParser::readSIZE(std::ifstream &file, uint32_t size, uint32_t nbChilren, VoxScene &scene) {
    auto before = file.tellg();

    auto &model = scene.models.emplace_back(VoxModel());
    file.read(reinterpret_cast<char *>(&model.size), 12);

    LOG_INFO("Model of size {}x{}x{}.", model.size.x, model.size.y, model.size.z);

    auto after = file.tellg();
    // Safe check, never reach (normaly)
    assert(after - before == size);
}

void VoxParser::readXYZI(std::ifstream &file, uint32_t size, uint32_t nbChilren, VoxScene &scene) {
    auto before = file.tellg();

    uint32_t nbVoxels = 0;
    file.read(reinterpret_cast<char *>(&nbVoxels), 4);
    for (uint32_t i = 0; i < nbVoxels; i++) {
        auto &voxel = scene.models.back().voxels.emplace_back(Voxel());

        file.read(reinterpret_cast<char *>(&voxel.coord), 3);
        file.read(reinterpret_cast<char *>(&voxel.colorIndex), 1);

        // LOG_TRACE("Create voxel at {}|{}|{} -> color idx: {}", voxel.coord.x, voxel.coord.y, voxel.coord.z,
        //           voxel.colorIndex);
    }
    LOG_INFO("Model contains {} voxels", scene.models.back().voxels.size());

    auto after = file.tellg();
    // Safe check, never reach (normaly)
    assert(after - before == size);
}

void VoxParser::readRGBA(std::ifstream &file, uint32_t size, uint32_t nbChilren, VoxScene &scene) {
    auto before = file.tellg();

    scene.palette[0] = glm::u8vec4(0); // palette[0] reserved for empty voxels
    for (uint32_t i = 0; i < 255; i++) {
        auto &color = scene.palette[i + 1]; // Color [0-254] are mapped to palette index [1-255]
        file.read(reinterpret_cast<char *>(&color), 4);
        // LOG_TRACE("Color at palette idx {} -> rgba({}, {}, {}, {})", i + 1, color.r, color.g, color.b, color.a);
    }
    file.seekg(4, std::ios::cur); // Skip the last color (empty voxel)

    auto after = file.tellg();
    // Safe check, never reach (normaly)
    assert(after - before == size);
}
