#include <fstream>

#include "VoxParser.hpp"
#include "Logger.hpp"

void VoxParser::parse(const std::filesystem::path &path, VoxScene &scene) {
    std::ifstream voxFile(path, std::ios_base::binary);
    if (!voxFile) {
        LOG_ERROR("\'{}\': File not found.", path.string());
        return;
    }
}
