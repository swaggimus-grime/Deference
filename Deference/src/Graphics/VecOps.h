#pragma once

static void operator+=(XMFLOAT3& v1, const XMFLOAT3& v2)
{
	v1.x += v2.x;
	v1.y += v2.y;
	v1.z += v2.z;
}