#include "ToneMapPass.h"
#include <imgui.h>
#include "Bindable/Pipeline/ToneMapPipeline.h"

ToneMapPass::ToneMapPass(Graphics& g, const std::string& name, FrameGraph* parent)
	:ScreenPass(g, std::move(name), parent), m_SamplerHeap(g, 1)
{
	AddInTarget("HDR");
	AddOutTarget("SDR");

	ConstantBufferLayout layout;
	layout.Add<CONSTANT_TYPE::INT>("On");
	m_Options = MakeShared<ConstantBuffer>(g, std::move(layout));
	(*m_Options)["On"] = 1;
	AddResource(m_Options);

	m_Pipeline = MakeUnique<ToneMapPipeline>(g);
	m_SamplerHeap.Add(g);
}

void ToneMapPass::Run(Graphics& g)
{
	__super::Run(g);

	auto& inTargets = GetInTargets();
	const auto& targets =
		std::views::iota(inTargets.begin(), inTargets.end()) |
		std::views::transform([&](const auto& it) {
		return it->second;
			}) |
		std::ranges::to<std::vector>();

	Resource::Transition(g, targets, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	ID3D12DescriptorHeap* heaps[] = { **m_GPUHeap, *m_SamplerHeap };
	g.CL().SetDescriptorHeaps(2, heaps);
	g.CL().SetGraphicsRootDescriptorTable(0, m_GPUHeap->GPUStart());
	g.CL().SetGraphicsRootDescriptorTable(1, m_SamplerHeap.GPUStart());

	Rasterize(g);

	Resource::Transition(g, targets, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	g.Flush();
}

void ToneMapPass::ShowGUI()
{
	if (ImGui::Begin("Tone Map Pass"))
	{
		ImGui::BeginGroup();
		ImGui::Text("Options");
		ImGui::Checkbox("On", static_cast<bool*>((*m_Options)["On"]));
		ImGui::EndGroup();
	}
	ImGui::End();
}
