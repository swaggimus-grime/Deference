#include "DiffusePass.h"
#include "Bindable/Heap/AccelStruct.h"
#include "Bindable/Heap/UnorderedAccess.h"
#include "Bindable/Pipeline/DiffusePipeline.h"
#include "Entity/Camera.h"
#include <imgui.h>

DiffusePass::DiffusePass(Graphics& g, FrameGraph* parent)
{
	AddInTarget("Position");
	AddInTarget("Normal");
	AddInTarget("Albedo");
	AddOutTarget("Diffuse");

	m_TLAS = parent->GetTLAS();
	AddResource(m_TLAS);
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

void DiffusePass::OnAdd(Graphics& g)
{
	__super::OnAdd(g);
	m_Pipeline = MakeUnique<DiffusePipeline>(g);

	{
		struct GenArgs
		{
			HGPU m_Pos;
			HGPU m_Norm;
			HGPU m_Albedo;
			HGPU m_Scene;
			HGPU m_Light;
			HGPU m_Output;
		} args;

		args.m_Pos = GetInTarget("Position")->GetShaderHGPU();
		args.m_Norm = GetInTarget("Normal")->GetShaderHGPU();
		args.m_Albedo = GetInTarget("Albedo")->GetShaderHGPU();
		args.m_Scene = m_TLAS->GetHGPU();
		args.m_Light = m_Light->GetHGPU();
		args.m_Output = GetOutput("Diffuse")->GetHGPU();

		auto table = MakeShared<ShaderBindTable>(g, m_Pipeline.get(), 1, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + sizeof(GenArgs));
		table->Add(DiffusePipeline::rayGenEP, &args, sizeof(GenArgs));
		m_Pipeline->SubmitTable(SHADER_TABLE_TYPE::RAY_GEN, std::move(table));
	}
	{
		auto table = MakeShared<ShaderBindTable>(g, m_Pipeline.get(), 1, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		table->Add(DiffusePipeline::missEP);
		m_Pipeline->SubmitTable(SHADER_TABLE_TYPE::MISS, std::move(table));
	}
	{
		auto table = MakeShared<ShaderBindTable>(g, m_Pipeline.get(), 1, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		table->Add(DiffusePipeline::hitGroup);
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