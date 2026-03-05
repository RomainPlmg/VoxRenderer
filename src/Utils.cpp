#include "Utils.hpp"

#include <fstream>

std::vector<char> readSpirV(const std::string &path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    size_t size = file.tellg();
    std::vector<char> buf(size);
    file.seekg(0);
    file.read(buf.data(), size);
    return buf;
}
