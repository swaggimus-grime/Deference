#include "Pass.h"

Pass::Pass(FrameGraph* parent)
	:m_Parent(parent)
{
}

void Pass::OnAdd(Graphics& g)
{
	m_Viewport = MakeShared<Viewport>(g);
	AddBindable(m_Viewport);

	m_RTs = MakeUnique<RenderTargetHeap>(g, m_OutTargets.size());
	for (auto& t : m_OutTargets)
	{
		t.second = MakeShared<RenderTarget>(g);
		t.second->CreateView(g, m_RTs->Next().m_HCPU);
	}

	m_GPUHeap = MakeUnique<GPUShaderHeap>(g, m_InTargets.size() + m_Resources.size() + m_GlobalResources.size());
	for (auto& in : m_InTargets)
		in.second->CreateShaderResourceView(g, m_GPUHeap->Next());

	for (auto& name : m_GlobalResourceNames)
	{
		auto r = m_Parent->GetGlobalResource(name);
		r->CopyView(g, m_GPUHeap->Next().m_HCPU);
	}

	for (auto& r : m_Resources)
	{
		r->CreateView(g, m_GPUHeap->Next().m_HCPU);
	}
}

void Pass::OnResize(Graphics& g, UINT w, UINT h)
{
	m_Viewport->Resize(w, h);

	for (auto& t : m_OutTargets)
		t.second->Resize(g, w, h);

	m_GPUHeap->Reset();
	auto& ins = GetInTargets();
	for (auto& in : ins)
		in.second->CreateShaderResourceView(g, m_GPUHeap->Next());
}

void Pass::AddResource(Shared<Resource> r)
{
	m_Resources.push_back(std::move(r));
}

void Pass::GetGlobalResource(const std::string& name)
{
	m_GlobalResourceNames.push_back(std::move(name));
}

void Pass::BindBindables(Graphics& g)
{
	for (auto& b : m_Bindables)
		b->Bind(g);
}
