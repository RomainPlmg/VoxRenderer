#include "Utils.hpp"

#include <fstream>
#include "Logger.hpp"

std::vector<char> readSpirV(const std::string &path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file) {
        LOG_ERROR("\'{}\': File not found.", path);
        return std::vector<char>();
    }

    size_t size = file.tellg();
    std::vector<char> buf(size);
    file.seekg(0);
    file.read(buf.data(), size);
    return buf;
}
