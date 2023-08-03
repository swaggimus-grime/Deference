#pragma once

#include "ScreenPass.h"

class AccumPass : public ScreenPass
{
public:
	AccumPass(Graphics& g, FrameGraph* parent);

	virtual void OnAdd(Graphics& g) override;
	virtual void Run(Graphics& g) override;
	virtual void ShowGUI() override;
	virtual void OnResize(Graphics& g, UINT w, UINT h) override;

private:
	UINT m_NumPassedFrames;

	RenderTargetHeap m_PrevFrameHeap;
	Unique<RenderTarget> m_PrevFrame;
	Shared<Camera> m_Cam;
	XMFLOAT3 m_PrevCamHash;
};