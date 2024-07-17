#include "VertexBuffer.h"

VkBuffer vertexBuffer;
VkDeviceMemory vertexBufferMemory;
VkBuffer normalBuffer;
VkDeviceMemory normalBufferMemory;

void VertexBuffer::CreateVertexBuffer(const std::vector<Triangle>& triangles) {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;

    for (const auto& tri : triangles) {
        vertices.push_back(tri.posA);
        vertices.push_back(tri.posB);
        vertices.push_back(tri.posC);

        glm::vec3* norms = tri.GetNormals();
        normals.push_back(norms[0]);
        normals.push_back(norms[1]);
        normals.push_back(norms[2]);
    }

    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    // Create vertex buffer and memory (use Vulkan functions for buffer creation and memory allocation)
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
        | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffer, vertexBufferMemory);

    // Copy vertex data to buffer
    void* data;
    vkMapMemory(device, vertexBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, vertexBufferMemory);

    // Repeat for normal buffer
    bufferSize = sizeof(normals[0]) * normals.size();
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
        | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, normalBuffer, normalBufferMemory);
    vkMapMemory(device, normalBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, normals.data(), (size_t)bufferSize);
    vkUnmapMemory(device, normalBufferMemory);
}

void VertexBuffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
    VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

uint32_t VertexBuffer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && 
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void VertexBuffer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    //VkCommandBufferBeginInfo beginInfo = {};
    //beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    //if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    //    throw std::runtime_error("failed to begin recording command buffer!");
    //}

    //VkRenderPassBeginInfo renderPassInfo = {};
    //renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    //renderPassInfo.renderPass = renderPass;
    //renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    //renderPassInfo.renderArea.offset = { 0, 0 };
    //renderPassInfo.renderArea.extent = swapChainExtent;

    //VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
    //renderPassInfo.clearValueCount = 1;
    //renderPassInfo.pClearValues = &clearColor;

    //vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    //vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    //VkBuffer vertexBuffers[] = { vertexBuffer };
    //VkDeviceSize offsets[] = { 0 };
    //vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    //VkBuffer normalBuffers[] = { normalBuffer };
    //vkCmdBindVertexBuffers(commandBuffer, 1, 1, normalBuffers, offsets);

    //vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);

    //vkCmdEndRenderPass(commandBuffer);

    //if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    //    throw std::runtime_error("failed to record command buffer!");
    //}
}
