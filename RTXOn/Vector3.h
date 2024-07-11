#pragma once

class Vector3 {
public:
	Vector3(int x, int y, int z) : x(x), y(y), z(z) {}

	int X() const { return x; }
	int Y() const { return y; }
	int Z() const { return z; }
private:
	int x, y, z;
};