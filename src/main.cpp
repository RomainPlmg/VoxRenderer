#include "Application.hpp"
#include "WorldLayer.hpp"

#include <iostream>

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Invalid number of arguments" << std::endl;
        return EXIT_FAILURE;
    }

    std::string voxFile = std::string(argv[1]);

    Application app;

    app.pushLayer<WorldLayer>(voxFile);

    app.run();
    app.stop();

    return 0;
}
