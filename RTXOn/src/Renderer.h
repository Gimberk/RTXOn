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
	struct HitRecord {
		float hitDist;
		glm::vec3 worldNormal, worldPosition;

		int objIndex;
	};

	// Ray-gen
	glm::vec4 PerPixel(const uint32_t x, const uint32_t y);

	// coord should be in in -1->1 range
	HitRecord TraceRay(const Ray& ray);
	HitRecord ClosestHit(const Ray& ray, float hitDist, int objIndex);
	HitRecord RayMiss(const Ray& ray);
private:
	std::shared_ptr<Walnut::Image> finalImage;

	const Scene* activeScene = nullptr;
	const Camera* activeCamera = nullptr;

	uint32_t* imageData = nullptr;
};