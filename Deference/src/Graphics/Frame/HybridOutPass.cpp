#include "HybridOutPass.h"
#include "Bindable/Pipeline/HybridOutPipeline.h"
#include <ranges>

HybridOutPass::HybridOutPass(Graphics& g)
	:ScreenPass(g)
{
	AddInTarget("Position");
	AddInTarget("Diffuse");
	AddInTarget("Accumulation");
	AddOutTarget("Hybrid");

	m_Pipeline = MakeUnique<HybridOutPipeline>(g);
}

void HybridOutPass::Run(Graphics& g)
{
	__super::Run(g);

	auto& inTargets = GetInTargets();
	const auto& targets = 
		std::views::iota(0u, (UINT)inTargets.size()) |
		std::views::transform([&](UINT i) {
			return inTargets.at(i).second;
		}) |
		std::ranges::to<std::vector>();
	Resource::Transition(g, targets, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	m_GPUHeap->Bind(g);
	g.CL().SetGraphicsRootDescriptorTable(0, m_GPUHeap->GPUStart());

	Rasterize(g);

	Resource::Transition(g, targets, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	g.Flush();
}
