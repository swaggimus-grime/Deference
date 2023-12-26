#pragma once

#include <d3d12.h>

struct HRES
{
	HCPU hcpu;
	HGPU hgpu;

	HRES()
		:hcpu({ 0 }), hgpu({ 0 })
	{}

	HRES(HCPU hc, HGPU hg = { 0 })
		:hcpu(hc), hgpu(hg)
	{}

	inline void operator+=(UINT offset)
	{
		hcpu.ptr += offset;
		hgpu.ptr += offset;
	}
};