#include "Renderer.h"
#include "Walnut/Random.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <execution>

namespace Utils {
	static uint32_t ToRGBA(const glm::vec4& color) {
		uint8_t r = (uint8_t)(color.r * 255.0f), g = (uint8_t)(color.g * 255.0f),
			b = (uint8_t)(color.b * 255.0f), a = (uint8_t)(color.a * 255.0f);
		return (a << 24) | (b << 16) | (g << 8) | r;
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
	Ray ray(activeCamera->GetPosition(), 
		activeCamera->GetRayDirections()[x + y * finalImage->GetWidth()]);

	glm::vec3 color(0.0f);
	const int bounceCount = 10;
	float multiplier = 1.0f;

	for (int i = 0; i < bounceCount; i++) {
		HitRecord record = TraceRay(ray);
		if (record.hitDist < 0) {
			color += glm::vec3(0.6f, 0.7f, 0.9f) * multiplier;
			break;
			float bgIntensity = 0.5f * (glm::normalize(ray.direction).y +
				1.0f);
			glm::vec3 bgColor(1.0f - bgIntensity * glm::vec3(1.0f) +
				bgIntensity * glm::vec3(0.5f, 0.7f, 1.0f));
			color += bgColor * multiplier;
			multiplier *= 0.5f;
			break;
		}

		const Sphere& sphere = activeScene->spheres[record.objIndex];
		const Material& material = activeScene->materials[sphere.matIndex];
		glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, -1.0f, -1.0f));
		float lightIntensity = glm::max(0.0f, glm::dot(-lightDir, record.worldNormal));
		glm::vec3 hitColor = material.albedo;
		//hitColor = normal * 0.5f + 0.5f; // color by normals
		hitColor *= lightIntensity;
		color += hitColor * multiplier;
		multiplier *= 0.5f;

		ray.origin = record.worldPosition + record.worldNormal * 0.0001f;
		ray.direction = glm::reflect(ray.direction, record.worldNormal + 
			material.roughness * Walnut::Random::Vec3(-0.5f, 0.5f));
	}
	return glm::vec4(color, 1.0f);
}

Renderer::HitRecord Renderer::TraceRay(const Ray& ray) {
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

	for (size_t i = 0; i < activeScene->spheres.size(); i++) {
		const Sphere& sphere = activeScene->spheres[i];
		glm::vec3 origin = ray.origin - sphere.position;

		float a = glm::dot(ray.direction, ray.direction);
		float b = 2.0f * glm::dot(origin, ray.direction);
		float c = glm::dot(origin, origin) - sphere.radius * 
			sphere.radius;

		// disciminant
		// b^2 - 4(a)(c)
		float discriminant = b * b - 4.0f * a * c;
		// ( -b +- sqrt(discriminant) ) / (2a)
		if (discriminant < 0.0f) continue;

		// the first hit is the closest
		float hits[2] = {
				(-b - std::sqrt(discriminant)) / (2.0f * a),
				(-b + std::sqrt(discriminant)) / (2.0f * a)
		};
		if (hits[0] > 0.0f && hits[0] < closestDist) {
			closestDist = hits[0];
			closest = (int)i;
		}
	}

	if (closest < 0) return RayMiss(ray);
	return ClosestHit(ray, closestDist, closest);
}

Renderer::HitRecord Renderer::ClosestHit(const Ray& ray, float hitDist, 
														 int objIndex) {
	HitRecord record;
	record.hitDist = hitDist;
	record.objIndex = objIndex;

	const Sphere& closest = activeScene->spheres[objIndex];

	glm::vec3 origin = ray.origin - closest.position;
	record.worldPosition = origin + ray.direction * hitDist;
	record.worldNormal = glm::normalize(record.worldPosition);

	// this is necessary to reset the position bc we moved it to the origin to 
	// get the normal easier.
	record.worldPosition += closest.position;
	return record;
}

Renderer::HitRecord Renderer::RayMiss(const Ray& ray) {
	HitRecord record;
	record.hitDist = -1;
	return record;
}