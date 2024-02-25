#include "Pass.h"
#include "FrameGraph.h"

namespace Def
{
	void Pass::Link(const std::string& otherPass, const std::string& otherTarget, std::optional<std::string> targetName)
	{
		std::string target = targetName.has_value() ? *targetName : otherTarget;
		auto it = std::find_if(m_InTargets.begin(), m_InTargets.end(),
			[&](const auto& p) {
				return p.first == target;
			});
		if (it != m_InTargets.end())
			it->second = m_Parent->GetTarget(PassTargetName(std::move(otherPass), std::move(otherTarget)));
		else
			throw new DefException("Cannot find pass with name " + otherPass);
	}

	void Pass::Compile(Graphics& g)
	{
		m_TargetHeap = MakeUnique<RenderTargetHeap>(g, m_OutTargets.size());
		for (auto& target : m_OutTargets)
			m_TargetHeap->Add(g, target.second);

		for (auto& in : m_InTargets)
			AddResource(in.second);
	}

	void Pass::BuildResources(Graphics& g)
	{
		m_GPUHeap = MakeUnique<GPUHeap>(g, m_InTargets.size() + m_Resources.size());
		for (auto& r : m_Resources)
			if(r.second)
				m_GPUHeap->AddNull(g, r.first);
			else
				m_GPUHeap->Add(g, r.first);

		for (auto& r : m_InTargets)
			m_GPUHeap->Add(g, r.second);
	}

	void Pass::OnResize(Graphics& g, UINT w, UINT h)
	{
		for (auto& t : m_OutTargets)
			t.second->Resize(g, w, h);

		for (auto& r : m_Resizeables)
			r->Resize(g, w, h);
	}
}