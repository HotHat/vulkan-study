//
// Created by lyhux on 2025/10/25.
//

#ifndef LYH_LK_SWAPCHAIN_H
#define LYH_LK_SWAPCHAIN_H
#include <vulkan/vulkan_core.h>
#include <vector>


class LkSwapChain {
public:
    explicit LkSwapChain(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, uint32_t graphicsFamilyIndex, uint32_t presentFamilyIndex);
    void createSwapChain(int width, int height);
    void setSurface(VkSurfaceKHR surface);
    ~LkSwapChain();
private:
    void querySupport();
    void chooseSwapSurfaceFormat();
    void chooseSwapPresentMode();
    void chooseSwapExtent(int width, int height);

private:
    VkPhysicalDevice physicalDevice_ = nullptr;
    VkDevice device_ = nullptr;
    uint32_t graphicsFamilyIndex_ = 0;
    uint32_t presentFamilyIndex_ = 0;
    VkSurfaceKHR surface_ = nullptr;

    //
    VkSurfaceCapabilitiesKHR capabilities_{};
    std::vector<VkSurfaceFormatKHR> formats_;
    std::vector<VkPresentModeKHR> presentModes_;
    //
    VkSurfaceFormatKHR surfaceFormat_{};
    VkPresentModeKHR presentMode_;
    VkExtent2D extent_{};

    //
    VkSwapchainKHR swapChain_{};
    std::vector<VkImage> swapChainImages_;
    VkFormat swapChainImageFormat_;
    VkExtent2D swapChainExtent_{};
    std::vector<VkImageView> swapChainImageViews_;
    std::vector<VkFramebuffer> swapChainFramebuffers_;
};


#endif //LYH_LK_SWAPCHAIN_H
