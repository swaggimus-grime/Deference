#include "DiffuseGIPass.h"
#include "Bindable/Heap/AccelStruct.h"
#include "Bindable/Heap/UnorderedAccess.h"
#include "Bindable/Pipeline/DiffuseGIPipeline.h"
#include "Scene/Camera.h"
#include <imgui.h>
#include "VecOps.h"

DiffuseGIPass::DiffuseGIPass(Graphics& g, const std::string& name, FrameGraph* parent)
	:RaytracePass(std::move(name), parent), m_FrameCount(0)
{
	AddInTarget("Position");
	AddInTarget("Normal");
	AddInTarget("Albedo");
	AddInTarget("Specular");
	AddInTarget("Emissive");
	AddOutTarget(g, "Target");

	QueryGlobalResource("Env");
	QueryGlobalVectorResource("Models");

	{
		ConstantBufferLayout layout;
		layout.Add<CONSTANT_TYPE::XMFLOAT3>("pos");
		layout.Add<CONSTANT_TYPE::FLOAT>("intensity");
		layout.Add<CONSTANT_TYPE::XMFLOAT3>("color");
		layout.Add<CONSTANT_TYPE::FLOAT>("emissive");
		layout.Add<CONSTANT_TYPE::UINT>("on");
		m_Light = MakeShared<ConstantBuffer>(g, std::move(layout));
		(*m_Light)["pos"] = XMFLOAT3{ 0, 0.f, 0.f };
		(*m_Light)["color"] = XMFLOAT3{ 1, 1, 1 };
		(*m_Light)["intensity"] = 1.f;
		(*m_Light)["emissive"] = 1.f;
		(*m_Light)["on"] = 1u;
		AddResource(m_Light);
	}
	{
		ConstantBufferLayout layout;
		layout.Add<CONSTANT_TYPE::XMFLOAT3>("camPos");
		layout.Add<CONSTANT_TYPE::UINT>("maxRec");
		layout.Add<CONSTANT_TYPE::UINT>("frameCount");
		layout.Add<CONSTANT_TYPE::FLOAT>("minT");
		layout.Add<CONSTANT_TYPE::UINT>("on");
		m_Constants = MakeShared<ConstantBuffer>(g, std::move(layout));
		(*m_Constants)["maxRec"] = 1;
		(*m_Constants)["minT"] = 0.001f;
		(*m_Constants)["on"] = 1u;

		AddResource(m_Constants);
	}
}

void DiffuseGIPass::OnSceneLoad(Graphics& g)
{
	m_Pipeline = MakeUnique<DiffuseGIPipeline>(g);

	const auto& entries = GetGlobalVectorResource("Models");
	{
		auto table = MakeShared<ShaderBindTable>(g, m_Pipeline.get(), 1, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		table->Add(DiffuseGIPipeline::rayGenEP);
		m_Pipeline->SubmitTable(SHADER_TABLE_TYPE::RAY_GEN, std::move(table));
	}
	{
		auto table = MakeShared<ShaderBindTable>(g, m_Pipeline.get(), 2, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		table->Add(DiffuseGIPipeline::indirectMiss);
		table->Add(DiffuseGIPipeline::shadowMiss);
		m_Pipeline->SubmitTable(SHADER_TABLE_TYPE::MISS, std::move(table));
	}
	{
		auto table = MakeShared<ShaderBindTable>(g, m_Pipeline.get(), entries.size() * 2, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + sizeof(HGPU) * entries[0].size());

		for (UINT i = 0; i < entries.size(); i++)
		{
			table->Add(DiffuseGIPipeline::shadowGroup);
			table->Add(DiffuseGIPipeline::indirectGroup, (void*)entries[i].data(), entries[i].size() * sizeof(HGPU));
		}

		m_Pipeline->SubmitTable(SHADER_TABLE_TYPE::HIT, std::move(table));
	}
}

void DiffuseGIPass::ShowGUI()
{
	auto scene = m_Parent->GetScene();
	if (ImGui::Begin("Diffuse Pass"))
	{
		ImGui::BeginGroup();
		ImGui::Text("Direct Light");
		ImGui::Checkbox("On", (*m_Light)["on"]);
		ImGui::Text("Position");
		auto dim = scene.GetModel().GetBBox().Dim();
		XMFLOAT3& pos = (*m_Light)["pos"];
		ImGui::SliderFloat("X", &pos.x, -dim.x, dim.x);
		ImGui::SliderFloat("Y", &pos.y, -dim.y, dim.y);
		ImGui::SliderFloat("Z", &pos.z, -dim.z, dim.z);
		ImGui::SliderFloat3("Color", (*m_Light)["color"], 0.f, 1.f);
		ImGui::SliderFloat("Intensity", (*m_Light)["intensity"], 0.f, 10.f);
		ImGui::SliderFloat("Emissive", (*m_Light)["emissive"], 0.f, 10.f);
		ImGui::EndGroup();

		ImGui::BeginGroup();
		ImGui::Text("Indirect Light");
		ImGui::Checkbox("On", (*m_Constants)["on"]);
		ImGui::SliderInt("Max Recursion", ((*m_Constants)["maxRec"]), 1, 30);
		ImGui::SliderFloat("minT", (*m_Constants)["minT"], 0.001f, 1.f);
		ImGui::EndGroup();
	}
	ImGui::End();
}

void DiffuseGIPass::Run(Graphics& g)
{
	(*m_Constants)["camPos"] = m_Parent->GetScene().GetCamera().Pos();
	(*m_Constants)["frameCount"] = m_FrameCount++;

	auto& ins = GetInTargets();
	const auto& inTargets =
		std::views::iota(ins.begin(), ins.end()) |
		std::views::transform([&](const auto& it) {
		return std::get<1>(*it);
			}) |
		std::ranges::to<std::vector>();

	Resource::Transition(g, inTargets, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	m_Pipeline->Bind(g);
	m_GPUHeap->Bind(g);
	for(UINT i = 0; i < inTargets.size(); i++)
		g.CL().SetComputeRootDescriptorTable(i, GetTargetResource(inTargets[i]));

	g.CL().SetComputeRootDescriptorTable(5, GetGlobalResource("TLAS"));
	g.CL().SetComputeRootDescriptorTable(6, GetGlobalResource("Env"));
	auto target = GetOutput("Target");
	g.CL().SetComputeRootDescriptorTable(7, GetResource(target));
	g.CL().SetComputeRootConstantBufferView(8, m_Light->GetGPUAddress());
	g.CL().SetComputeRootConstantBufferView(9, m_Constants->GetGPUAddress());

	m_Pipeline->Dispatch(g);

	Resource::Transition(g, inTargets, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	auto& outs = GetOutTargets();
	const auto& targets =
		std::views::iota(outs.begin(), outs.end()) |
		std::views::transform([&](const auto& it) {
			return it->second;
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
