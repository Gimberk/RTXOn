#pragma once

#include <vector>

#include "Primitive.h"
#include "Triangle.h"

// OMG these things were a pain! Just getting the vertices was fine, and I got the model
// up and rendering FIRST TRY! But then I noticed, the reflections, and the shadows seemed...
// discontinuous. This made the model look really bad. Thus began a back a forth with the model loader
// to find out the issue. And after coloring it by its normals, I noticed the normals were all messed up.
// And then after figuring out the error with the OBJ parsing... it was over. FINALLY!
class Mesh : public Primitive {
public:
	Mesh(const std::vector<Triangle, std::allocator<Triangle>>& tris) {
		type = PrimitiveType::MESH;
		objectIndex = 0;

		this->tris = tris;

		boundingBox = this->tris[0].BoundingBox();
		for (int i = 1; i < this->tris.size(); i++) {
			Triangle& tri = this->tris[i];
			boundingBox = AABB(boundingBox, tri.BoundingBox());
		}

		std::cout << "Created mesh with " << tris.size() << " triangles!" 
			<< " Don't forget to update the object index!" << '\n';
	}

	Mesh() {
		std::cerr << "Mesh Failed To Generate!" << '\n';
		type = PrimitiveType::TRIANGLE;
	}

	HitRecord Intersect(const Ray& ray) const override {
		HitRecord record;

		int closest = -1;
		float closestDist = std::numeric_limits<float>::max();
		for (int i = 0; i < tris.size(); i++) {
			Triangle tri = tris[i];
			HitRecord triRecord = tri.Intersect(ray);
			if (triRecord.hitDist > 0.0f && triRecord.hitDist < closestDist) {
				closestDist = triRecord.hitDist;
				closest = static_cast<int>(i);
				record = triRecord;
			}
		}

		if (closest < 0) record.hitDist = -1;
		record.objIndex = objectIndex;
		return record;
	}


	void SetObjectIndex(const int index) { objectIndex = index; }

	void OnRender() override {

	}

	AABB BoundingBox() const override { return boundingBox; }
private:
	AABB boundingBox;

	int objectIndex;
	std::vector<Triangle> tris;
};