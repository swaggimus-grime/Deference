#pragma once

#include "ShaderAccessible.h"
#include <d3d12.h>
#include <wrl.h>

class Graphics;

class ShaderResource : public ShaderAccessible
{
public:
	ShaderResource(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, ComPtr<ID3D12Resource> res, D3D12_SRV_DIMENSION dimension);
};

class TopLevelAS : public ShaderResource
{
public:
	TopLevelAS(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, ComPtr<ID3D12Resource> res);
};