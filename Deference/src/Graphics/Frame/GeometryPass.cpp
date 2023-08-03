//#include "GeometryPass.h"
//#include "Bindable/Heap/RenderTarget.h"
//#include "Bindable/Pipeline/GeometryPipeline.h"
//#include "Bindable/Viewport.h"
//#include "FrameGraph.h"
//#include "Bindable/Heap/Sampler.h"
//
//GeometryPass::GeometryPass(Graphics& g, FrameGraph* parent)
//	:RasterPass(g), m_SamplerHeap(g, 1), 
//	m_Cam(parent->GetCamera()), m_Models(std::move(parent->GetModels()))
//{
//	AddOutTarget("Position");
//	AddOutTarget("Normal");
//	AddOutTarget("Albedo");
//
//	m_Pipeline = MakeUnique<GeometryPipeline>(g);
//
//	m_SamplerHeap.Add(g);
//
//	const auto& models = parent->GetModels();
//	for (const auto& m : models)
//		for (const auto& texture : m->GetTextures())
//			AddResource(texture);
//
//	ConstantBufferLayout layout;
//	layout.Add<CONSTANT_TYPE::XMMATRIX>("world");
//	layout.Add<CONSTANT_TYPE::XMMATRIX>("mvp");
//	layout.Add<CONSTANT_TYPE::XMFLOAT3X3>("normMat");
//	m_Transform = MakeShared<ConstantBuffer>(g, std::move(layout));
//	AddResource(m_Transform);
//}
//
//void GeometryPass::Run(Graphics& g)
//{
//	__super::Run(g);
//
//	ID3D12DescriptorHeap* heaps[] = { **m_GPUHeap, *m_SamplerHeap };
//	g.CL().SetDescriptorHeaps(2, heaps);
//	g.CL().SetGraphicsRootDescriptorTable(0, m_GPUHeap->GPUStart());
//	g.CL().SetGraphicsRootDescriptorTable(1, m_SamplerHeap.GPUStart());
//
//	g.CL().IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	for (const auto& m : m_Models)
//	{
//		XMMATRIX world = m->GetWorldTransform();
//
//		(*m_Transform)["world"] = XMMatrixTranspose(world);
//		(*m_Transform)["mvp"] = XMMatrixTranspose(world * m_Cam->View() * m_Cam->Proj());
//		(*m_Transform)["normMat"] = XMMatrixInverse(nullptr, world);
//
//		g.CL().SetGraphicsRootConstantBufferView(2, m_Transform->GetGPUAddress());
//
//		const auto& buffers = m->GetBuffers();
//		const auto& textureIndexes = m->GetTextureIndexes();
//		for (UINT i = 0; i < buffers.size(); i++)
//		{
//			const auto& buffPair = buffers[i];
//			buffPair.first->Bind(g);
//			buffPair.second->Bind(g);
//			g.CL().SetGraphicsRoot32BitConstants(3, sizeof(TextureIndex) / sizeof(INT), &textureIndexes[i], 0);
//
//			g.CL().DrawIndexedInstanced(buffPair.second->NumIndices(), 1, 0, 0, 0);
//		}
//	}
//
//	g.Flush();
//}