#include "RaytracedGeometryPass.h"
#include "Bindable/Pipeline/RaytracedGeometryPipeline.h"
#include <ranges>
#include "FrameGraph.h"
#include "Entity/Model.h"
#include "Entity/Camera.h"
#include <ranges>

RaytracedGeometryPass::RaytracedGeometryPass(Graphics& g, FrameGraph* parent)
	:m_Models(std::move(parent->GetModels())), m_Cam(parent->GetCamera())
{	
	AddOutTarget("Position");
	AddOutTarget("Normal");
	AddOutTarget("Albedo");

	m_TLAS = parent->GetTLAS();
	AddResource(m_TLAS);
	m_Environment = MakeShared<EnvironmentMap>(g, L"textures\\MonValley_G_DirtRoad_3k.hdr");
	AddResource(m_Environment);

	ConstantBufferLayout layout;
	layout.Add<CONSTANT_TYPE::XMMATRIX>("viewInv");
	layout.Add<CONSTANT_TYPE::XMMATRIX>("projInv");
	layout.Add<CONSTANT_TYPE::XMFLOAT3>("wPos");
	m_Transform = MakeShared<ConstantBuffer>(g, std::move(layout));
	AddResource(m_Transform);

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

void RaytracedGeometryPass::Run(Graphics& g)
{
	(*m_Transform)["viewInv"] = XMMatrixTranspose(m_Cam->ViewInv());
	(*m_Transform)["projInv"] = XMMatrixTranspose(m_Cam->ProjInv());
	(*m_Transform)["wPos"] = m_Cam->Pos();

	m_Pipeline->Bind(g);
	m_GPUHeap->Bind(g);
	g.CL().SetComputeRootConstantBufferView(0, m_Transform->GetGPUAddress());
	g.CL().SetComputeRootDescriptorTable(1, GetOutputs()[0]->GetHGPU());

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
}

void RaytracedGeometryPass::OnAdd(Graphics& g)
{
	__super::OnAdd(g);

	m_Pipeline = MakeUnique<RaytracedGeometryPipeline>(g);
	{
		auto table = MakeShared<ShaderBindTable>(g, m_Pipeline.get(), 1, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + sizeof(HGPU));
		auto scene = m_TLAS->GetHGPU();
		table->Add(RaytracedGeometryPipeline::rayGenEP, &scene, sizeof(HGPU)); //scene
		m_Pipeline->SubmitTable(SHADER_TABLE_TYPE::RAY_GEN, std::move(table));
	}
	{
		auto table = MakeShared<ShaderBindTable>(g, m_Pipeline.get(), 1, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + sizeof(HGPU));	
		auto env = m_Environment->GetHGPU();
		table->Add(RaytracedGeometryPipeline::missEP, &env, sizeof(HGPU));
		m_Pipeline->SubmitTable(SHADER_TABLE_TYPE::MISS, std::move(table));
	}
	{
		std::vector<HitArguments> argsList;
		for (auto& model : m_Models)
		{
			for (auto& mesh : model->GetMeshes())
			{
				HitArguments args
				{
					.m_VertexBuffer = mesh.m_VB->GetHGPU(),
					.m_IndexBuffer = mesh.m_IB->GetHGPU(),
					.m_DiffuseMap = mesh.m_DiffuseMap->GetHGPU(),
					.m_NormalMap = mesh.m_NormalMap->GetHGPU()
				};

				argsList.push_back(std::move(args));
			}
		}
		auto table = MakeShared<ShaderBindTable>(g, m_Pipeline.get(), argsList.size(), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + sizeof(HitArguments));
		for (auto& a : argsList)
			table->Add(RaytracedGeometryPipeline::hitGroup, &a, sizeof(HitArguments));
		m_Pipeline->SubmitTable(SHADER_TABLE_TYPE::HIT, std::move(table));
	}

}

void RaytracedGeometryPass::ShowGUI()
{
}
