#pragma once

#include "Resource.h"
#include "util.h"
#include <d3d12.h>

class Graphics;
class Target;

class ShaderResource : public Resource
{
public:
	ShaderResource(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, D3D12_SRV_DIMENSION dim, ComPtr<ID3D12Resource> res);
};

class TargetSR : public ShaderResource
{
public:
	TargetSR(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, Shared<Target> target);
};

class TopLevelAS : public ShaderResource
{
public:
	TopLevelAS(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, ComPtr<ID3D12Resource> res);
};