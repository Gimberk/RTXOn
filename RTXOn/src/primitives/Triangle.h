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

        Initialize();
	}

    void SetNormals(const glm::vec3* normals) {
        normalA = normals[0]; normalB = normals[1]; normalC = normals[2];
    }

    glm::vec3* GetNormals() const {
        glm::vec3 normals[3] = {
            normalA, normalB, normalC
        };
        return normals;
    }

    void Initialize() {
        glm::vec3 min = posA;
        glm::vec3 max = posA;

        min = glm::min(min, posB);
        max = glm::max(max, posB);

        min = glm::min(min, posC);
        max = glm::max(max, posC);

        boundingBox = AABB(min, max);
    }

	// stupid math stuff like cmon man! (Moller-Trumbore algorithm):
	//         <3
	// I'm so stupid, so please, don't be like me. When I create the triangle,
	// I lined all the points on the x-axis, and was confused as to why 
	// I didn't see a triangle for 20+ minutes... I'm going insane.
    HitRecord Intersect(const Ray& ray) const {
        HitRecord record{};

        glm::vec3 edge1 = posB - posA;
        glm::vec3 edge2 = posC - posA;
        glm::vec3 h = glm::cross(ray.direction, edge2);
        float a = glm::dot(edge1, h);

        if (fabs(a) < 1e-8)
            return record; // This means that there is no intersection

        float f = 1.0f / a;
        glm::vec3 s = ray.origin - posA;
        float u = f * glm::dot(s, h);

        if (u < 0.0 || u > 1.0)
            return record;

        glm::vec3 q = glm::cross(s, edge1);
        float v = f * glm::dot(ray.direction, q);

        if (v < 0.0 || u + v > 1.0)
            return record;

        float t = f * glm::dot(edge2, q);

        if (t > 1e-8) {
            record.hitDist = t;
            record.worldPosition = ray.origin + t * ray.direction;

            // Compute barycentric coordinates
            float w = 1.0f - u - v;

            // Interpolate normal using barycentric coordinates
            record.worldNormal = glm::normalize(w * normalA + u * normalB + v * normalC);

            return record;
        }
        else {
            return record; // This means that there is a line intersection but not a ray intersection
        }
    }

    void OnRender() override {

    }

    AABB BoundingBox() const override { return boundingBox; }
private:
	glm::vec3 normalA, normalB, normalC;
    AABB boundingBox;
};