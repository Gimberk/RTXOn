#pragma once

#include "../Util/Ray.h"
#include "../Util/HitRecord.h"
#include "../Util/AABB.h"

#include <iostream>

enum class PrimitiveType {
	SPHERE = 0, TRIANGLE = 1, QUAD = 2
};

class Primitive {
public:
	PrimitiveType type;
	int matIndex;
public:
	virtual ~Primitive() = default;
	virtual void OnRender() = 0;

	virtual HitRecord Intersect(const Ray& ray) 
		const = 0;
	virtual AABB BoundingBox() const = 0;

	PrimitiveType GetType() const { return type; }
};