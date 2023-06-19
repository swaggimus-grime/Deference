#pragma once

#include "ShaderAccessible.h"
#include <d3d12.h>

class Graphics;

class ConstantBuffer : public ShaderAccessible
{
public:
	ConstantBuffer(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, ComPtr<ID3D12Resource> res, SIZE_T size);

};