#include "Pass.h"

Pass::Pass(const std::string& name, FrameGraph* parent)
	:m_Name(std::move(name)), m_Parent(parent)
{
}

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
	for (auto& t : m_OutTargets)
		m_TargetHeap->Add(g, t.second);

	for (auto& in : m_InTargets)
		m_TargetResources.emplace_back(in.second, HGPU{ 0 });
}

void Pass::BuildGlobalResources(Graphics& g)
{
	UINT totalGlobalResources = m_GlobalResources.size();
	for (auto& p : m_GlobalVectorResources)
	{
		auto [hcpu, num, stride] = m_Parent->GetGlobalVectorResource(p.first);
		totalGlobalResources += num;
	}

	m_GPUHeap = MakeUnique<GPUShaderHeap>(g, 
		m_InTargets.size() + m_Resources.size() + m_TargetResources.size() + 
		totalGlobalResources);

	for (auto& r : m_Resources)
		r.second = m_GPUHeap->Add(g, r.first);

	for (auto& r : m_TargetResources)
		r.second = m_GPUHeap->AddTarget(g, r.first);

	for (auto& p : m_GlobalResources)
	{
		auto r = m_Parent->GetGlobalResource(p.first);
		p.second = m_GPUHeap->Copy(g, r);
	}

	for (auto& p : m_GlobalVectorResources)
	{
		auto [hcpu, num, stride] = m_Parent->GetGlobalVectorResource(p.first);
		p.second = m_GPUHeap->Copy(g, hcpu, num, stride);
	}
}

void Pass::OnResize(Graphics& g, UINT w, UINT h)
{
	for (auto& t : m_OutTargets)
		t.second->Resize(g, w, h);
}

void Pass::AddResource(Shared<Resource> r)
{
	m_Resources.emplace_back(std::move(r), HGPU{0});
}

void Pass::AddTargetResource(Shared<Target> r)
{
	m_TargetResources.emplace_back(std::move(r), HGPU{0});
}

void Pass::QueryGlobalResource(const std::string& name)
{
	m_GlobalResources.insert({ std::move(name), {0} });
}

void Pass::QueryGlobalVectorResource(const std::string& name)
{
	m_GlobalVectorResources.insert({ std::move(name), {} });
}

void Pass::BindBindables(Graphics& g)
{
	for (auto& b : m_Bindables)
		b->Bind(g);
}
