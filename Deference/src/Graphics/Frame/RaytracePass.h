#pragma once

#include "BindablePass.h"

class Target;
class DXRPipeline;
class UnorderedAccess;

class RaytracePass : public BindablePass
{
public:
	RaytracePass(Graphics& g, const std::string& name, ComPtr<ID3D12Resource> topLevelAS, Shared<Camera> cam);
	virtual void Run(Graphics& g) override;
	virtual void OnAdd(Graphics& g) override;

protected:
	Shared<Target> m_RT;

	Heap<Resource> m_Heap;
	Shared<UnorderedAccess> m_Output;

	Shared<DXRPipeline> m_Pipeline;
};