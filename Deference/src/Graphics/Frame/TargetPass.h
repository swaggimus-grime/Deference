#pragma once

#include "Pass.h"

class Target;

class TargetPass : public Pass
{
public:
	TargetPass(const std::string& name);
	virtual void OnAdd(Graphics& g) override;

protected:
	Shared<Target> m_Target;
};