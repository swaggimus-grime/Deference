#pragma once

#include "Pass.h"

class RaytracePass : public Pass
{
public:
	virtual void OnResize(Graphics& g, UINT w, UINT h) override;
	inline const auto& GetOutputs() const { return m_Outputs; }
	Shared<UnorderedAccess> GetOutput(const std::string& name);

protected:
	RaytracePass(const std::string& name, FrameGraph* parent);
	virtual void Finish(Graphics& g) override;

protected:
	Unique<RaytracingPipeline> m_Pipeline;
	std::vector<std::pair<std::string, Shared<UnorderedAccess>>> m_Outputs;
	Shared<TLAS> m_TLAS;
};