#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include "Graphics.h"
#include "Target.h"
#include "DepthStencil.h"

class RenderTarget : public Target
{
public:
	RenderTarget(Graphics& g);
	RenderTarget(const ComPtr<ID3D12Resource>& res);

	virtual void CreateView(Graphics& g, HCPU hcpu) override;

	virtual void Clear(Graphics& g) override;
	virtual void Bind(Graphics& g) override;
	void BindWithDepth(Graphics& g, Shared<DepthStencil> ds);
	void BindWithDepth(Graphics& g, DepthStencil& ds);

private:
	D3D12_CPU_DESCRIPTOR_HANDLE m_ShaderResourceHandle;
};
