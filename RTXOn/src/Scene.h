#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Material {
	glm::vec3 albedo{ 1.0f };
	float roughness = 1.0f;
	float metallicness = 0.0f;
};

struct Sphere {
public:
	glm::vec3 position{ 0.0f };
	float radius = 0.5f;

	int matIndex = 0;
};

struct Scene {
	std::vector<Sphere> spheres;
	std::vector<Material> materials;
};