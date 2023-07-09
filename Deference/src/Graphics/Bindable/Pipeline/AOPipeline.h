#pragma once

#include "RaytracingPipeline.h"

class AOPipeline : public RaytracingPipeline
{
public:
	AOPipeline(Graphics& g, const CSUHeap& heap);

private:
	Shared<RootSig> m_RayGenSig;
	Shared<RootSig> m_HitSig;
};