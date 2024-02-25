#pragma once

#include "VecOps.h"

namespace Def
{

	struct BBox
	{
		XMFLOAT3 min{ 0.f, 0.f, 0.f };
		XMFLOAT3 max{ 0.f, 0.f, 0.f };

		BBox() = default;

		BBox(const XMFLOAT3& min, const XMFLOAT3& max)
			:min(std::move(min)), max(std::move(max))
		{
		}

		BBox Union(BBox other)
		{
			return std::move(BBox{ Def::min(min, other.min), Def::max(max, other.max) });
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
}