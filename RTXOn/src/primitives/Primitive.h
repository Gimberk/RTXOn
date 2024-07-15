#pragma once

#include "../Ray.h"
#include "../HitRecord.h"

#include <iostream>

enum class PrimitiveType {
	SPHERE = 0, TRIANGLE = 1
};

class Primitive {
public:
	PrimitiveType type;
	int matIndex;
public:
	virtual ~Primitive() = default;
	virtual HitRecord Intersect(const Ray& ray) 
		const = 0;
	PrimitiveType GetType() const { return type; }
};