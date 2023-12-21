#pragma once

namespace VecOps
{
	static float length(const XMFLOAT3& v)
	{
		return sqrt(pow(v.x, 2) + pow(v.y, 2) + pow(v.z, 2));
	}

	static XMFLOAT3 min(const XMFLOAT3& v1, const XMFLOAT3& v2)
	{
		return length(v1) <= length(v2) ? v1 : v2;
	}

	static XMFLOAT3 max(const XMFLOAT3& v1, const XMFLOAT3& v2)
	{
		return length(v1) > length(v2) ? v1 : v2;
	}

	static void operator+=(XMFLOAT3& v1, const XMFLOAT3& v2)
	{
		v1.x += v2.x;
		v1.y += v2.y;
		v1.z += v2.z;
	}

	static XMFLOAT3 operator*(const XMFLOAT3& v, float s)
	{
		return std::move(XMFLOAT3(v.x * s, v.y * s, v.z * s));
	}

	static XMFLOAT3 operator/(const XMFLOAT3& v, float s)
	{
		return std::move(XMFLOAT3(v.x / s, v.y / s, v.z / s));
	}

	static bool operator!=(XMFLOAT3& v1, const XMFLOAT3& v2)
	{
		return !((v1.x == v2.x) && (v1.y == v2.y) && (v1.z == v2.z));
	}

	static XMFLOAT3 operator+(const XMFLOAT3& v1, const XMFLOAT3& v2)
	{
		return std::move(XMFLOAT3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z));
	}

	static XMFLOAT3 operator-(const XMFLOAT3& v1, const XMFLOAT3& v2)
	{
		return std::move(XMFLOAT3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z));
	}
}