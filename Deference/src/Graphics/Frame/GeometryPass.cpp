//#include "GeometryPass.h"
//#include "Bindable/Heap/RenderTarget.h"
//#include "Bindable/Pipeline/GeometryPipeline.h"
//#include "Bindable/Viewport.h"
//#include "FrameGraph.h"
//#include "Bindable/Pipeline/GeometryPipeline.h"
//
//GeometryPass::GeometryPass(Graphics& g, const std::string& name, FrameGraph* parent)
//	:RasterPass(g, std::move(name), parent), m_SamplerHeap(g, 1)
//{
//	AddOutTarget("Position", DXGI_FORMAT_R32G32B32A32_FLOAT);
//	AddOutTarget("Normal", DXGI_FORMAT_R32G32B32A32_FLOAT);
//	AddOutTarget("Albedo");
//	AddOutTarget("Specular");
//	AddOutTarget("Emissive");
//
//	QueryGlobalVectorResource("Models");
//
//	auto& outs = GetOutTargets();
//	const auto& formats =
//		std::views::iota(outs.begin(), outs.end()) |
//		std::views::transform([&](const auto& it) {
//		return std::get<1>(*it);
//			}) |
//		std::ranges::to<std::vector>();
//
//	m_Pipeline = MakeUnique<GeometryPipeline>(g, formats);
//	m_SamplerHeap.Add(g);
//
//	{
//		ConstantBufferLayout layout;
//		layout.Add<CONSTANT_TYPE::XMMATRIX>("world");
//		layout.Add<CONSTANT_TYPE::XMMATRIX>("mvp");
//		layout.Add<CONSTANT_TYPE::XMFLOAT3X3>("normMat");
//		m_Transform = MakeShared<ConstantBuffer>(g, std::move(layout));
//	}
//	{
//		ConstantBufferLayout layout;
//		layout.Add<CONSTANT_TYPE::XMFLOAT3>("pos");
//		m_Camera = MakeShared<ConstantBuffer>(g, std::move(layout));
//	}
//}
//
//void GeometryPass::Run(Graphics& g)
//{
//	__super::Run(g);
//
//	ID3D12DescriptorHeap* heaps[] = { **m_GPUHeap, *m_SamplerHeap };
//	g.CL().SetDescriptorHeaps(2, heaps);
//	g.CL().SetGraphicsRootDescriptorTable(4, m_SamplerHeap.GPUStart());
//
//	const auto& modelResources = GetGlobalVectorResource("Models");
//	const auto& models = m_Parent->GetModels();
//	const auto& cam = m_Parent->GetCamera();
//	(*m_Camera)["pos"] = cam->Pos();
//	g.CL().SetGraphicsRootConstantBufferView(6, m_Camera->GetGPUAddress());
//
//	UINT meshIdx = 0;
//	for (UINT i = 0; i < models.size(); i++)
//	{
//		XMMATRIX world = models[i]->GetWorldTransform();
//
//		(*m_Transform)["world"] = XMMatrixTranspose(world);
//		(*m_Transform)["mvp"] = XMMatrixTranspose(world * cam->View() * cam->Proj());
//		(*m_Transform)["normMat"] = XMMatrixInverse(nullptr, world);
//
//		g.CL().SetGraphicsRootConstantBufferView(5, m_Transform->GetGPUAddress());
//
//		const auto& meshes = models[i]->GetMeshes();
//		for (UINT j = 0; j < meshes.size(); j++)
//		{
//			auto& mesh = meshes[j];
//			mesh.m_VB->Bind(g);
//			mesh.m_IB->Bind(g);
//			g.CL().SetGraphicsRootDescriptorTable(0, modelResources[meshIdx][2]);
//			g.CL().SetGraphicsRootDescriptorTable(1, modelResources[meshIdx][3]);
//			g.CL().SetGraphicsRootDescriptorTable(2, modelResources[meshIdx][4]);
//			g.CL().SetGraphicsRootDescriptorTable(3, modelResources[meshIdx][5]);
//			g.CL().SetGraphicsRootConstantBufferView(7, mesh.m_Materials->GetGPUAddress());
//			g.CL().DrawIndexedInstanced(mesh.m_IB->NumIndices(), 1, 0, 0, 0);
//
//			meshIdx++;
//		}
//	}
//
//	g.Flush();
//}