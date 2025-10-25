//
// Created by lyhux on 2025/10/25.
//

#include "LkSwapChain.h"

#include <algorithm>
#include <stdexcept>
#include <GLFW/glfw3.h>

LkSwapChain::LkSwapChain(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t graphicsFamilyIndex,
                         uint32_t presentFamilyIndex)
    : physicalDevice_(physicalDevice), device_(device), graphicsFamilyIndex_(graphicsFamilyIndex),
      presentFamilyIndex_(presentFamilyIndex) {
    surface_ = nullptr;
}

void LkSwapChain::createSwapChain(int width, int height) {
    querySupport();
    chooseSwapSurfaceFormat();
    chooseSwapPresentMode();
    chooseSwapExtent(width, height);

    //
    uint32_t imageCount = capabilities_.minImageCount + 1;
    if (capabilities_.maxImageCount > 0 && imageCount > capabilities_.maxImageCount) {
        imageCount = capabilities_.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface_;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat_.format;
    createInfo.imageColorSpace = surfaceFormat_.colorSpace;
    createInfo.imageExtent = extent_;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndices[] = {graphicsFamilyIndex_, presentFamilyIndex_};

    if (graphicsFamilyIndex_ != presentFamilyIndex_) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = capabilities_.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode_;
    createInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(device_, &createInfo, nullptr, &swapChain_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device_, swapChain_, &imageCount, nullptr);
    swapChainImages_.resize(imageCount);
    vkGetSwapchainImagesKHR(device_, swapChain_, &imageCount, swapChainImages_.data());

    swapChainImageFormat_ = surfaceFormat_.format;
    swapChainExtent_ = extent_;
}

void LkSwapChain::setSurface(VkSurfaceKHR surface) {
    surface_ = surface;
}

LkSwapChain::~LkSwapChain() {
    vkDestroySwapchainKHR(device_, swapChain_, nullptr);
}

void LkSwapChain::querySupport() {
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice_, surface_, &capabilities_);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface_, &formatCount, nullptr);

    if (formatCount != 0) {
        formats_.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface_, &formatCount, formats_.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice_, surface_, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        presentModes_.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice_, surface_, &presentModeCount, presentModes_.data());
    }
}

void LkSwapChain::chooseSwapSurfaceFormat() {
    for (const auto &availableFormat: formats_) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace ==
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormat_ = availableFormat;
        }
    }

    surfaceFormat_ = formats_[0];
}

void LkSwapChain::chooseSwapPresentMode() {
    for (const auto &availablePresentMode: presentModes_) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            presentMode_ = availablePresentMode;
        }
    }
    presentMode_ = VK_PRESENT_MODE_FIFO_KHR;
}

void LkSwapChain::chooseSwapExtent(int width, int height) {
    if (capabilities_.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        extent_ = capabilities_.currentExtent;
    } else {
        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities_.minImageExtent.width,
                                        capabilities_.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities_.minImageExtent.height,
                                         capabilities_.maxImageExtent.height);

        extent_ = actualExtent;
    }
}
