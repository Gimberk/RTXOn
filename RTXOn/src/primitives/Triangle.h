#pragma once

#include "Primitive.h"

class Triangle : public Primitive {
public:
	glm::vec3 posA, posB, posC;
public:
	Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c) : posA(a), posB(b), posC(c) {
		type = PrimitiveType::TRIANGLE;

		normalA = glm::normalize(a);
		normalB = glm::normalize(b);
		normalC = glm::normalize(c);

        boundingBox = AABB();
	}

	// stupid math stuff like cmon man! (Moller-Trumbore algorithm):
	//         <3
	// I'm so stupid, so please, don't be like me. When I create the triangle,
	// I lined all the points on the x-axis, and was confused as to why 
	// I didn't see a triangle for 20+ minutes... I'm going insane.
    HitRecord Intersect(const Ray& ray) const override {
        glm::vec3 abEdge = posB - posA;
        glm::vec3 acEdge = posC - posA;
        glm::vec3 normal = glm::cross(abEdge, acEdge);
        glm::vec3 oa = ray.origin - posA;
        glm::vec3 doa = glm::cross(oa, ray.direction);

        float determinant = -glm::dot(ray.direction, normal);
        float inverseDeterminant = 1 / determinant;

        float distance = glm::dot(oa, normal) * inverseDeterminant;
        float u = glm::dot(acEdge, doa) * inverseDeterminant;
        float v = -glm::dot(abEdge, doa) * inverseDeterminant;
        float w = 1 - u - v;

        HitRecord record;
        if (!(determinant >= 1E-6 && distance >= 0 && u >= 0 && v >= 0 && w >= 0)) {
            record.hitDist = -1;
            return record;
        }

        record.hitDist = distance;
        record.worldPosition = ray.origin + ray.direction * record.hitDist;
        record.worldNormal = glm::normalize(normal);
        return record;
    }

    void OnRender() override {

    }

    AABB BoundingBox() const override { return boundingBox; }
private:
	glm::vec3 normalA, normalB, normalC;
    AABB boundingBox;
};