#include "ModelLoader.h"

#include "../primitives/Triangle.h"

#include <fstream>
#include <windows.h>

#include "ModelLoader.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include <glm/glm.hpp>

struct Face {
    glm::ivec3 vertexIndices;
    glm::ivec3 normalIndices;
};

Mesh ModelLoader::LoadModel(const char* fn) {
    std::string path = ToString(ExePath()).append(GetModelPath().append(fn));
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file at " << path << std::endl;
        return Mesh();
    }

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<Face> faces;
    std::string line;

    while (std::getline(file, line)) {
        if (line.size() <= 1) continue;
        if (line.substr(0, 2) == "v ") {
            std::istringstream s(line.substr(2));
            glm::vec3 v;
            s >> v.x; s >> v.y; s >> v.z;
            vertices.push_back(v);
        }
        else if (line.substr(0, 3) == "vn ") {
            std::istringstream s(line.substr(3));
            glm::vec3 n;
            s >> n.x; s >> n.y; s >> n.z;
            normals.push_back(n);
        }
        else if (line.substr(0, 2) == "f ") {
            std::istringstream s(line.substr(2));
            Face face;
            char dummy;
            for (int i = 0; i < 3; ++i) {
                s >> face.vertexIndices[i] >> dummy >> dummy >> face.normalIndices[i];
                // OBJ indices are 1-based, convert them to 0-based
                face.vertexIndices[i]--;
                face.normalIndices[i]--;
            }
            faces.push_back(face);
        }
    }

    std::vector<Triangle> tris;
    for (const Face& face : faces) {
        glm::vec3 vertexA = vertices[face.vertexIndices[0]];
        glm::vec3 vertexB = vertices[face.vertexIndices[1]];
        glm::vec3 vertexC = vertices[face.vertexIndices[2]];

        glm::vec3 normalA = normals[face.normalIndices[0]];
        glm::vec3 normalB = normals[face.normalIndices[1]];
        glm::vec3 normalC = normals[face.normalIndices[2]];

        Triangle tri(vertexA, vertexB, vertexC);
        glm::vec3 triangleNormals[3] = { normalA, normalB, normalC };
        tri.SetNormals(triangleNormals);
        tri.matIndex = 0; // Assuming a single material index for now
        tris.push_back(tri);
    }

    file.close();
    return Mesh(tris);
}


void ModelLoader::Split(std::string line, std::vector<std::string>& out) {
	std::string buffer;
	for (int i = 0; i < line.size(); i++) {
		const char c = line[i];
		if (c == '\n' || c == ' ') {
			out.push_back(buffer);
			buffer = "";
		}
		else buffer += c;
	}

	out.push_back(buffer);
}

std::wstring ModelLoader::ExePath() {
	TCHAR buffer[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
	return std::wstring(buffer).substr(0, pos);
}

std::string ModelLoader::ToString(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;
	return converterX.to_bytes(wstr);
}