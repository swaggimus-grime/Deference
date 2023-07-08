#pragma once

#include "Resource.h"
#include <d3d12.h>

class Graphics;

class UnorderedAccess : public Resource
{
public:
	UnorderedAccess(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle);
};