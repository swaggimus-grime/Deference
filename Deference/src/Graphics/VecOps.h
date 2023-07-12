#pragma once

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

static bool operator!=(XMFLOAT3& v1, const XMFLOAT3& v2)
{
	return !((v1.x == v2.x) && (v1.y == v2.y) && (v1.z == v2.z));
}