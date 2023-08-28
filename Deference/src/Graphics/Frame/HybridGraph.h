#pragma once

#include "FrameGraph.h"

class HybridGraph : public FrameGraph
{
public:
	HybridGraph(Graphics& g, Scene& scene);

protected:
	virtual void RecordPasses(Graphics& g) override;

private:
	Unique<CPUShaderHeap> m_ModelHeap;
};