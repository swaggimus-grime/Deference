#pragma once

#include "Pass.h"
#include "Bindable/Heap/Heap.h"
#include "Bindable/Heap/RenderTarget.h"
#include "Bindable/Heap/DepthStencil.h"

class VertexBuffer;
class IndexBuffer;
class Material;
class DrawableCollection;

class GeometryPass : public Pass
{
public:
	GeometryPass(Graphics& g);
	virtual void OnAdd(Graphics& g, GeometryGraph* parent) override;
	virtual void Run(Graphics& g, GeometryGraph* parent) override;

private:
	DepthStencilHeap m_DepthHeap;
	Shared<DepthStencil> m_Depth;
	Unique<SamplerHeap> m_SamplerHeap;
	Unique<CSUHeap> m_TextureHeap;
	Shared<Camera> m_Cam;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_CBVHandle;
};