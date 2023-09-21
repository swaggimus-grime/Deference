#include "RaytracedGeometryPass.h"
#include "Bindable/Pipeline/RaytracedGeometryPipeline.h"
#include <ranges>
#include "FrameGraph.h"
#include "Entity/Model.h"
#include "Entity/Camera.h"
#include <ranges>

RaytracedGeometryPass::RaytracedGeometryPass(Graphics& g, const std::string& name, FrameGraph* parent)
	:RaytracePass(std::move(name), parent)
{	
	AddOutTarget("Position");
	AddOutTarget("Normal");
	AddOutTarget("Albedo");

	QueryGlobalResource("Env");
	QueryGlobalVectorResource("Models");

	ConstantBufferLayout layout;
	layout.Add<CONSTANT_TYPE::XMMATRIX>("projToWorld");
	layout.Add<CONSTANT_TYPE::XMMATRIX>("worldToProj");
	layout.Add<CONSTANT_TYPE::XMFLOAT3>("wPos");
	m_Transform = MakeShared<ConstantBuffer>(g, std::move(layout));
	AddResource(m_Transform);
}

void RaytracedGeometryPass::Run(Graphics& g)
{
	const auto& cam = m_Parent->GetCamera();
	(*m_Transform)["projToWorld"] = XMMatrixTranspose(cam->ProjToWorld());
	(*m_Transform)["worldToProj"] = XMMatrixTranspose(cam->WorldToProj());
	(*m_Transform)["wPos"] = cam->Pos();

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

	m_Pipeline->Bind(g);
	m_GPUHeap->Bind(g);
	g.CL().SetComputeRootConstantBufferView(0, m_Transform->GetGPUAddress());
	g.CL().SetComputeRootDescriptorTable(1, GetResource(uas[0]));

	m_Pipeline->Dispatch(g);

	Resource::Transition(g, targets, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
	Resource::Transition(g, uas, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	
	for (UINT i = 0; i < targets.size(); i++)
		g.CL().CopyResource(**targets[i], **uas[i]);
	
	Resource::Transition(g, targets, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
	Resource::Transition(g, uas, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	
	g.Flush();
}

void RaytracedGeometryPass::Finish(Graphics& g)
{
	__super::Finish(g);

	m_Pipeline = MakeUnique<RaytracedGeometryPipeline>(g);
	{
		auto table = MakeShared<ShaderBindTable>(g, m_Pipeline.get(), 1, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + sizeof(HGPU));
		table->Add(RaytracedGeometryPipeline::rayGenEP, (void*)&GetGlobalResource("TLAS"), sizeof(HGPU)); //scene
		m_Pipeline->SubmitTable(SHADER_TABLE_TYPE::RAY_GEN, std::move(table));
	}
	{
		auto table = MakeShared<ShaderBindTable>(g, m_Pipeline.get(), 1, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + sizeof(HGPU));	
		table->Add(RaytracedGeometryPipeline::missEP, (void*)&GetGlobalResource("Env"), sizeof(HGPU));
		m_Pipeline->SubmitTable(SHADER_TABLE_TYPE::MISS, std::move(table));
	}
	{
		const auto& entries = GetGlobalVectorResource("Models");
		auto table = MakeShared<ShaderBindTable>(g, m_Pipeline.get(), entries.size(), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + sizeof(HGPU) * entries[0].size());
		for (auto& entry : entries)
			table->Add(RaytracedGeometryPipeline::hitGroup, (void*)entry.data(), entry.size() * sizeof(HGPU));
		
		m_Pipeline->SubmitTable(SHADER_TABLE_TYPE::HIT, std::move(table));
	}

}

void RaytracedGeometryPass::ShowGUI()
{
}
