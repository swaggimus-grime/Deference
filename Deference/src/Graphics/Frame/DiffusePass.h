#pragma once

#include "RaytracePass.h"

class DiffusePass : public RaytracePass
{
public:
	DiffusePass(Graphics& g, FrameGraph* parent);
	virtual void OnAdd(Graphics& g) override;
	virtual void ShowGUI() override;
	virtual void Run(Graphics& g) override;

private:
	Shared<EnvironmentMap> m_Environment;
	Shared<ConstantBuffer> m_Constants;
	Shared<ConstantBuffer> m_Light;
	std::vector<Shared<Model>> m_Models;
	UINT m_FrameCount;
};