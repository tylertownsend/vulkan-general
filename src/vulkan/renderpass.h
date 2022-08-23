#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <array>
#include <vector>

namespace VT {
struct RenderPassOptions {
  VkFormat swapchain_image_format;
  VkDevice device;
  VkPhysicalDevice physical_device;
};

VkFormat find_depth_format(VkPhysicalDevice physical_device);
/**
 * @brief Takes a list of candidates from most desirable to least desirable and
 * returns the first one that is supported.
 * 
 * @param candidates 
 * @param tiling 
 * @param features 
 * @return VkFormat The most desirable supported format.
 */
VkFormat find_supported_format(
    VkPhysicalDevice physical_device,
    const std::vector<VkFormat>& candidates, 
    VkImageTiling tiling,
    VkFormatFeatureFlags features);
VkRenderPass CreateRenderPass(RenderPassOptions& options);

VkRenderPass CreateRenderPass(RenderPassOptions& options) {
  VkRenderPass render_pass;

  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = options.swapchain_image_format;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  // The loadOp and storeOp determine what to do with the data in the attachment
  // before rendering and after rendering.
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef{};
  // which attachment to reference by its index in the attachment descriptions array.
  // Our array consists of a single VkAttachmentDescription, so its index is 0.
  colorAttachmentRef.attachment = 0;
  // The layout specifies which layout we would like the attachment to have
  // during a subpass that uses this reference
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription depthAttachment{};
  depthAttachment.format = find_depth_format(options.physical_device);
  depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthAttachmentRef{};
  depthAttachmentRef.attachment = 1;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  // The index of the attachment in this array is directly referenced from the fragment
  // shader with the layout(location = 0) out vec4 outColor directive!
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;
  subpass.pDepthStencilAttachment = &depthAttachmentRef;

  // these transitions are controlled by subpass dependencies, which specify memory
  // and execution dependencies between subpasses
  // two built-in dependencies that take care of transition at start of render pass and end of renders pass.
  // at the start it assumes transition occurs but we haven't acquired image yet.
  VkSubpassDependency dependency{};
  // The special value VK_SUBPASS_EXTERNAL refers to the implicit subpass before or after the render pass
  // depending on whether it is specified in srcSubpass or dstSubpass.
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  // The dstSubpass must always be higher than srcSubpass to prevent cycles in the
  // dependency graph (unless one of the subpasses is VK_SUBPASS_EXTERNAL).
  dependency.dstSubpass = 0;
  // We need to wait for the swap chain to finish reading from the image before we can access it.
  // This can be accomplished by waiting on the color attachment output stage itself.
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = 0;
  // These settings will prevent the transition from happening until it's actually necessary
  // (and allowed): when we want to start writing colors to it.
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


  // A subpass can only use a single depth stencil attchment
  std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

  VkRenderPassCreateInfo render_pass_info{};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
  render_pass_info.pAttachments = attachments.data();
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;
  render_pass_info.dependencyCount = 1;
  render_pass_info.pDependencies = &dependency;


  if (vkCreateRenderPass(options.device, &render_pass_info, nullptr, &render_pass) != VK_SUCCESS) {
    throw std::runtime_error("failed to create render pass!");
  }

  return render_pass;
}

VkFormat find_depth_format(const VkPhysicalDevice physical_device) {
  return find_supported_format(
    physical_device,
    {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
    VK_IMAGE_TILING_OPTIMAL,
    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
  );
}

VkFormat find_supported_format(
    const VkPhysicalDevice physical_device,
    const std::vector<VkFormat>& candidates, 
    VkImageTiling tiling,
    VkFormatFeatureFlags features) {

  for (auto format : candidates) {
    VkFormatProperties props;

    vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features)) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }
  throw std::runtime_error("failed to find supported format!");
}
} // VT