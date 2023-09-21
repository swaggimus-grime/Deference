#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include "Graphics.h"
#include "Target.h"
#include "DepthStencil.h"
#include "Swapchain.h"

class RenderTarget : public Target
{
public:
	RenderTarget(Graphics& g, DXGI_FORMAT fmt = Swapchain::s_Format);
	RenderTarget(const ComPtr<ID3D12Resource>& res);

	virtual void CreateView(Graphics& g, HCPU h) override;

	virtual void Clear(Graphics& g) override;
	virtual void Bind(Graphics& g) override;
	void BindWithDepth(Graphics& g, Shared<DepthStencil> ds);
	void BindWithDepth(Graphics& g, DepthStencil& ds);
	virtual void Resize(Graphics& g, UINT w, UINT h) override;

};