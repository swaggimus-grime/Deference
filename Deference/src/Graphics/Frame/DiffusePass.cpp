#include "DiffusePass.h"
#include "Bindable/Heap/AccelStruct.h"
#include "Bindable/Heap/UnorderedAccess.h"
#include "Bindable/Pipeline/DiffusePipeline.h"
#include "Entity/Camera.h"
#include <imgui.h>

DiffusePass::DiffusePass(Graphics& g)
{
	AddInTarget("Position");
	AddInTarget("Normal");
	AddInTarget("Albedo");

	AddOutTarget("Diffuse");
}

void DiffusePass::OnAdd(Graphics& g, FrameGraph* parent)
{
	Pass::OnAdd(g, parent);

	auto& ins = GetInTargets();
	const auto& models = parent->GetModels();

	m_GPUHeap = MakeUnique<GPUShaderHeap>(g, ins.size() + models.size() + 2);
	for (auto& in : ins)
		in.second->CreateShaderResourceView(g, m_GPUHeap->Next());

	parent->GetTLAS().CreateView(g, m_GPUHeap->Next());

	m_Output = MakeUnique<UnorderedAccess>(g);
	m_Output->CreateView(g, m_GPUHeap->Next());

	ConstantBufferLayout layout;
	layout.Add<CONSTANT_TYPE::XMFLOAT3>("pos");
	layout.Add<CONSTANT_TYPE::FLOAT>("intensity");
	layout.Add<CONSTANT_TYPE::XMFLOAT3>("color");
	m_Light = MakeUnique<ConstantBuffer>(g, std::move(layout));
	m_Light->CreateView(g, m_GPUHeap->Next());

	(*m_Light)["pos"] = XMFLOAT3{ 0, 5.f, 10.f };
	(*m_Light)["color"] = XMFLOAT3{ 1, 1, 1 };
	(*m_Light)["intensity"] = 2.f;

	m_Pipeline = MakeShared<DiffusePipeline>(g);
	UINT64* heapPtr = reinterpret_cast<UINT64*>(m_GPUHeap->GPUStart().ptr);
	m_Pipeline->UpdateTable(g,
		{
			{DiffusePipeline::rayGenEP, {heapPtr}}
		},
		{
			{DiffusePipeline::missEP, {}}
		},
		{
			{DiffusePipeline::hitGroup, {}}
		}
	);
}

void DiffusePass::Run(Graphics& g, FrameGraph* parent)
{
	auto diffuse = GetOutTarget("Diffuse");

	m_Pipeline->Bind(g);
	m_GPUHeap->Bind(g);

	m_Pipeline->Dispatch(g);

	m_Output->Transition(g, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	diffuse->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
	g.CL().CopyResource(**diffuse, **m_Output);

	m_Output->Transition(g, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	diffuse->Transition(g, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
	g.Flush();
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

void DiffusePass::OnResize(Graphics& g, UINT w, UINT h)
{
	Pass::OnResize(g, w, h);
	m_Output->Resize(g, w, h);

	m_GPUHeap->Reset();
	auto& ins = GetInTargets();
	for (auto& in : ins)
		in.second->CreateShaderResourceView(g, m_GPUHeap->Next());
}
