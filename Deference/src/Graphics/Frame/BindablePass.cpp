#include "BindablePass.h"

void BindablePass::Bind(Graphics& g)
{
	for (auto& b : m_Bindables)
		b->Bind(g);
}

void BindablePass::AddBindable(Shared<Bindable> bindable)
{
	m_Bindables.push_back(std::move(bindable));
}