#pragma once

#include <glm/glm.hpp>

#include "Primitive.h"

class Quad : public Primitive {
public:
	Quad(const glm::vec3 Q, glm::vec3 u, glm::vec3 v) : Q(Q), u(u), v(v) {
        type = PrimitiveType::QUAD;

		normal = glm::normalize(cross(u, v));
		D = glm::dot(normal, Q);

		SetBoundingBox();
	}

	virtual void SetBoundingBox() {
		AABB diagonalA = AABB(Q, Q + u + v);
		AABB diagonalB = AABB(Q + u, Q + v);
		boundingBox = AABB(diagonalA, diagonalB);
	}

	// meh
    HitRecord Intersect(const Ray& ray) const override {
        float denominator = glm::dot(normal, ray.direction);

        HitRecord record;
        record.hitDist = -1;

        // No hit if the ray is parallel to the plane.
        if (fabs(denominator) < 1e-8) return record;

        // Calculate the parameter t of the intersection point.
        auto t = (glm::dot(normal, Q) - glm::dot(normal, ray.origin)) / denominator;
        if (t < 0) return record; // Ensure t is within the valid range.

        // Calculate the intersection point.
        auto intersection = ray.origin + t * ray.direction;

        // Calculate the local coordinates of the intersection point.
        glm::vec3 d = intersection - Q;
        float du = glm::dot(d, u);
        float dv = glm::dot(d, v);

        // Check if the intersection point is within the quad.
        if (du < 0 || du > glm::dot(u, u) || dv < 0 || dv > glm::dot(v, v)) {
            return record; // Intersection point is outside the quad.
        }

        // Set the hit record with intersection details.
        record.hitDist = t;
        record.worldPosition = intersection;
        record.worldNormal = normal;

        return record;
    }

    void OnRender() override {

    }

	AABB BoundingBox() const override { return boundingBox; }
private:
	glm::vec3 Q, u, v;
	float D;

	AABB boundingBox;
	glm::vec3 normal;
};