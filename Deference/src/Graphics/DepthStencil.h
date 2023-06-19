#pragma once

#include "Target.h"

class RenderTarget;

class DepthStencil : public Target
{
public:
	DepthStencil(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle);
	virtual void Clear(Graphics& g) override;
	virtual void Bind(Graphics& g) override;
	void BindWithRT(Graphics& g, RenderTarget* rt);

public:
	static constexpr auto s_Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
};