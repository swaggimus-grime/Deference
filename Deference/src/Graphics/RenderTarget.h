#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include "Graphics.h"
#include "Target.h"
#include "DepthStencil.h"

class RenderTarget : public Target
{
public:
	RenderTarget(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, ComPtr<ID3D12Resource> res);
	virtual void Clear(Graphics& g) override;
	virtual void Bind(Graphics& g) override;
	void BindWithDepth(Graphics& g, DepthStencil* ds);

public:
	static constexpr auto s_Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
};