#pragma once

#include "Resource.h"

class Sampler : public Resource
{
public:
	Sampler(Graphics& g, const D3D12_CPU_DESCRIPTOR_HANDLE& handle);
};