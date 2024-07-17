#pragma once

#include <vector>

#include "primitives/Triangle.h"
#include "vulkan/vulkan.h"

class VertexBuffer {
public:
	VertexBuffer(const VkDevice& device, const VkPhysicalDevice& physicalDevice) : 
		device(device), physicalDevice(physicalDevice) {}

	void CreateVertexBuffer(const std::vector<Triangle>&);
	void VertexBuffer::CreateBuffer(VkDeviceSize, VkBufferUsageFlags,
		VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&);
	uint32_t FindMemoryType(uint32_t, VkMemoryPropertyFlags);

	void RecordCommandBuffer(VkCommandBuffer, uint32_t);
private:
	const VkDevice device;
	const VkPhysicalDevice physicalDevice;
};