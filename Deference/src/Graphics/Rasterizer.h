#pragma once

#include "Context.h"
#include "Heap.h"
#include "DepthStencil.h"

class DepthStencil;

class Rasterizer : public Context
{
public:
	Rasterizer(Graphics& g);
	virtual void Render(Graphics& g, Shared<RenderTarget> bb) override;

private:


	Unique<Heap<DepthStencil>> m_DSs;
};