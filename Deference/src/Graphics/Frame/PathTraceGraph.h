#pragma once

#include "FrameGraph.h"

class PathTraceGraph : public FrameGraph
{
public:
	PathTraceGraph(Graphics& g, Scene& scene);

protected:
	virtual void RecordPasses(Graphics& g) override;

private:
	Unique<CPUShaderHeap> m_ModelHeap;
};