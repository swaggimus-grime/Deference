#include "RaytracePass.h"
#include "Commander.h"

namespace Def
{
	RaytracePass::RaytracePass(const std::string& name, FrameGraph* parent)
		:Pass(std::move(name), parent)
	{
	}

	void RaytracePass::AddOutTarget(Graphics& g, const std::string& target, DXGI_FORMAT fmt, XMFLOAT4 clearColor)
	{
		__super::AddOutTarget(g, target, fmt);
		auto out = MakeShared<UnorderedAccess>(g, fmt);
		AddResource(out);
		m_UAVs.emplace_back(std::move(target), std::move(out));
	}

	void RaytracePass::CopyUAVsToRTs(Graphics& g)
	{
		auto& c = Commander<D3D12_RESOURCE_BARRIER>::Init();
		auto& targets = GetOutTargets();
		for (auto& target : targets)
			c.Add(target.second->Transition(D3D12_RESOURCE_STATE_COPY_DEST));
		for (auto& uav : m_UAVs)
			c.Add(uav.second->Transition(D3D12_RESOURCE_STATE_COPY_SOURCE));
		c.Transition(g);

		for (UINT i = 0; i < targets.size(); i++)
			g.CL().CopyResource(**targets.at(i).second, **m_UAVs.at(i).second);
		
		c = Commander<D3D12_RESOURCE_BARRIER>::Init();
		for (auto& target : GetOutTargets())
			c.Add(target.second->Transition(D3D12_RESOURCE_STATE_RENDER_TARGET));
		for (auto& uav : m_UAVs)
			c.Add(uav.second->Transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
		c.Transition(g);
	}

	void RaytracePass::OnResize(Graphics& g, UINT w, UINT h)
	{
		__super::OnResize(g, w, h);
		for (auto& out : m_UAVs)
			out.second->Resize(g, w, h);
	}

	/*Shared<UnorderedAccess> RaytracePass::GetUAV(const std::string& name)
	{
		return std::find_if(m_UAVs.begin(), m_UAVs.end(), [&](const auto& p) {return name == p.first; })->second;
	}*/
}