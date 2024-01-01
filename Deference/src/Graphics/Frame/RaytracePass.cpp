#include "RaytracePass.h"

RaytracePass::RaytracePass(const std::string& name, FrameGraph* parent)
	:Pass(std::move(name), parent)
{
	QueryGlobalResource("TLAS");
}

void RaytracePass::AddOutTarget(Graphics& g, const std::string& target, DXGI_FORMAT fmt)
{
	__super::AddOutTarget(g, target, fmt);
	auto out = MakeShared<UnorderedAccess>(g, fmt);
	AddResource(out);
	m_Outputs.emplace_back(std::move(target), std::move(out));
}

void RaytracePass::OnResize(Graphics& g, UINT w, UINT h)
{
	__super::OnResize(g, w, h);
	for(auto& out : m_Outputs)
		out.second->Resize(g, w, h);
}

Shared<UnorderedAccess> RaytracePass::GetOutput(const std::string& name)
{
	return std::find_if(m_Outputs.begin(), m_Outputs.end(), [&](const auto& p) {return name == p.first; })->second;
}
