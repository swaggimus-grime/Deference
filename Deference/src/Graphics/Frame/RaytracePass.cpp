#include "RaytracePass.h"

RaytracePass::RaytracePass(const std::string& name, FrameGraph* parent)
	:Pass(std::move(name), parent)
{
	QueryGlobalResource("TLAS");
}

void RaytracePass::Finish(Graphics& g)
{
	for (const auto& name : GetOutTargets())
	{
		auto out = MakeShared<UnorderedAccess>(g);
		AddResource(out);
		m_Outputs.insert({ std::get<0>(name), std::move(out)});
	}

	Pass::Finish(g);
}

void RaytracePass::OnResize(Graphics& g, UINT w, UINT h)
{
	Pass::OnResize(g, w, h);
	for(auto& out : m_Outputs)
		out.second->Resize(g, w, h);
}

Shared<UnorderedAccess> RaytracePass::GetOutput(const std::string& name)
{
	return m_Outputs[name];
}
