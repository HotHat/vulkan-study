//
// Created by admin on 2025/12/2.
//

#include "texture_manager.h"

namespace lvk {

TextureId TextureManager::Add(const std::string &path) {
    if (!isInitialized) { throw std::runtime_error("Not Initialization"); }

    auto texture = std::make_shared<Texture>(*context);
    // texture->LoadImage("textures/texture.jpg");
    texture->LoadImage(path);

    textures.emplace_back(std::move(texture));

    return textures.size();
}

// TextureId TextureManager::Add(std::string const &name, Texture &texture) {
    // if (!isInitialized) { throw std::runtime_error("Not Initialization"); }
    // size_t n = Insert(texture);
    // nameMap[name] = n;
    // return n - 1;
// }

} // end namespace lvk
