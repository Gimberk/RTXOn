#pragma once

#include "Walnut/Image.h"

#include "Util/Camera.h"
#include "Util/Ray.h"
#include "Scene.h"
#include "Util/HitRecord.h"

#include <memory>

class Renderer
{
public:
	struct Settings {
		bool accumulate = true;
		bool correctGamma = false;

		int bounceLimit = 10;

		float gammaCorrection = 0.5f;
	};
public:
	void OnResize(const uint32_t width, const uint32_t height);
	void Render(const Scene& scene, const Camera& camera);

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return finalImage; }

	void ResetFrameIndex() { frameIndex = 1; }
	Settings& GetSettings() { return settings; }
private:
	// Ray-gen
	glm::vec4 PerPixel(const uint32_t x, const uint32_t y);

	// coord should be in in -1->1 range
	HitRecord TraceRay(const Ray& ray);
	HitRecord RayMiss(const Ray& ray);
private:
	std::shared_ptr<Walnut::Image> finalImage;

	Settings settings;

	std::vector<uint32_t> imageRowIterator, imageColumnIterator;

	const Scene* activeScene = nullptr;
	const Camera* activeCamera = nullptr;

	uint32_t* imageData = nullptr;
	glm::vec4* accumulationData = nullptr;

	uint32_t frameIndex = 1;
};