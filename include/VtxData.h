#pragma once
#include "mai_renderer.h"

#include <cstdint>
#include <vector>

constexpr const uint32_t kMaxLODs = 8;
struct Mesh {
  uint32_t lodCount = 1;
  uint32_t indexOffset = 0;
  uint32_t vertexOffset = 0;

  uint32_t vertexCount = 0;
  uint32_t materialID = 0;

  uint32_t lodOffset[kMaxLODs + 1] = {0};

  inline uint32_t getLODIndices(uint32_t lod) const {
    return lod < lodCount ? lodOffset[lod + 1] - lodOffset[lod] : 0;
  }
};

struct MeshData {
  MAI::VertextInput streams{};
  std::vector<uint32_t> indexData;
  std::vector<uint8_t> vertexData;
};
