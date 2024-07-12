#pragma once

#include "Walnut/Image.h"

#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

#include <memory>
#include <glm/glm.hpp>

class Renderer
{
public:
	Renderer() = default;

	void OnResize(const uint32_t width, const uint32_t height);
	void Render(const Scene& scene, const Camera& camera);

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return finalImage; }
private:
	// coord should be in in -1->1 range
	glm::vec4 TraceRay(const Scene& scene, const Ray& ray);
private:
	std::shared_ptr<Walnut::Image> finalImage;
	uint32_t* imageData = nullptr;
};