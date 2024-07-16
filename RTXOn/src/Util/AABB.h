#pragma once

#include <glm/glm.hpp>
#include<limits>

#include "Util/HitRecord.h"
#include "Util/Ray.h"

struct Interval {
	float min, max;

	Interval() : min(std::numeric_limits<float>::max()),
				 max(std::numeric_limits<float>::min()) {}

	Interval(float min, float max) : min(min), max(max) {}

	Interval(const Interval a, const Interval b) {
		min = fmin(a.min, b.min);
		max = fmax(a.max, b.max);
	}

	float size() { return min + max; }

	void expand(float pad) {
		float delta = pad / 2;
		min -= delta;
		max += delta;
	}
};

class AABB {
public:
	Interval x, y, z;
public:
	AABB() {
		x = Interval(std::numeric_limits<float>::min(), std::numeric_limits<float>::max());
		y = Interval(x.min, x.max);
		z = Interval(y.min, y.max);
	}

	AABB(const glm::vec3 a, const glm::vec3 b) {
		x = (a.x <= b.x) ? Interval(a.x, b.x) : Interval(b.x, a.x);
		y = (a.y <= b.y) ? Interval(a.y, b.y) : Interval(b.y, a.y);
		z = (a.z <= b.z) ? Interval(a.z, b.z) : Interval(b.z, a.z);

		pad();
	}

	AABB(const AABB a, const AABB b) {
		x = Interval(a.x, b.x);
		y = Interval(a.y, b.y);
		z = Interval(a.z, b.z);
	}

	AABB(const Interval x, const Interval y, const Interval z) : x(x), y(y), z(z) {
		pad();
	}


	HitRecord Intersect(const Ray& ray, bool colored) const {
		float tMin = (x.min - ray.origin.x) / ray.direction.x;
		float tMax = (x.max - ray.origin.x) / ray.direction.x;
		if (tMin > tMax) std::swap(tMin, tMax);

		float tyMin = (y.min - ray.origin.y) / ray.direction.y;
		float tyMax = (y.max - ray.origin.y) / ray.direction.y;
		if (tyMin > tyMax) std::swap(tyMin, tyMax);

		HitRecord record;
		record.hitDist = -1;
		if ((tMin > tyMax) || (tyMin > tMax)) return record;

		if (tyMin > tMin) tMin = tyMin;
		if (tyMax < tMax) tMax = tyMax;

		float tzMin = (z.min - ray.origin.z) / ray.direction.z;
		float tzMax = (z.max - ray.origin.z) / ray.direction.z;
		if (tzMin > tzMax) std::swap(tzMin, tzMax);

		if ((tMin > tzMax) || (tzMin > tMax)) return record;

		if (tzMin > tMin) tMin = tzMin;
		if (tzMax < tMax) tMax = tzMax;

		if (tMin < 0) {
			tMin = tMax;
			if (tMin < 0) return record;
		}

		if (!colored) {
			record.hitDist = 0;
			return record;
		}

		glm::vec3 hitPoint = ray.at(tMin);
		record.hitDist = tMin;
		record.worldPosition = hitPoint;

		// Determine normal based on which face was hit
		const float epsilon = 1e-4f;
		if (fabs(hitPoint.x - x.min) < epsilon) record.worldNormal = glm::vec3(-1, 0, 0);
		else if (fabs(hitPoint.x - x.max) < epsilon) record.worldNormal = glm::vec3(1, 0, 0);
		else if (fabs(hitPoint.y - y.min) < epsilon) record.worldNormal = glm::vec3(0, -1, 0);
		else if (fabs(hitPoint.y - y.max) < epsilon) record.worldNormal = glm::vec3(0, 1, 0);
		else if (fabs(hitPoint.z - z.min) < epsilon) record.worldNormal = glm::vec3(0, 0, -1);
		else if (fabs(hitPoint.z - z.max) < epsilon) record.worldNormal = glm::vec3(0, 0, 1);

		return record;
	}

private:
	void pad() {
		float min = 0.0001;
		if (x.size() < min) x.expand(min);
		if (y.size() < min) y.expand(min);
		if (z.size() < min) z.expand(min);
	}
};