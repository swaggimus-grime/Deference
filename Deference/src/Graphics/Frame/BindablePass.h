#pragma once

#include "Pass.h"

class BindablePass : public Pass
{
public:
	BindablePass(const std::string& name);
	virtual void Run(Graphics& g) override;

protected:
	void AddBindable(Shared<Bindable> bindable);

private:
	std::vector<Shared<Bindable>> m_Bindables;
};