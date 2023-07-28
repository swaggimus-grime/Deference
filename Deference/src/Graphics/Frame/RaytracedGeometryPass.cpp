//#include "RaytracedGeometryPass.h"
//#include "Bindable/Pipeline/RaytracedGeometryPipeline.h"
//#include <ranges>
//#include "FrameGraph.h"
//
//RaytracedGeometryPass::RaytracedGeometryPass(Graphics& g)
//	:m_GPUHeap(g, 6), m_Environment(g, L"textures\\MonValley_G_DirtRoad_3k.hdr")
//{
//	m_Outputs.reserve(3);
//	
//	AddOutTarget("Position");
//	AddOutTarget("Normal");
//	AddOutTarget("Albedo");
//}
//
//void RaytracedGeometryPass::OnAdd(Graphics& g, FrameGraph* parent)
//{
//	Pass::OnAdd(g, parent);
//
//	auto& ins = GetInTargets();
//
//	for (auto& o : m_Outputs)
//	{
//		o = MakeShared<UnorderedAccess>(g);
//		o->CreateView(g, m_GPUHeap.Next());
//	}
//
//	const auto& models = parent->GetModels();
//	for (const auto& m : models)
//	{
//		
//	}
//
//	ConstantBufferLayout layout;
//	layout.Add<CONSTANT_TYPE::XMMATRIX>("viewInv");
//	layout.Add<CONSTANT_TYPE::XMMATRIX>("projInv");
//	layout.Add<CONSTANT_TYPE::XMFLOAT3>("wPos");
//	m_Transform = MakeUnique<ConstantBuffer>(g, std::move(layout));
//	m_Transform->CreateView(g, m_GPUHeap.Next());
//
//	parent->GetTLAS().CreateView(g, m_GPUHeap->Next());
//	m_Environment.CreateView(g, m_GPUHeap->Next());
//
//	m_Pipeline = MakeShared<RaytracedGeometryPipeline>(g);
//	UINT64* heapPtr = reinterpret_cast<UINT64*>(m_GPUHeap.GPUStart().ptr);
//	m_Pipeline->UpdateTable(g,
//		{
//			{RaytracedGeometryPipeline::rayGenEP, {heapPtr}}
//		},
//		{
//			{RaytracedGeometryPipeline::missEP, {}}
//		},
//		{
//			{RaytracedGeometryPipeline::hitGroup, {}}
//		}
//		);
//}
//
//void RaytracedGeometryPass::Run(Graphics& g, FrameGraph* parent)
//{
//	auto& outs = GetOutTargets();
//
//	m_Pipeline->Bind(g);
//	m_GPUHeap->Bind(g);
//
//	m_Pipeline->Dispatch(g);
//
//	const auto& targets =
//		std::views::iota(0u, (UINT)outs.size()) |
//		std::views::transform([&](UINT i) {
//		return outs.at(i).second;
//			}) |
//		std::ranges::to<std::vector>();
//	Resource::Transition(g, m_Outputs, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
//	Resource::Transition(g, targets, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
//	
//	for(UINT i = 0; i < m_Outputs.size(); i++)
//		g.CL().CopyResource(**targets[i], **m_Outputs[i]);
//
//	Resource::Transition(g, m_Outputs, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//	Resource::Transition(g, targets, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
//	g.Flush();
//}
//
//void RaytracedGeometryPass::ShowGUI()
//{
//}
