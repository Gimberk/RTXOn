#pragma once

#include "../primitives/Mesh.h"

#include <string>
#include <codecvt>

class ModelLoader
{
public:
	static Mesh LoadModel(const char*);
	static std::wstring ExePath();
	static std::string ToString(const std::wstring&);
private:
	static std::string GetModelPath() { return "\\assets\\RAYTRACING_MODELS\\"; }
	static void Split(std::string line, std::vector<std::string>& out);
};