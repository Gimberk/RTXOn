#pragma once
#include "Vector3.h";

class Ray {
public:
	Ray(Vector3 origin, Vector3 direction) : origin(origin), direction(direction) {}

	Vector3 Origin() const { return origin; }
	Vector3 Direction() const { return direction; }
private:
	Vector3 direction, origin;
};