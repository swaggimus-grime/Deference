#include "BindablePass.h"

BindablePass::BindablePass(const std::string& name)
	:Pass(std::move(name))
{
}

void BindablePass::Run(Graphics& g)
{
	for (auto& b : m_Bindables)
		b->Bind(g);
}

void BindablePass::AddBindable(Shared<Bindable> bindable)
{
	m_Bindables.push_back(std::move(bindable));
}