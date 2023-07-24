#include "AOPass.h"
#include "Bindable/Heap/AccelStruct.h"
#include "Bindable/Heap/UnorderedAccess.h"
#include "Bindable/Pipeline/AOPipeline.h"
#include "Entity/Camera.h"
#include <imgui.h>

AOPass::AOPass(Graphics& g)
	:m_FrameCount(0)
{
	AddInTarget("Position");
	AddInTarget("Normal");

	AddOutTarget("AO");
}

void AOPass::OnAdd(Graphics& g, FrameGraph* parent)
{
	Pass::OnAdd(g, parent);

	auto& ins = GetInTargets();
	const auto& models = parent->GetModels();

	m_GPUHeap = MakeUnique<GPUShaderHeap>(g, ins.size() + 3);

	for (auto& in : ins)
		in.second->CreateShaderResourceView(g, m_GPUHeap->Next());
	parent->GetTLAS().CreateView(g, m_GPUHeap->Next());

	m_Output = MakeUnique<UnorderedAccess>(g);
	m_Output->CreateView(g, m_GPUHeap->Next());
	
	ConstantBufferLayout layout;
	layout.Add<CONSTANT_TYPE::FLOAT>("radius");
	layout.Add<CONSTANT_TYPE::FLOAT>("minT");
	layout.Add<CONSTANT_TYPE::UINT>("frameCount");
	m_Constants = MakeUnique<ConstantBuffer>(g, std::move(layout));
	m_Constants->CreateView(g, m_GPUHeap->Next());

	(*m_Constants)["radius"] = 100.f;
	(*m_Constants)["minT"] = 0.0001f;

	m_Pipeline = MakeShared<AOPipeline>(g);
	UINT64* heapPtr = reinterpret_cast<UINT64*>(m_GPUHeap->GPUStart().ptr);
	m_Pipeline->UpdateTable(g,
		{
			{AOPipeline::rayGenEP, {heapPtr}}
		},
		{
			{AOPipeline::missEP, {}}
		},
		{
			{AOPipeline::hitGroup, {}}
		}
	);
}

void AOPass::Run(Graphics& g, FrameGraph* parent)
{
	auto ao = GetOutTarget("AO");

	m_Pipeline->Bind(g);
	m_GPUHeap->Bind(g);

	(*m_Constants)["frameCount"] = m_FrameCount++;

	m_Pipeline->Dispatch(g);

	m_Output->Transition(g, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	ao->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
	g.CL().CopyResource(**ao, **m_Output);

	m_Output->Transition(g, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	ao->Transition(g, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
	g.Flush();
}

void AOPass::ShowGUI()
{
	if (ImGui::Begin("AO Pass"))
	{
		ImGui::SliderFloat("Radius", ((*m_Constants)["radius"]), 10.f, 1000.f);
		ImGui::SliderFloat("MinT", (*m_Constants)["minT"], 0.0001f, 1.f);

		ImGui::End();
	}
}
