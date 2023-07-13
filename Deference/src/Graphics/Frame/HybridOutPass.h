#pragma once

#include "ScreenPass.h"

class HybridOutPass : public ScreenPass
{
public:
	HybridOutPass(Graphics& g);

	virtual void OnAdd(Graphics& g, GeometryGraph* parent) override;
	virtual void Run(Graphics& g, GeometryGraph* parent) override;\

private:
	DepthStencilHeap m_DepthHeap;
	Shared<DepthStencil> m_Depth;
	Unique<CSUHeap> m_ResHeap;
};