#pragma once

#include "BindablePass.h"

class AccumPass : public BindablePass
{
public:
	AccumPass(Graphics& g);

	virtual void OnAdd(Graphics& g, GeometryGraph* parent) override;
	virtual void Run(Graphics& g, GeometryGraph* parent) override;

private:
	DepthStencilHeap m_DepthHeap;
	Shared<DepthStencil> m_Depth;
	Unique<CSUHeap> m_ResHeap;
	Shared<VertexBuffer> m_VB;
	Shared<IndexBuffer> m_IB;
	UINT m_NumPassedFrames;

	RenderTargetHeap m_PrevFrameHeap;
	Shared<RenderTarget> m_PrevFrame;
	XMFLOAT3 m_PrevCamHash;
};