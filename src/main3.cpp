//
// Created by HotHat on 2025/10/23.
//

#include <iostream>
#include <GLFW/glfw3.h>

#include "VkInfo.h"


int main() {

    VkInfo app;
    try {
        app.Run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}