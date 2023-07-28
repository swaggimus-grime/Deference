#include "Pass.h"

void Pass::OnAdd(Graphics& g, FrameGraph* parent)
{
	m_Viewport = MakeShared<Viewport>(g);
	AddBindable(m_Viewport);

	m_RTs = MakeUnique<RenderTargetHeap>(g, m_OutTargets.size());
	for (auto& t : m_OutTargets)
	{
		t.second = MakeShared<RenderTarget>(g);
		t.second->CreateView(g, m_RTs->Next());
	}
}

void Pass::OnResize(Graphics& g, UINT w, UINT h)
{
	for (auto& t : m_OutTargets)
		t.second->Resize(g, w, h);

	m_Viewport->Resize(w, h);
}

void Pass::BindBindables(Graphics& g)
{
	for (auto& b : m_Bindables)
		b->Bind(g);
}
