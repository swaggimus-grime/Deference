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

void Pass::Finish(Graphics& g)
{
	m_Viewport = MakeShared<Viewport>(g);
	AddBindable(m_Viewport);

	m_TargetHeap = MakeUnique<RenderTargetHeap>(g, m_OutTargets.size());
	for (auto& t : m_OutTargets)
	{
		auto& target = std::get<2>(t);
		target = MakeShared<RenderTarget>(g, std::get<1>(t));
		m_TargetHeap->Add(g, target);
	}

	UINT totalVectorResources = 0;
	for (auto& p : m_GlobalVectorResources)
	{
		auto [hcpu, num, stride] = m_Parent->GetGlobalVectorResource(p.first);
		totalVectorResources += num;
	}

	m_GPUHeap = MakeUnique<GPUShaderHeap>(g, 
		m_InTargets.size() + m_Resources.size() + m_TargetResources.size() + 
		m_GlobalResources.size() + totalVectorResources);

	for (auto& r : m_Resources)
		r.second = m_GPUHeap->Add(g, r.first);

	for (auto& in : m_InTargets)
		m_Resources.emplace_back(in.second, m_GPUHeap->AddTarget(g, in.second));
	
	for(auto& t : m_TargetResources)
		m_Resources.emplace_back(t, m_GPUHeap->AddTarget(g, t));

	for (auto& p : m_GlobalResources)
	{
		auto r = m_Parent->GetGlobalResource(p.first);
		p.second = m_GPUHeap->Copy(g, r);
	}

	for (auto& p : m_GlobalVectorResources)
	{
		auto[hcpu, num, stride] = m_Parent->GetGlobalVectorResource(p.first);
		p.second = m_GPUHeap->Copy(g, hcpu, num, stride);
	}
}

void Pass::OnResize(Graphics& g, UINT w, UINT h)
{
	m_Viewport->Resize(w, h);

	for (auto& t : m_OutTargets)
		std::get<2>(t)->Resize(g, w, h);
}

void Pass::AddResource(Shared<Resource> r)
{
	m_Resources.emplace_back(std::move(r), HGPU{0});
}

void Pass::AddTargetResource(Shared<Target> r)
{
	m_TargetResources.push_back(std::move(r));
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
