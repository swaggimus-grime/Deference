#pragma once

#include "Pass.h"

class RasterPass : public Pass
{
public:
	RasterPass(Graphics& g, FrameGraph* parent);
	virtual void Run(Graphics& g) override;
	virtual void OnResize(Graphics& g, UINT w, UINT h) override;


protected:
	DepthStencil m_Depth;
	Unique<Pipeline> m_Pipeline;

private:
	DepthStencilHeap m_DepthHeap;
};