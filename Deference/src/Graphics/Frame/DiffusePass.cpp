#include "DiffusePass.h"
#include "Bindable/Heap/AccelStruct.h"
#include "Bindable/Heap/UnorderedAccess.h"
#include "Bindable/Pipeline/DiffusePipeline.h"
#include "Entity/Camera.h"
#include <imgui.h>

DiffusePass::DiffusePass(Graphics& g, FrameGraph* parent)
	:m_Models(parent->GetModels()), m_FrameCount(0)
{
	AddInTarget("Position");
	AddInTarget("Normal");
	AddInTarget("Albedo");
	AddOutTarget("Diffuse");

	m_TLAS = parent->GetTLAS();
	AddResource(m_TLAS);

	m_Environment = MakeShared<EnvironmentMap>(g, L"textures\\MonValley_G_DirtRoad_3k.hdr");
	AddResource(m_Environment);

	{
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
	{
		ConstantBufferLayout layout;
		layout.Add<CONSTANT_TYPE::UINT>("frameCount");
		m_Constants = MakeShared<ConstantBuffer>(g, std::move(layout));
		AddResource(m_Constants);
	}

	for (auto& model : m_Models)
	{
		for (auto& mesh : model->GetMeshes())
		{
			AddResource(mesh.m_VB);
			AddResource(mesh.m_IB);
			AddResource(mesh.m_DiffuseMap);
			AddResource(mesh.m_NormalMap);
		}
	}
}

void DiffusePass::OnAdd(Graphics& g)
{
	__super::OnAdd(g);
	m_Pipeline = MakeUnique<DiffusePipeline>(g);

	typedef RaytracedGeometryPass::HitArguments HitArgs;
	std::vector<HitArgs> argsList;
	for (auto& model : m_Models)
	{
		for (auto& mesh : model->GetMeshes())
		{
			HitArgs args
			{
				.m_VertexBuffer = mesh.m_VB->GetHGPU(),
				.m_IndexBuffer = mesh.m_IB->GetHGPU(),
				.m_DiffuseMap = mesh.m_DiffuseMap->GetHGPU(),
				.m_NormalMap = mesh.m_NormalMap->GetHGPU()
			};

			argsList.push_back(std::move(args));
		}
	}

	{
		auto table = MakeShared<ShaderBindTable>(g, m_Pipeline.get(), argsList.size(), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + sizeof(HitArgs));
		for (auto& a : argsList)
			table->Add(DiffusePipeline::rayGenEP, &a, sizeof(HitArgs));
		m_Pipeline->SubmitTable(SHADER_TABLE_TYPE::RAY_GEN, std::move(table));
	}
	{
		auto table = MakeShared<ShaderBindTable>(g, m_Pipeline.get(), 2, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + sizeof(HGPU));
		auto env = m_Environment->GetHGPU();
		table->Add(DiffusePipeline::indirectMiss, &env, sizeof(HGPU));
		table->Add(DiffusePipeline::shadowMiss);
		m_Pipeline->SubmitTable(SHADER_TABLE_TYPE::MISS, std::move(table));
	}
	{
		auto table = MakeShared<ShaderBindTable>(g, m_Pipeline.get(), argsList.size() + 1, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + sizeof(HitArgs));
		table->Add(DiffusePipeline::shadowGroup, nullptr, sizeof(HitArgs));
		for (auto& a : argsList)
			table->Add(DiffusePipeline::indirectGroup, &a, sizeof(HitArgs));
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

void DiffusePass::Run(Graphics& g)
{
	m_Pipeline->Bind(g);
	m_GPUHeap->Bind(g);
	g.CL().SetComputeRootDescriptorTable(0, m_GPUHeap->GPUStart());
	g.CL().SetComputeRootDescriptorTable(1, m_TLAS->GetHGPU());
	g.CL().SetComputeRootDescriptorTable(2, GetOutputs()[0]->GetHGPU());
	g.CL().SetComputeRootConstantBufferView(3, m_Light->GetGPUAddress());
	g.CL().SetComputeRootConstantBufferView(4, m_Constants->GetGPUAddress());

	m_Pipeline->Dispatch(g);

	auto& outs = GetOutTargets();
	const auto& targets =
		std::views::iota(0u, (UINT)outs.size()) |
		std::views::transform([&](UINT i) {
		return outs.at(i).second;
			}) |
		std::ranges::to<std::vector>();

	Resource::Transition(g, targets, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
	Resource::Transition(g, m_Outputs, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);

	for (UINT i = 0; i < targets.size(); i++)
		g.CL().CopyResource(**targets[i], **m_Outputs[i]);

	Resource::Transition(g, targets, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
	Resource::Transition(g, m_Outputs, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	g.Flush();

	(*m_Constants)["frameCount"] = m_FrameCount;
}
