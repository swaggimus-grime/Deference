#include "AOPass.h"
#include "Bindable/Heap/AccelStruct.h"
#include "Bindable/Pipeline/AOPipeline.h"
#include "Entity/Camera.h"
#include <imgui.h>

AOPass::AOPass(Graphics& g, FrameGraph* parent)
	:m_FrameCount(0)
{
	m_TLAS = parent->GetTLAS();
	AddResource(m_TLAS);

	AddInTarget("Position");
	AddInTarget("Normal");
	AddOutTarget("AO");

	ConstantBufferLayout layout;
	layout.Add<CONSTANT_TYPE::FLOAT>("radius");
	layout.Add<CONSTANT_TYPE::FLOAT>("minT");
	layout.Add<CONSTANT_TYPE::UINT>("frameCount");
	layout.Add<CONSTANT_TYPE::UINT>("rayCount");
	m_Constants = MakeShared<ConstantBuffer>(g, std::move(layout));
	AddResource(m_Constants);
	(*m_Constants)["radius"] = 100.f;
	(*m_Constants)["minT"] = 0.001f;
	(*m_Constants)["rayCount"] = 1;
}

void AOPass::OnAdd(Graphics& g)
{
	__super::OnAdd(g);

	m_Pipeline = MakeUnique<AOPipeline>(g);

	{
		struct RayGenArgs
		{
			HGPU m_Pos;
			HGPU m_Norm;
			HGPU m_Scene;
			HGPU m_Constants;
			HGPU m_Output;
		} args;
		args.m_Pos = GetInTarget("Position")->GetShaderHGPU();
		args.m_Norm = GetInTarget("Normal")->GetShaderHGPU();
		args.m_Scene = m_TLAS->GetHGPU();
		args.m_Constants = m_Constants->GetHGPU();
		args.m_Output = GetOutput("AO")->GetHGPU();

		auto table = MakeShared<ShaderBindTable>(g, m_Pipeline.get(), 1, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + sizeof(RayGenArgs));
		table->Add(AOPipeline::rayGenEP, &args, sizeof(RayGenArgs));
		m_Pipeline->SubmitTable(SHADER_TABLE_TYPE::RAY_GEN, std::move(table));
	}
	{
		auto table = MakeShared<ShaderBindTable>(g, m_Pipeline.get(), 1, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		table->Add(AOPipeline::missEP);
		m_Pipeline->SubmitTable(SHADER_TABLE_TYPE::MISS, std::move(table));
	}
	{
		auto table = MakeShared<ShaderBindTable>(g, m_Pipeline.get(), 1, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		table->Add(AOPipeline::hitGroup);
		m_Pipeline->SubmitTable(SHADER_TABLE_TYPE::HIT, std::move(table));
	}
}

void AOPass::Run(Graphics& g)
{
	__super::Run(g);
	(*m_Constants)["frameCount"] = m_FrameCount++;
}

void AOPass::ShowGUI()
{
	if (ImGui::Begin("AO Pass"))
	{
		ImGui::SliderFloat("Radius", ((*m_Constants)["radius"]), 10.f, 1000.f);
		ImGui::SliderFloat("MinT", (*m_Constants)["minT"], 0.001f, 1.f);
		ImGui::SliderInt("Ray Count", (*m_Constants)["rayCount"], 1, 100);

		ImGui::End();
	}
}