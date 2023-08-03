#pragma once

#include "RaytracePass.h"

class AOPipeline;

class AOPass : public RaytracePass
{
public:
	AOPass(Graphics& g, FrameGraph* parent);
	virtual void OnAdd(Graphics& g) override;
	virtual void Run(Graphics& g) override;
	virtual void ShowGUI() override;

private:
	Shared<ConstantBuffer> m_Constants;
	UINT m_FrameCount;
};