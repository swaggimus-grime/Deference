#pragma once

#include <dxgi.h>

namespace Def
{
	static SIZE_T DXGI_FORMAT_Sizeof(DXGI_FORMAT fmt)
	{
		switch (fmt)
		{
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			return 4 * sizeof(FLOAT);
		case DXGI_FORMAT_R32G32B32_FLOAT:
			return 3 * sizeof(FLOAT);
		case DXGI_FORMAT_R32G32_FLOAT:
			return 2 * sizeof(FLOAT);
		case DXGI_FORMAT_R32_FLOAT:
			return 1 * sizeof(FLOAT);
		};

		return 0;
	}

	static DXGI_FORMAT DXGI_FORMAT_ConvertToTypeless(DXGI_FORMAT fmt)
	{
		switch (fmt)
		{
		case DXGI_FORMAT_R32_UINT:
			return DXGI_FORMAT_R32_TYPELESS;
		case DXGI_FORMAT_R16_UINT:
			return DXGI_FORMAT_R16_TYPELESS;
		}
	}
}