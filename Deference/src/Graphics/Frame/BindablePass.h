#pragma once

#include "Pass.h"

class BindablePass : public Pass
{
protected:
	void AddBindable(Shared<Bindable> bindable);
	void Bind(Graphics& g);

private:
	std::vector<Shared<Bindable>> m_Bindables;
};