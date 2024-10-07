#include <iostream>
#include <GLFW/glfw3.h>

#include "HelloTriangle.h"


int main() {

    HelloTriangle app;
    try {
        app.Run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
