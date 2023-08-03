//#pragma once
//
//#include "RasterPass.h"
//#include "Bindable/Heap/DescriptorHeap.h"
//#include "Bindable/Heap/RenderTarget.h"
//#include "Bindable/Heap/DepthStencil.h"
//
//class Model;
//class VertexBuffer;
//class IndexBuffer;
//
//class GeometryPass : public RasterPass
//{
//public:
//	GeometryPass(Graphics& g, FrameGraph* parent);
//	virtual void Run(Graphics& g) override;
//
//protected:
//	
//
//private:
//	SamplerHeap m_SamplerHeap;
//
//	Shared<Camera> m_Cam;
//	std::vector<Shared<Model>> m_Models;
//	Shared<ConstantBuffer> m_Transform;
//};