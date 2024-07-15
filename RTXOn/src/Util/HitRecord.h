#pragma once

#include <glm/glm.hpp>

struct HitRecord {
	float hitDist;
	glm::vec3 worldNormal, worldPosition;

	float u, v; // for texture mapping

	int objIndex;
};