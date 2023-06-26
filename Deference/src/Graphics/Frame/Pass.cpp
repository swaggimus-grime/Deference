#include "Pass.h"
#include "Resource/Target.h"
#include "Entity/Step.h"

Pass::Pass(const std::string& name)
	:m_Name(std::move(name))
{
}

Shared<Target>& Pass::GetTarget(const std::string& name)
{
	return m_Targets[name];
}

void Pass::AddTarget(const std::string& name)
{
	m_Targets.insert({ std::move(name), nullptr });
}

void Pass::AddStep(const Step& step)
{
	m_Steps.push_back(std::move(step));
}