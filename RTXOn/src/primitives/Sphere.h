#pragma once

#include "Primitive.h"

class Sphere : public Primitive {
public:
	glm::vec3 position{ 0.0f };
	float radius = 0.5f;
public:
	Sphere() {
		Initialize();
	}

	Sphere(glm::vec3 position, float radius) : radius(radius), position(position) {
		Initialize();
	}

	HitRecord Intersect(const Ray& ray) const override {
		glm::vec3 origin = ray.origin - position;

		float a = glm::dot(ray.direction, ray.direction);
		float b = 2.0f * glm::dot(origin, ray.direction);
		float c = glm::dot(origin, origin) - radius *
			radius;

		// disciminant
		// b^2 - 4(a)(c)
		float discriminant = b * b - 4.0f * a * c;
		// ( -b +- sqrt(discriminant) ) / (2a)
		if (discriminant < 0.0f) {
			HitRecord record;
			record.hitDist = -1;
			return record;
		}

		// the first hit is the closest
		float hits[2] = {
				(-b - std::sqrt(discriminant)) / (2.0f * a),
				(-b + std::sqrt(discriminant)) / (2.0f * a)
		};

		HitRecord record;
		record.hitDist = hits[0];

		glm::vec3 hitOrigin = ray.origin - position;
		record.worldPosition = hitOrigin + ray.direction * record.hitDist;
		record.worldNormal = glm::normalize(record.worldPosition);

		// this is necessary to reset the position bc we moved it to the origin to 
		// get the normal easier.
		record.worldPosition += position;

		float* uv = GetSphereUV(record.worldPosition);
		record.u = uv[0]; record.v = uv[1];

		return record;
	}

	void OnRender() override {
		Initialize();
	}

	AABB BoundingBox() const override { return boundingBox; }

	static float* GetSphereUV(const glm::vec3 hitPoint) {
		float pi = 3.14159265359;

		float theta = acos(-hitPoint.y);
		float phi = atan2(-hitPoint.z, hitPoint.x) + pi;
		
		float uv[2] = {
			phi / (2*pi),
			theta / pi
		};

		return uv;
	}
private:
	void Initialize() {
		type = PrimitiveType::SPHERE;

		glm::vec3 radiusVector(radius);
		boundingBox = AABB(position - radiusVector, position + radiusVector);
	}
private:
	AABB boundingBox;
};