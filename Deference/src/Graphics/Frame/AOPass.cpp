//#include "AOPass.h"
//#include "Bindable/Heap/AccelStruct.h"
//#include "Bindable/Pipeline/AOPipeline.h"
//#include "Scene/Camera.h"
//#include <imgui.h>
//
//AOPass::AOPass(Graphics& g, const std::string& name, FrameGraph* parent)
//	:RaytracePass(std::move(name), parent), m_FrameCount(0)
//{
//	AddInTarget("Position");
//	AddInTarget("Normal");
//	AddOutTarget("Target");
//
//	ConstantBufferLayout layout;
//	layout.Add<CONSTANT_TYPE::FLOAT>("radius");
//	layout.Add<CONSTANT_TYPE::FLOAT>("minT");
//	layout.Add<CONSTANT_TYPE::UINT>("frameCount");
//	layout.Add<CONSTANT_TYPE::UINT>("rayCount");
//	m_Constants = MakeShared<ConstantBuffer>(g, std::move(layout));
//	AddResource(m_Constants);
//	(*m_Constants)["radius"] = 20.f;
//	(*m_Constants)["minT"] = 0.001f;
//	(*m_Constants)["rayCount"] = 1;
//}
//
//void AOPass::Compile(Graphics& g)
//{
//	__super::Compile(g);
//
//	m_Pipeline = MakeUnique<AOPipeline>(g);
//	{
//		struct RayGenArgs
//		{
//			HGPU m_Pos;
//			HGPU m_Norm;
//			HGPU m_Scene;
//			HGPU m_Constants;
//			HGPU m_Output;
//		} args;
//		args.m_Pos = GetResource(GetInTarget("Position"));
//		args.m_Norm = GetResource(GetInTarget("Normal"));
//		args.m_Scene = GetGlobalResource("TLAS");
//		args.m_Constants = GetResource(m_Constants);
//		args.m_Output = GetResource(GetOutput("Target"));
//
//		auto table = MakeShared<ShaderBindTable>(g, m_Pipeline.get(), 1, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + sizeof(RayGenArgs));
//		table->Add(AOPipeline::rayGenEP, &args, sizeof(RayGenArgs));
//		m_Pipeline->SubmitTable(SHADER_TABLE_TYPE::RAY_GEN, std::move(table));
//	}
//	{
//		auto table = MakeShared<ShaderBindTable>(g, m_Pipeline.get(), 1, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
//		table->Add(AOPipeline::missEP);
//		m_Pipeline->SubmitTable(SHADER_TABLE_TYPE::MISS, std::move(table));
//	}
//	{
//		auto table = MakeShared<ShaderBindTable>(g, m_Pipeline.get(), 1, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
//		table->Add(AOPipeline::hitGroup);
//		m_Pipeline->SubmitTable(SHADER_TABLE_TYPE::HIT, std::move(table));
//	}
//}
//
//void AOPass::Run(Graphics& g)
//{
//	m_Pipeline->Bind(g);
//	m_GPUHeap->Bind(g);
//
//	m_Pipeline->Dispatch(g);
//
//	auto& outs = GetOutTargets();
//	const auto& targets =
//		std::views::iota(outs.begin(), outs.end()) |
//		std::views::transform([&](const auto& it) {
//		return std::get<2>(*it);
//			}) |
//		std::ranges::to<std::vector>();
//
//	const auto& uas =
//		std::views::iota(m_Outputs.begin(), m_Outputs.end()) |
//		std::views::transform([&](const auto& it) {
//		return it->second;
//			}) |
//		std::ranges::to<std::vector>();
//
//	Resource::Transition(g, targets, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
//	Resource::Transition(g, uas, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
//
//	for (UINT i = 0; i < targets.size(); i++)
//		g.CL().CopyResource(**targets[i], **uas[i]);
//
//	Resource::Transition(g, targets, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
//	Resource::Transition(g, uas, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//
//	g.Flush();
//	(*m_Constants)["frameCount"] = m_FrameCount++;
//}
//
//void AOPass::ShowGUI()
//{
//	if (ImGui::Begin("AO Pass"))
//	{
//		ImGui::SliderFloat("Radius", ((*m_Constants)["radius"]), 10.f, 1000.f);
//		ImGui::SliderFloat("MinT", (*m_Constants)["minT"], 0.001f, 1.f);
//		ImGui::SliderInt("Ray Count", (*m_Constants)["rayCount"], 1, 100);
//	}
//	ImGui::End();
//}