#include "Renderer.h"
#include "Walnut/Random.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glm/gtc/random.hpp>

#include <execution>

namespace Utils {
	float pi = 3.14159265359f;

	static uint32_t ToRGBA(const glm::vec4& color) {
		uint8_t r = (uint8_t)(color.r * 255.0f), g = (uint8_t)(color.g * 255.0f),
			b = (uint8_t)(color.b * 255.0f), a = (uint8_t)(color.a * 255.0f);
		return (a << 24) | (b << 16) | (g << 8) | r;
	}

	// for fun, but most importantly, fast, random numbers!
	static uint32_t PCGHash(uint32_t input) {
		uint32_t state = input * 747796405u + 2891336453u;
		uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
		return (word >> 22u) ^ word;
	}

	static float RandFloat(uint32_t& seed) {
		seed = PCGHash(seed);
		return (float)seed / (float)std::numeric_limits<uint32_t>::max();
	}

	static glm::vec3 RandInUnitSphere(uint32_t& seed) {
		return glm::normalize(glm::vec3(
			RandFloat(seed) * 2.0f - 1.0f,
			RandFloat(seed) * 2.0f - 1.0f,
			RandFloat(seed) * 2.0f - 1.0f
		));
	}

	glm::vec3 FastSphericalRand(uint32_t& seed) {
		float z = 2.0f * (seed % 10000) / 9999.0f - 1.0f;
		float r = sqrtf(1.0f - z * z);
		float phi = 2.0f * pi * (seed % 10000) / 9999.0f;
		return glm::vec3(r * cosf(phi), r * sinf(phi), z);
	}

	glm::vec3 ApproximateNormalize(const glm::vec3& v) {
		float invLength = glm::inversesqrt(glm::dot(v, v));
		return v * invLength;
	}

	static void GammaCorrect(glm::vec4& color, float gamma) {
		color.r = std::pow(color.r, gamma);
		color.g = std::pow(color.g, gamma);
		color.b = std::pow(color.b, gamma);
	}
}

void Renderer::OnResize(const uint32_t width, const uint32_t height) {
	if (finalImage) {
		if (finalImage->GetWidth() == width && finalImage->GetHeight() == height)
			return;
		finalImage->Resize(width, height);
	}
	else {
		finalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	// Reuse buffers instead of deleting and allocating new memory
	if (imageData) delete[] imageData;
	imageData = new uint32_t[width * height];

	if (accumulationData) delete[] accumulationData;
	accumulationData = new glm::vec4[width * height];

	imageRowIterator.resize(width);
	imageColumnIterator.resize(height);
	for (uint32_t i = 0; i < width; i++) imageRowIterator[i] = i;
	for (uint32_t i = 0; i < height; i++) imageColumnIterator[i] = i;
}

void Renderer::Render(const Scene& scene, const Camera& camera) {
	activeScene = &scene;
	activeCamera = &camera;

	if (frameIndex == 1) {
		memset(accumulationData, 0, finalImage->GetWidth() * finalImage->GetHeight() * sizeof(glm::vec4));
	}

	std::for_each(std::execution::par, imageColumnIterator.begin(), imageColumnIterator.end(),
		[this](uint32_t y) {
			std::for_each(std::execution::par, imageRowIterator.begin(), imageRowIterator.end(),
				[this, y](uint32_t x) {
					glm::vec4 color = PerPixel(x, y);
					if (settings.correctGamma)
						Utils::GammaCorrect(color, settings.gammaCorrection);

					size_t index = y * finalImage->GetWidth() + x;
					accumulationData[index] += color;

					glm::vec4 accumulatedColor = accumulationData[index];
					accumulatedColor /= static_cast<float>(frameIndex);

					accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
					imageData[index] = Utils::ToRGBA(accumulatedColor);
				});
		});

	finalImage->SetData(imageData);

	if (settings.accumulate) {
		frameIndex++;
	}
	else {
		frameIndex = 1;
	}
}

//glm::vec4 Renderer::PerPixel(const uint32_t x, const uint32_t y) {
//	Ray ray(activeCamera->GetPosition(), activeCamera->GetRayDirections()
//		[x + y * finalImage->GetWidth()]);
//
//	glm::vec3 color(0.0f);
//
//	HitRecord record = TraceRay(ray);
//	float hit = record.hitDist;
//
//	if (hit != -1) {
//		// Normalize the normal to be between 0 and 1 for coloring
//		glm::vec3 normalColor = (record.worldNormal + glm::vec3(1.0f)) * 0.5f;
//		color = normalColor;
//	}
//
//	return glm::vec4(color, 1.0f);
//}

glm::vec4 Renderer::PerPixel(const uint32_t x, const uint32_t y) {
	Ray ray(activeCamera->GetPosition(), activeCamera->GetRayDirections()
		[x + y * finalImage->GetWidth()]);

	glm::vec3 light(0.0f);
	glm::vec3 throughput(1.0f);

	uint32_t seed = x + y * finalImage->GetWidth();
	seed *= frameIndex;

	for (int i = 0; i < settings.bounceLimit; i++) {
		seed += i;

		HitRecord record = TraceRay(ray);
		float hit = record.hitDist;

		// Branchless programming for miss check
		bool miss = (hit == -1.0f);
		float missFactor = miss ? 1.0f : 0.0f;
		light += glm::vec3(0.6f, 0.7f, 0.9f) * throughput * missFactor;
		if (miss) break;

		const Material& material = record.objIndex == -1 ? activeScene->materials[0] :
			activeScene->materials[activeScene->objects[record.objIndex]->matIndex];

		// Interpolate and normalize normal
		glm::vec3 interpolatedNormal = glm::normalize(record.worldNormal);

		ray.origin = record.worldPosition + interpolatedNormal * 0.0001f;

		// Calculate reflection vector
		glm::vec3 specular = glm::reflect(ray.direction, interpolatedNormal);

		// Calculate diffuse lighting
		glm::vec3 diffuse = glm::normalize(interpolatedNormal + Utils::RandInUnitSphere(seed));

		bool specularReflection = material.specularProbability >= Utils::RandFloat(seed);

		// Mix diffuse and specular reflections
		ray.direction = glm::mix(diffuse, specular, material.metallicness * specularReflection);

		// Update throughput with material albedo
		throughput *= material.albedo;

		// Add emission if the material is a light source
		if (material.light) light += material.GetEmission() * throughput;
	}
	return glm::vec4(light, 1.0f);
}

HitRecord Renderer::TraceRay(const Ray& ray) {
	int closest = -1;
	float closestDist = std::numeric_limits<float>::max();
	HitRecord record;

	for (size_t i = 0; i < activeScene->objects.size(); i++) {
		const auto& object = activeScene->objects[i];
		if (settings.renderBoundingBoxes) {
			HitRecord bBoxRecord = object->BoundingBox().Intersect(ray, true);
			if (bBoxRecord.hitDist == -1) continue;
			bBoxRecord.objIndex = -1;
			return bBoxRecord;
		}
		else {
			HitRecord bBoxRecord = object->BoundingBox().Intersect(ray, false);
			if (bBoxRecord.hitDist == -1) continue;
		}

		HitRecord tempRecord = object->Intersect(ray);
		if (tempRecord.hitDist > 0.0f && tempRecord.hitDist < closestDist) {
			tempRecord.objIndex = static_cast<int>(i);
			closestDist = tempRecord.hitDist;
			closest = static_cast<int>(i);
			record = tempRecord;
		}
	}

	if (closest < 0) return RayMiss(ray);
	return record;
}

HitRecord Renderer::RayMiss(const Ray& ray) {
	HitRecord record;
	record.hitDist = -1;
	return record;
}