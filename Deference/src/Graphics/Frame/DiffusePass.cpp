#include "DiffusePass.h"
#include "Bindable/Heap/AccelStruct.h"
#include "Bindable/Heap/UnorderedAccess.h"
#include "Bindable/Pipeline/DiffusePipeline.h"
#include "Entity/Camera.h"
#include <imgui.h>

DiffusePass::DiffusePass(Graphics& g, const std::string& name, FrameGraph* parent)
	:RaytracePass(std::move(name), parent)
{
	AddInTarget("Position");
	AddInTarget("Normal");
	AddInTarget("Albedo");
	AddInTarget("Emissive");
	AddOutTarget("Target");

	ConstantBufferLayout layout;
	layout.Add<CONSTANT_TYPE::XMFLOAT3>("pos");
	layout.Add<CONSTANT_TYPE::FLOAT>("intensity");
	layout.Add<CONSTANT_TYPE::XMFLOAT3>("color");
	m_Light = MakeShared<ConstantBuffer>(g, std::move(layout));
	(*m_Light)["pos"] = XMFLOAT3{ 0, 5.f, 10.f };
	(*m_Light)["color"] = XMFLOAT3{ 1, 1, 1 };
	(*m_Light)["intensity"] = 2.f;
	AddResource(m_Light);
}

void DiffusePass::Run(Graphics& g)
{
	m_Pipeline->Bind(g);
	m_GPUHeap->Bind(g);

	m_Pipeline->Dispatch(g);

	auto& outs = GetOutTargets();
	const auto& targets =
	std::views::iota(outs.begin(), outs.end()) |
	std::views::transform([&](const auto& it) {
		return std::get<2>(*it);
	}) |
	std::ranges::to<std::vector>();

	const auto& uas =
		std::views::iota(m_Outputs.begin(), m_Outputs.end()) |
		std::views::transform([&](const auto& it) {
		return it->second;
			}) |
		std::ranges::to<std::vector>();

	Resource::Transition(g, targets, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
	Resource::Transition(g, uas, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);

	for (UINT i = 0; i < targets.size(); i++)
		g.CL().CopyResource(**targets[i], **uas[i]);

	Resource::Transition(g, targets, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
	Resource::Transition(g, uas, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	g.Flush();
}

void DiffusePass::Finish(Graphics& g)
{
	__super::Finish(g);
	m_Pipeline = MakeUnique<DiffusePipeline>(g);

	{
		struct GenArgs
		{
			HGPU m_Pos;
			HGPU m_Norm;
			HGPU m_Albedo;
			HGPU m_Emissive;
			HGPU m_Scene;
			HGPU m_Light;
			HGPU m_Output;
		} args;

		args.m_Pos = GetResource(GetInTarget("Position"));
		args.m_Norm = GetResource(GetInTarget("Normal"));
		args.m_Albedo = GetResource(GetInTarget("Albedo"));
		args.m_Emissive = GetResource(GetInTarget("Emissive"));
		args.m_Scene = GetGlobalResource("TLAS");
		args.m_Light = GetResource(m_Light);
		args.m_Output = GetResource(GetOutput("Diffuse"));

		auto table = MakeShared<ShaderBindTable>(g, m_Pipeline.get(), 1, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + sizeof(GenArgs));
		table->Add(DiffusePipeline::rayGenEP, &args, sizeof(GenArgs));
		m_Pipeline->SubmitTable(SHADER_TABLE_TYPE::RAY_GEN, std::move(table));
	}
	{
		auto table = MakeShared<ShaderBindTable>(g, m_Pipeline.get(), 1, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		table->Add(DiffusePipeline::shadowMiss);
		m_Pipeline->SubmitTable(SHADER_TABLE_TYPE::MISS, std::move(table));
	}
	{
		auto table = MakeShared<ShaderBindTable>(g, m_Pipeline.get(), 1, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		table->Add(DiffusePipeline::shadowGroup);
		m_Pipeline->SubmitTable(SHADER_TABLE_TYPE::HIT, std::move(table));
	}
}

void DiffusePass::ShowGUI()
{
	if (ImGui::Begin("Diffuse Pass"))
	{
		ImGui::BeginGroup();
		ImGui::Text("Point Light");

		ImGui::SliderFloat3("Position", ((*m_Light)["pos"]), -10.f, 10.f);
		ImGui::SliderFloat3("Color", (*m_Light)["color"], 0.f, 1.f);
		ImGui::SliderFloat("Intensity", (*m_Light)["intensity"], 0.f, 5.f);
		ImGui::EndGroup();

		ImGui::End();
	}
}