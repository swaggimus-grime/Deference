//#include "HybridOutPass.h"
//#include "Bindable/Pipeline/HybridOutPipeline.h"
//#include <ranges>
//
//HybridOutPass::HybridOutPass(Graphics& g, const std::string& name, FrameGraph* parent)
//	:ScreenPass(g, std::move(name), parent)
//{
//	AddInTarget("Position");
//	AddInTarget("Diffuse");
//	AddInTarget("AO");
//	AddOutTarget("Target");
//
//	m_Pipeline = MakeUnique<HybridOutPipeline>(g);
//}
//
//void HybridOutPass::Run(Graphics& g)
//{
//	__super::Run(g);
//
//	auto& inTargets = GetInTargets();
//	const auto& targets = 
//		std::views::iota(inTargets.begin(), inTargets.end()) |
//		std::views::transform([&](const auto& it) {
//			return it->second;
//		}) |
//		std::ranges::to<std::vector>();
//
//	Resource::Transition(g, targets, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
//
//	m_GPUHeap->Bind(g);
//	g.CL().SetGraphicsRootDescriptorTable(0, m_GPUHeap->GPUStart());
//
//	Rasterize(g);
//
//	Resource::Transition(g, targets, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
//
//	g.Flush();
//}
