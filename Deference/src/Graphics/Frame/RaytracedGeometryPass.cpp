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
	AddOutTarget("Position", DXGI_FORMAT_R32G32B32A32_FLOAT);
	AddOutTarget("Normal", DXGI_FORMAT_R32G32B32A32_FLOAT);
	AddOutTarget("Albedo");
	AddOutTarget("Specular");
	AddOutTarget("Emissive");

	QueryGlobalResource("Env");
	QueryGlobalVectorResource("Models");

	ConstantBufferLayout layout;
	layout.Add<CONSTANT_TYPE::XMFLOAT3>("u");
	layout.Add<CONSTANT_TYPE::FLOAT>("lensRadius");
	layout.Add<CONSTANT_TYPE::XMFLOAT3>("v");
	layout.Add<CONSTANT_TYPE::FLOAT>("focalLength");
	layout.Add<CONSTANT_TYPE::XMFLOAT3>("w");
	layout.Add<CONSTANT_TYPE::UINT>("frameCount");
	layout.Add<CONSTANT_TYPE::XMFLOAT4>("wPos");
	layout.Add<CONSTANT_TYPE::XMFLOAT2>("jitter");
	m_Transform = MakeShared<ConstantBuffer>(g, std::move(layout));
	AddResource(m_Transform);

	auto now = std::chrono::high_resolution_clock::now();
	auto msTime = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
	m_Rng = std::mt19937(uint32_t(msTime.time_since_epoch().count()));
}

void RaytracedGeometryPass::Run(Graphics& g)
{
	static const float kMSAA[8][2] = { { 1,-3 },{ -1,3 },{ 5,1 },{ -3,-5 },{ -5,5 },{ -7,-1 },{ 3,7 },{ 7,-7 } };

	(*m_Transform)["frameCount"] = m_FrameCount++;
	(*m_Transform)["lensRadius"] = 0.f;
	(*m_Transform)["focalLength"] = m_FocalLength;

	// Compute our jitter, either (0,0) as the center or some computed random/MSAA offset
	float xOff = 0.0f, yOff = 0.0f;
	xOff = kMSAA[m_FrameCount % 8][0] * 0.0625f;
	yOff = kMSAA[m_FrameCount % 8][1] * 0.0625f;

	const auto& cam = m_Parent->GetCamera();

	m_LensRadius = m_FocalLength / (2.0f * m_FStop);

	XMStoreFloat3((*m_Transform)["u"], cam->U());
	XMStoreFloat3((*m_Transform)["v"], cam->V());
	XMStoreFloat3((*m_Transform)["w"], cam->W());
	(*m_Transform)["wPos"] = cam->Pos();
	(*m_Transform)["jitter"] = XMFLOAT2{ xOff + 0.5f, yOff + 0.5f };

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
	g.CL().SetComputeRootDescriptorTable(1, GetResource(GetOutput("Position")));
	g.CL().SetComputeRootDescriptorTable(2, GetResource(GetOutput("Normal")));
	g.CL().SetComputeRootDescriptorTable(3, GetResource(GetOutput("Albedo")));
	g.CL().SetComputeRootDescriptorTable(4, GetResource(GetOutput("Specular")));
	g.CL().SetComputeRootDescriptorTable(5, GetResource(GetOutput("Emissive")));

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
