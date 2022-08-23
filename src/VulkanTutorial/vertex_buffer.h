#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>

# define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "command_buffer.h"
#include "indices.h"
#include "vertex.h"

namespace VT {

}