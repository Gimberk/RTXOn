#include "Renderer.h"
#include "Walnut/Random.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <iostream>
#include <iomanip>

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
		if (finalImage->GetWidth() == width && finalImage->GetHeight() == height) return;
		finalImage->Resize(width, height);
	}
	else finalImage = std::make_shared<Walnut::Image>(width, height, 
		Walnut::ImageFormat::RGBA);

	delete[] imageData;
	imageData = new uint32_t[width * height];
}

void Renderer::Render(const Camera& camera) {
	float radius = 0.5f;
	float radiusSquared = radius * radius;
	const glm::vec3& rayOrigin = camera.GetPosition();

	Ray ray(rayOrigin);

	for (uint32_t y = 0; y < finalImage->GetHeight(); y++) {
		for (uint32_t x = 0; x < finalImage->GetWidth(); x++) {
			const glm::vec3& rayDirection = 
				camera.GetRayDirections()[x + y * finalImage->GetWidth()];
			ray.direction = rayDirection;

			glm::vec4 color = TraceRay(ray, radiusSquared);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			imageData[y * finalImage->GetWidth() + x] = Utils::ToRGBA(color);
		}
	}
	finalImage->SetData(imageData);
}

// Returns color
glm::vec4 Renderer::TraceRay(const Ray& ray, const float radiusSquared) {
	// -----------      Math      -----------
	// R(t) = a + bt, P(x, y) = x^2 + y^2 - r^2 = 0
	// Quadratic Equation:
	// -	(bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	// -	a = origin of ray, b = direction of ray, r = raidus of circle,
	// -	t = hit distance along ray

	// With a = (-3, -3), b = (1, 1), r = 2
	// -	(1^2 + 1^2)t^2 + (2(-3)(1) + 2(-3)(1))t + ((-3)^2 + (-3)^2 - (2)^2) = 0
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

	float a = glm::dot(ray.direction, ray.direction);
	float b = 2.0f * glm::dot(ray.origin, ray.direction);
	float c = glm::dot(ray.origin, ray.origin) - radiusSquared;

	// disciminant
	// b^2 - 4(a)(c)
	float discriminant = b * b - 4.0f * a * c;
	// ( -b +- sqrt(discriminant) ) / (2a)
	if (discriminant < 0.0f) return glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	glm::vec3 hits[2] = {
			ray.origin + ray.direction * ((-b + std::sqrt(discriminant)) / (2.0f * a)),
			ray.origin + ray.direction * ((-b - std::sqrt(discriminant)) / (2.0f * a))
	};

	glm::vec3 normal = glm::normalize(hits[1] - glm::vec3(0.0, 0.0, 0.0)); // sphere origin
	glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, -1.0f, -1.0f));

	float lightIntensity = glm::max(0.0f, glm::dot(-lightDir, normal));
	glm::vec3 hitColor(1.0f, 0.3f, 0.0f);
	//hitColor = normal * 0.5f + 0.5f; // color by normals
	hitColor *= lightIntensity;
	return glm::vec4(hitColor, 1.0f);
}