#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "primitives/Primitive.h"

struct Material {
	glm::vec3 albedo{ 1.0f };
	glm::vec3 emissionColor{ 0.0f };
	glm::vec3 specularColor{ 1.0f };

	float metallicness = 0.0f;
	float specularProbability = 1.0f;

	bool light = false;
	float emissionPower = 0.0f;

	glm::vec3 GetEmission() const { return emissionColor * emissionPower; }
};

struct Scene {
	std::vector<std::shared_ptr<Primitive>> objects;
	std::vector<Material> materials;
};