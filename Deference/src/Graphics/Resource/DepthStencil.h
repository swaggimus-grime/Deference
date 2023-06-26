#pragma once

#include "Target.h"

class RenderTarget;

class DepthStencil : public Target
{
public:
	DepthStencil(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle);
	virtual void Clear(Graphics& g) override;
	virtual void Bind(Graphics& g) override;
	virtual void BindWithOther(Graphics& g, Target* rt) override;
};