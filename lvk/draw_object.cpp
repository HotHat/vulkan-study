//
// Created by admin on 2025/11/21.
//

#include "draw_object.h"

namespace lvk {

template<typename T>
void DrawObject<T>::AddTriangle(T t1, T t2, T t3) {
    uint32_t size = vertexes.size();
    vertexes.emplace_back(t1);
    vertexes.emplace_back(t2);
    vertexes.emplace_back(t3);

    indices.emplace_back(size);
    indices.emplace_back(size + 1);
    indices.emplace_back(size + 2);
}

template<typename T>
void DrawObject<T>::AddRectangle(T t1, T t2, T t3, T t4) {
    uint32_t size = vertexes.size();
    vertexes.emplace_back(t1);
    vertexes.emplace_back(t2);
    vertexes.emplace_back(t3);
    vertexes.emplace_back(t4);

    indices.emplace_back(size);
    indices.emplace_back(size + 1);
    indices.emplace_back(size + 2);
    indices.emplace_back(size + 2);
    indices.emplace_back(size + 3);
    indices.emplace_back(size);
}

} // end namespace lvk