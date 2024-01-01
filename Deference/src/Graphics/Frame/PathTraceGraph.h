#pragma once

#include "FrameGraph.h"

class PathTraceGraph : public FrameGraph
{
public:
	PathTraceGraph(Graphics& g);

protected:
	virtual void PrepLoadScene(Graphics& g) override;

private:
	Unique<CPUShaderHeap> m_ModelHeap;
};