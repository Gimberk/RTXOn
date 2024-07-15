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


	// ugh, this was the worst to write out. ihml
	HitRecord Intersect(const Ray& ray, bool colored) const {
		Interval tx(
			(x.min - ray.origin.x) / ray.direction.x, 
			(x.max - ray.origin.x) / ray.direction.x
		);
		if (tx.min > tx.max) {
			float temp = tx.min;
			tx.min = tx.max;
			tx.max = temp;
		}

		Interval ty(
			(y.min - ray.origin.y) / ray.direction.y,
			(y.max - ray.origin.y) / ray.direction.y
		);
		if (ty.min > ty.max) {
			float temp = ty.min;
			ty.min = ty.max;
			ty.max = temp;
		}

		HitRecord record;
		record.hitDist = -1;
		if ((tx.min > ty.max) || (ty.min > tx.max)) return record;

		if (ty.min > tx.min) tx.min = ty.min;
		if (ty.max < tx.max) tx.max = ty.max;

		Interval tz(
			(z.min - ray.origin.z) / ray.direction.z,
			(z.max - ray.origin.z) / ray.direction.z
		);

		if (tz.min > tz.max) {
			float temp = tz.min;
			tz.min = tz.max;
			tz.max = temp;
		}

		if ((tx.min > tz.max) || (tz.min > tx.max)) return record;

		if (tz.min > tx.min) tx.min = tz.min;
		if (tz.max < tx.max) tx.max = tz.max;

		// At this point, tmin is the entrance to the AABB and tmax is the exit
		if (tx.min < 0) {  // Ray origin is inside the AABB
			tx.min = tx.max;  // Set entrance to exit
			if (tx.min < 0)  // If tmax is also negative, no intersection
				return record;
		}

		if (!colored) {
			record.hitDist = 0;
			return record;
		}

		glm::vec3 hitPoint = ray.at(tx.min);
		record.hitDist = tx.min;
		record.worldPosition = hitPoint;

		// Determine normal based on which face was hit
		if (hitPoint.x <= x.min + 1e-4) record.worldNormal = glm::vec3(-1, 0, 0);
		else if (hitPoint.x >= x.max - 1e-4) record.worldNormal = glm::vec3(1, 0, 0);
		else if (hitPoint.y <= y.min + 1e-4) record.worldNormal = glm::vec3(0, -1, 0);
		else if (hitPoint.y >= y.max - 1e-4) record.worldNormal = glm::vec3(0, 1, 0);
		else if (hitPoint.z <= z.min + 1e-4) record.worldNormal = glm::vec3(0, 0, -1);
		else if (hitPoint.z >= z.max - 1e-4) record.worldNormal = glm::vec3(0, 0, 1);
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