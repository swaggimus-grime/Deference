#pragma once

#include "BindablePass.h"

class RasterPass : public BindablePass
{
public:
	RasterPass(const std::string& name);
	virtual void Run(Graphics& g) override;
	virtual void OnAdd(Graphics& g) override;

protected:
	Shared<Target> m_RT;
	Shared<Target> m_DS;
};