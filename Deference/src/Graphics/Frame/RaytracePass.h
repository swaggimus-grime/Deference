#pragma once

#include "Pass.h"

class RaytracePass : public Pass
{
public:
	virtual void OnAdd(Graphics& g) override;
	virtual void Run(Graphics& g) override;
	virtual void OnResize(Graphics& g, UINT w, UINT h) override;
	inline const auto& GetOutputs() const { return m_Outputs; }
	Shared<UnorderedAccess> GetOutput(const std::string& name);

protected:
	Unique<RaytracingPipeline> m_Pipeline;
	std::vector<Shared<UnorderedAccess>> m_Outputs;
	Shared<TLAS> m_TLAS;
};