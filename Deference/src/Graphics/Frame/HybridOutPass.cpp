#include "HybridOutPass.h"
#include "Bindable/Pipeline/HybridOutPipeline.h"
#include <ranges>

HybridOutPass::HybridOutPass(Graphics& g)
	:ScreenPass(g), m_DepthHeap(g)
{
	m_Depth = MakeShared<DepthStencil>(g);
	m_Depth->CreateView(g, m_DepthHeap.Next());

	AddInTarget("Position");
	AddInTarget("Diffuse");
	AddInTarget("Accumulation");

	AddOutTarget("Hybrid");

	AddBindable(MakeShared<HybridOutPipeline>(g));
}

void HybridOutPass::OnAdd(Graphics& g, FrameGraph* parent)
{
	Pass::OnAdd(g, parent);

	auto& ins = GetInTargets();
	m_GPUHeap = MakeUnique<GPUShaderHeap>(g, ins.size());

	for (auto& in : ins)
		in.second->CreateShaderResourceView(g, m_GPUHeap->Next());
}

void HybridOutPass::Run(Graphics& g, FrameGraph* parent)
{
	auto target = GetOutTarget("Hybrid");
	target->BindWithDepth(g, m_Depth);
	target->Clear(g);
	m_Depth->Clear(g);

	auto& inTargets = GetInTargets();
	const auto& targets = 
		std::views::iota(0u, (UINT)inTargets.size()) |
		std::views::transform([&](UINT i) {
			return inTargets.at(i).second;
		}) |
		std::ranges::to<std::vector>();
	Resource::Transition(g, targets, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	BindBindables(g);
	m_GPUHeap->Bind(g);
	g.CL().SetGraphicsRootDescriptorTable(0, m_GPUHeap->GPUStart());

	Rasterize(g);

	Resource::Transition(g, targets, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	g.Flush();
}

void HybridOutPass::OnResize(Graphics& g, UINT w, UINT h)
{
	Pass::OnResize(g, w, h);
	m_Depth->Resize(g, w, h);

	m_GPUHeap->Reset();
	auto& ins = GetInTargets();
	for (auto& in : ins)
		in.second->CreateShaderResourceView(g, m_GPUHeap->Next());
}
