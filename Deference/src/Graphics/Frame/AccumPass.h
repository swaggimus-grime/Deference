#pragma once

#include "ScreenPass.h"

class AccumPass : public ScreenPass
{
public:
	AccumPass(Graphics& g);

	virtual void OnAdd(Graphics& g, FrameGraph* parent) override;
	virtual void Run(Graphics& g, FrameGraph* parent) override;
	virtual void ShowGUI() override;
	virtual void OnResize(Graphics& g, UINT w, UINT h) override;
private:
	Unique<GPUShaderHeap> m_GPUHeap;
	UINT m_NumPassedFrames;

	RenderTargetHeap m_PrevFrameHeap;
	Unique<RenderTarget> m_PrevFrame;
	XMFLOAT3 m_PrevCamHash;
};