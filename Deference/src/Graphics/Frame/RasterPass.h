#pragma once

#include "Pass.h"

class RasterPass : public Pass
{
public:
	RasterPass(Graphics& g);
	virtual void OnResize(Graphics& g, UINT w, UINT h) override;

protected:
	DepthStencil m_Depth;

private:
	DepthStencilHeap m_DepthHeap;
};