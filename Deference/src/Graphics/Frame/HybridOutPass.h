#pragma once

#include "ScreenPass.h"

class HybridOutPass : public ScreenPass
{
public:
	HybridOutPass(Graphics& g);

	virtual void OnAdd(Graphics& g, FrameGraph* parent) override;
	virtual void Run(Graphics& g, FrameGraph* parent) override;

private:
	DepthStencilHeap m_DepthHeap;
	Shared<DepthStencil> m_Depth;
	Unique<GPUShaderHeap> m_GPUHeap;
};