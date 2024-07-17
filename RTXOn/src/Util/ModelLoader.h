#pragma once

#include "../primitives/Mesh.h"

#include <string>
#include <codecvt>

class ModelLoader
{
public:
	static Mesh LoadModel(const char*);
private:
	static std::string GetModelPath() { return "\\assets\\RAYTRACING_MODELS\\"; }
	static std::wstring ExePath();
	static std::string ToString(const std::wstring&);
	static void Split(std::string line, std::vector<std::string>& out);
};