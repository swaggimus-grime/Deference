#include "TargetPass.h"

TargetPass::TargetPass(const std::string& name)
	:Pass(std::move(name))
{
	AddTarget("target");
}

void TargetPass::OnAdd(Graphics& g)
{
	m_Target = GetTarget("target");
}