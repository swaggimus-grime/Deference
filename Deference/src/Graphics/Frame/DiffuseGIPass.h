#pragma once

#include "RaytracePass.h"

class DiffuseGIPass : public RaytracePass
{
public:
	DiffuseGIPass(Graphics& g, const std::string& name, FrameGraph* parent);
	virtual void ShowGUI() override;
	virtual void Run(Graphics& g) override;
	virtual void OnSceneLoad(Graphics& g) override;
private:
	Shared<ConstantBuffer> m_Constants;
	Shared<ConstantBuffer> m_Light;
	UINT m_FrameCount;
};