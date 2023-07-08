#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include "Graphics.h"
#include "Target.h"
#include "DepthStencil.h"

class RenderTarget : public Target
{
public:
	RenderTarget(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, D3D12_RESOURCE_STATES initState, ComPtr<ID3D12Resource> res);
	RenderTarget(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_RENDER_TARGET);

	virtual void Clear(Graphics& g) override;
	virtual void Bind(Graphics& g) override;
	virtual void BindWithOther(Graphics& g, Target* ds) override;
};

class RTV : public Resource
{
public:
	RTV(Graphics& g, const D3D12_CPU_DESCRIPTOR_HANDLE& handle, Shared<RenderTarget> rt);
};