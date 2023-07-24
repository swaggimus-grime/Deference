#pragma once

#include "Pass.h"
#include "Bindable/Heap/DescriptorHeap.h"
#include "Bindable/Heap/RenderTarget.h"
#include "Bindable/Heap/DepthStencil.h"

class VertexBuffer;
class IndexBuffer;

class GeometryPass : public Pass
{
public:
	GeometryPass(Graphics& g);
	virtual void OnAdd(Graphics& g, FrameGraph* parent) override;
	virtual void Run(Graphics& g, FrameGraph* parent) override;

private:
	DepthStencilHeap m_DepthHeap;
	Shared<DepthStencil> m_Depth;

	SamplerHeap m_SamplerHeap;

	GPUShaderHeap m_GPUHeap;

	Shared<Camera> m_Cam;
	Unique<ConstantBuffer> m_Transform;
};