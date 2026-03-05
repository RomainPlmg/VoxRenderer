#include "Application.hpp"
#include "WorldLayer.hpp"

int main(int, char **) {
    Application app;

    app.pushLayer<WorldLayer>();

    app.run();
    app.stop();

    return 0;
}
