#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Sphere {
public:
	glm::vec3 position{ 0.0f };
	float radius = 0.5f;

	glm::vec3 albedo{ 1.0f };
};

struct Scene {
	std::vector<Sphere> spheres;
};