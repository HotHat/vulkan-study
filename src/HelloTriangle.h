#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <vector>
#include <array>
#include <optional>
#include <string>
#include <fstream>

#define VK_CHECK(result, message) do {\
    VkResult err = result; \
    if (err) { \
       fprintf(stderr, "Detected Vulkan error %d at %s:%d message:%s.\n", int(err), __FILE__, __LINE__, message); \
       abort(); \
   } \
} while(0)

// test functions
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
);

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct SwapChainData {
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
};

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription binding{};
        binding.binding = 0;
        binding.stride = sizeof(Vertex);
        binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return binding;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attrib{};
        attrib[0].binding = 0;
        attrib[0].location = 0;
        attrib[0].format = VK_FORMAT_R32G32_SFLOAT;
        attrib[0].offset = offsetof(Vertex, pos);
        attrib[1].binding = 0;
        attrib[1].location = 1;
        attrib[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attrib[1].offset = offsetof(Vertex, color);

        return attrib;
    }
};

const std::vector<Vertex> g_vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},

    // {{  -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    // {{0.0f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    // {{-1.0f, 0.5f}, {0.0f, 1.0f, 0.0f}},

    // {{  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    // {{1.0f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    // {{0.0f, 0.5f}, {0.0f, 0.0f, 1.0f}},

};

const std::vector<uint16_t> g_indices = {
    0, 1, 2, 2, 3, 0
};

class HelloTriangle {
public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    const std::vector<const char*> validationLayers_ = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> deviceExtensions_ = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };



#ifdef NDEBUG
    const bool enableValidationLayers_ = false;
#else
    const bool enableValidationLayers_ = true;
#endif



    void Run();
    static std::vector<char> ReadFile(const std::string& fileName);
    static VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& code);
    uint32_t FindMemoryType( uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

private:
    GLFWwindow *window_ = nullptr;
    VkInstance instance_ = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger_;

    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkQueue graphicQueue_ = VK_NULL_HANDLE;
    VkQueue presentQueue_ = VK_NULL_HANDLE;
    //
    // SwapChainData swapChainData_;
    //
    VkSwapchainKHR swapChain_ = VK_NULL_HANDLE;
    std::vector<VkImage> swapChainImages_;
    std::vector<VkImageView> swapChainImageViews_;
    VkFormat swapChainImageFormat_ = VK_FORMAT_UNDEFINED;
    VkExtent2D swapChainExtent_ = {};

    VkRenderPass renderPass_ = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout_ = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout_ = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline_ = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> swapChainFramebuffers_;
    VkCommandPool commandPool_ = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers_;

    std::vector<VkSemaphore> imageAvailableSemaphores_;
    std::vector<VkSemaphore> renderFinishedSemaphores_;
    std::vector<VkFence> inFlightFences_;
    std::vector<VkFence> imagesInFlight_;

    // vertex buffer
    VkBuffer vertexBuffer_ = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory_ = VK_NULL_HANDLE;
    // vertex index
    VkBuffer indexBuffer_ = VK_NULL_HANDLE;
    VkDeviceMemory indexBufferMemory_ = VK_NULL_HANDLE;

    // uniform
    std::vector<VkBuffer> uniformBuffers_;
    std::vector<VkDeviceMemory> uniformBuffersMemory_;
    std::vector<void*> uniformBuffersMapped_;
    VkDescriptorPool descriptorPool_ = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets_;


    uint32_t currentFrame_ = 0;

    // help functions
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();

    void setupDebugMessenger();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;
    bool isDeviceSuitable(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresent);
    void recordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void updateUniformBuffer(uint32_t imageIndex);

    //
    void createInstance();
    void pickPhysicalDevice();
    void createSurface();
    void createLogicalDevice();
    void createSwapChain();
    void cleanupSwapChain();
    void recreateSwapChain();
    void createImageViews();
    void createRenderPass();
    void createDescriptorPool();
    void createDescriptorSets();
    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    void createFrameBuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();
    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffer();
    void drawFrame();

    // vulkan functions
    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();

};

