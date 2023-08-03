#pragma once

#include "Resource.h"

class Sampler
{
public:
	Sampler(Graphics& g, HDESC h);
	inline HCPU GetHCPU() const { return m_Handle.m_HCPU; }
	inline HGPU GetHGPU() const { return m_Handle.m_HGPU; }

private:
	HDESC m_Handle;
};