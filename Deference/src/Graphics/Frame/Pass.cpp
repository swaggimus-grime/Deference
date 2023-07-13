#include "Pass.h"

void Pass::OnAdd(Graphics& g, GeometryGraph* parent)
{
	m_RTs = MakeUnique<RenderTargetHeap>(g, m_OutTargets.size());
	for (auto& t : m_OutTargets)
		t.second = m_RTs->Add(g);
}

void Pass::BindBindables(Graphics& g)
{
	for (auto& b : m_Bindables)
		b->Bind(g);
}
