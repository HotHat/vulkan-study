//
// Created by admin on 2025/12/2.
//

#ifndef LYH_TEXTURE_MANAGER_H
#define LYH_TEXTURE_MANAGER_H
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Texture.h"


namespace lvk {

using TextureId = size_t;

class TextureManager {
public:
    static TextureManager & Instance() {
        static TextureManager instance;
        return instance;
    }

    TextureManager(TextureManager const &) = delete;
    TextureManager(TextureManager &&) = delete;
    TextureManager & operator=(TextureManager const &) = delete;
    TextureManager & operator=(TextureManager &&) = delete;

    void InitContext(RenderContext *context_) { context = context_; isInitialized = true; }
    TextureId Add(const std::string &path);
    // TextureId Add(std::string const &name, Texture &pipeline);

    void Cleanup() {
        for (auto it = textures.begin(); it != textures.end(); ++it) {
            it->get()->Destroy();
        }
    }

    [[nodiscard]] std::shared_ptr<Texture> Get(TextureId textureId) {
        if (textureId > textures.size()) {
            throw std::runtime_error("Not Texture Id" + std::to_string(textureId));
        }
        return textures[textureId - 1];
    }

private:
    TextureManager() = default;
    bool isInitialized = false;
    RenderContext *context = nullptr;
    std::vector<std::shared_ptr<Texture > > textures{};
    // std::unordered_map<std::string, size_t> nameMap{};
};

} // end namespace lvk

#endif //LYH_TEXTURE_MANAGER_H