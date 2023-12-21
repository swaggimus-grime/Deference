#pragma once

#include "VecOps.h"

using namespace VecOps;

struct BBox
{
	XMFLOAT3 min;
	XMFLOAT3 max;
	XMFLOAT3 dim;

	BBox() = default;

	BBox(const XMFLOAT3& min, const XMFLOAT3& max)
		:min(std::move(min)), max(std::move(max))
	{
	}

	BBox Union(BBox other)
	{
		return std::move(BBox{ VecOps::min(min, other.min), VecOps::max(max, other.max) });
	}

	inline XMFLOAT3 Dim()
	{
		return std::move(XMFLOAT3{
			abs(max.x - min.x),
			abs(max.y - min.y),
			abs(max.z - min.z)
		});
	}

	inline float DiagonalLength()
	{
		return length(max - min);
	}
};