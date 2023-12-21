#include "RaytracePass.h"

RaytracePass::RaytracePass(const std::string& name, FrameGraph* parent)
	:Pass(std::move(name), parent)
{
	QueryGlobalResource("TLAS");
}

void RaytracePass::Finish(Graphics& g)
{
	for (const auto& tuple : GetOutTargets())
	{
		auto out = MakeShared<UnorderedAccess>(g, std::get<1>(tuple));
		AddResource(out);
		m_Outputs.emplace_back(std::get<0>(tuple), std::move(out));
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
	return std::find_if(m_Outputs.begin(), m_Outputs.end(), [&](const auto& p) {return name == p.first; })->second;
}
