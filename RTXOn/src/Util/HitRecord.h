#pragma once

#include <glm/glm.hpp>

struct HitRecord {
	float hitDist;
	glm::vec3 worldNormal, worldPosition;

	int objIndex;
};