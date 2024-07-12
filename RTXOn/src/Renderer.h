#pragma once

#include "Walnut/Image.h"

#include <memory>
#include <glm/glm.hpp>

class Renderer
{
public:
	Renderer() = default;

	void OnResize(const uint32_t width, const uint32_t height);
	void Render();

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return finalImage; }
private:
	// coord should be in in -1->1 range
	glm::vec4 PerPixel(const glm::vec2 coord, const float radiusSquared);
private:
	std::shared_ptr<Walnut::Image> finalImage;
	uint32_t* imageData = nullptr;
};