#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <iostream>
#include <vector>
#include <stdexcept>

#include "vertex.h"

namespace VT {

void LoadModel(std::vector<VT::Vertex>& vertices, std::vector<uint32_t>& indices, const char* model_path) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  // obj files can contain an arbitrary number of vertices whereas our application
  // can only render triangles. Luckily the LoadObj has optional parameter to triangulate
  // such faces which is enabled by default.
  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, model_path)) {
    throw std::runtime_error(warn + err);
  }

  std::unordered_map<VT::Vertex, uint32_t> uniqueVertices{};

  for (const auto& shape : shapes) {
    for (const auto& index: shape.mesh.indices) {
      VT::Vertex vertex{};

      vertex.pos = {
        attrib.vertices[3 * index.vertex_index + 0],
        attrib.vertices[3 * index.vertex_index + 1],
        attrib.vertices[3 * index.vertex_index + 2],
      };

      vertex.texCoord = {
        attrib.texcoords[2 * index.texcoord_index + 0],
        1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
      };

      vertex.color = { 1.0f, 1.0f, 1.0f };

      if (uniqueVertices.count(vertex) == 0) {
        uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
        vertices.push_back(vertex);
      }

      indices.push_back(uniqueVertices[vertex]);
    }
  }
}
}
