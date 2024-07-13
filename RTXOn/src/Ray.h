#pragma once

#include "glm/glm.hpp"

class Ray {
public:
	Ray(glm::vec3 origin, glm::vec3 direction) : origin(origin), direction(direction) {}
	Ray(glm::vec3 origin) : origin(origin), direction(glm::vec3(0.0f)) {}

	glm::vec3 at(const float t) const {
		// based on the ray formula R(t) = o + dt
		return origin + direction * t;
	}

	glm::vec3 origin, direction;
};