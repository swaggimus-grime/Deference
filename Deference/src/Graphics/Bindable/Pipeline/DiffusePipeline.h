#pragma once

#include "RaytracingPipeline.h"
#include "Bindable/Heap/Heap.h"

class DiffusePipeline : public RaytracingPipeline
{
public:
	DiffusePipeline(Graphics& g, const SucHeap& heap);

private:
	Shared<RootSig> m_RayGenSig;
	Shared<RootSig> m_HitSig;
};