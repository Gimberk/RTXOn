#include "Renderer.h"
#include "Walnut/Random.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glm/gtc/random.hpp>

#include <execution>

namespace Utils {
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

	static void GammaCorrect(glm::vec4& color, float gamma) {
		color.r = std::pow(color.r, gamma);
		color.g = std::pow(color.g, gamma);
		color.b = std::pow(color.b, gamma);
	}
}

void Renderer::OnResize(const uint32_t width, const uint32_t height)
{
	if (finalImage) {
		if (finalImage->GetWidth() == width && finalImage->GetHeight() ==
			height) return;
		finalImage->Resize(width, height);
	}
	else finalImage = std::make_shared<Walnut::Image>(width, height,
		Walnut::ImageFormat::RGBA);

	delete[] imageData;
	imageData = new uint32_t[width * height];

	delete[] accumulationData;
	accumulationData = new glm::vec4[width * height];

	imageRowIterator.resize(width);
	imageColumnIterator.resize(height);
	for (uint32_t i = 0; i < width; i++) imageRowIterator[i] = i;
	for (uint32_t i = 0; i < height; i++) imageColumnIterator[i] = i;
}

void Renderer::Render(const Scene& scene, const Camera& camera) {
	activeScene = &scene;
	activeCamera = &camera;

	if (frameIndex == 1)
		memset(accumulationData, 0, finalImage->GetWidth()
			* finalImage->GetHeight() * sizeof(glm::vec4));

#if 1
	std::for_each(std::execution::par, imageColumnIterator.begin(),
		imageColumnIterator.end(),
		[this](uint32_t y) {
			std::for_each(std::execution::par, imageRowIterator.begin(),
				imageRowIterator.end(),
				[this, y](uint32_t x) {
					glm::vec4 color = PerPixel(x, y);
					if (settings.correctGamma)
						Utils::GammaCorrect(color, settings.gammaCorrection);

					accumulationData[y * finalImage->GetWidth() + x] += color;

					glm::vec4 accumulatedColor = accumulationData[y * finalImage->
						GetWidth() + x];
					accumulatedColor /= (float)frameIndex;

					accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f),
						glm::vec4(1.0f));
					imageData[y * finalImage->GetWidth() + x] =
						Utils::ToRGBA(accumulatedColor);
				});
		});
#else
	for (uint32_t y = 0; y < finalImage->GetHeight(); y++) {
		for (uint32_t x = 0; x < finalImage->GetWidth(); x++) {
			glm::vec4 color = PerPixel(x, y);
			accumulationData[y * finalImage->GetWidth() + x] += color;

			glm::vec4 accumulatedColor = accumulationData[y * finalImage->GetWidth() + x];
			accumulatedColor /= (float)frameIndex;

			accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f),
				glm::vec4(1.0f));
			imageData[y * finalImage->GetWidth() + x] = Utils::ToRGBA(accumulatedColor);
		}
	}
#endif
	finalImage->SetData(imageData);

	if (settings.accumulate) frameIndex++;
	else if (frameIndex != 1) frameIndex = 1;
}

glm::vec4 Renderer::PerPixel(const uint32_t x, const uint32_t y) {
	Ray ray(activeCamera->GetPosition(), activeCamera->GetRayDirections()
		[x + y * finalImage->GetWidth()]);

	glm::vec3 light(0.0f);
	const int bounceCount = 10;
	glm::vec3 throughput(1.0f);

	uint32_t seed = x + y * finalImage->GetWidth();
	seed *= frameIndex;

	for (int i = 0; i < bounceCount; i++) {
		seed += i;

		HitRecord record = TraceRay(ray);
		if (record.hitDist < 0) {
			light += glm::vec3(0.6f, 0.7f, 0.9f) * throughput;
			break;
		}

		const Material& material = record.objIndex == -1 ? activeScene->materials[0] :
			activeScene->materials[activeScene->objects[record.objIndex]->matIndex];

		ray.origin = record.worldPosition + record.worldNormal * 0.0001f;

		glm::vec3 specular = glm::reflect(ray.direction, record.worldNormal);
		glm::vec3 diffuse = glm::normalize(record.worldNormal + glm::sphericalRand(1.0f));

		bool specularReflection = material.specularProbability >= Utils::RandFloat(seed);

		ray.direction = glm::mix(diffuse, specular, material.metallicness *
			specularReflection);

		throughput *= material.albedo;
		if (material.light) light += material.GetEmission() * throughput;
	}
	return glm::vec4(light, 1.0f);
}


HitRecord Renderer::TraceRay(const Ray& ray) {
	// -----------      Math      -----------
	// R(t) = a + bt, P(x, y) = x^2 + y^2 - r^2 = 0
	// Quadratic Equation:
	// -	(bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	// -	a = origin of ray, b = direction of ray, r = raidus of circle,
	// -	t = hit distance along ray

	// With a = (-3, -3), b = (1, 1), r = 2
	// -	(1^2 + 1^2)t^2 + (2(-3)(1) + 2(-3)(1))t + ((-3)^2 + (-3)^2 - 
	//															(2)^2) = 0
	// -	(1 + 1)t^2 + (-6 - 6)t + (9 + 9 - 4) = 0
	// -	2t^2 -12t + 14 = 0
	// -	t^2 - 6t + 7 = 0
	// ------------------------
	// -	(-b +- sqrt(b^2 - 4(a)(c))) / 2a
	// -	(6 +- sqrt(36 - 4(1)(7))) / 2(1)
	// -	(6 +- sqrt(36 - 28)) / 2
	// -	(6 +- sqrt(8)) / 2
	//		-	Discriminant: 8
	//		-	8 = positive real = 2 real solutions
	// ------------------------
	// Answer: t = (6 +- sqrt(8)) / 2


	// ----------- Implementation -----------

	int closest = -1;
	float closestDist = std::numeric_limits<float>::max();

	HitRecord record;

	for (size_t i = 0; i < activeScene->objects.size(); i++) {
		if (activeScene->objects[i]->GetType() == PrimitiveType::SPHERE) {
			HitRecord bBoxRecord =
				activeScene->objects[i]->BoundingBox().Intersect(ray, false);
			if (bBoxRecord.hitDist == -1) continue;
		}
		HitRecord tempRecord = activeScene->objects[i]->Intersect(ray);
		tempRecord.objIndex = (int)i;
		if (tempRecord.hitDist > 0.0f && tempRecord.hitDist < closestDist) {
			closestDist = tempRecord.hitDist;
			closest = (int)i;
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