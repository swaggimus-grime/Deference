#pragma once

#include "RaytracePass.h"

class DiffusePass : public RaytracePass
{
public:
	DiffusePass(Graphics& g, FrameGraph* parent);
	virtual void OnAdd(Graphics& g) override;
	virtual void ShowGUI() override;

private:
	Shared<ConstantBuffer> m_Light;
};