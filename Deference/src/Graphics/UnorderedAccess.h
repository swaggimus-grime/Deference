#pragma once

#include "ShaderAccessible.h"
#include <d3d12.h>

class Graphics;

class UnorderedAccess : public ShaderAccessible
{
public:
	UnorderedAccess(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle);
};