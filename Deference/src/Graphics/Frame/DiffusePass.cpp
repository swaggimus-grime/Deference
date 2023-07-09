#include "DiffusePass.h"
#include "Bindable/Heap/AccelStruct.h"
#include "Bindable/Heap/UnorderedAccess.h"
#include "Bindable/Pipeline/DiffusePipeline.h"
#include "Bindable/Heap/PointLight.h"
#include "Entity/Camera.h"

DiffusePass::DiffusePass(Graphics& g)
	:m_Heap(g, 6)
{
	AddOutTarget("Diffuse");
}

void DiffusePass::OnAdd(Graphics& g, GeometryGraph* parent)
{
	BindablePass::OnAdd(g, parent);

	auto& ins = GetInTargets();
	for (auto& in : ins)
		m_Heap.Add<RTV>(g, in.second);

	auto& drawables = parent->Drawables();
	m_Heap.Add<TLAS>(g, drawables);
	m_Output = m_Heap.Add<UnorderedAccess>(g);
	m_Light = m_Heap.Add<PointLight>(g);

	m_Pipeline = MakeShared<DiffusePipeline>(g);
}

void DiffusePass::Run(Graphics& g, GeometryGraph* parent)
{
	auto diffuse = GetOutTarget("Diffuse");

	m_Pipeline->Bind(g);
	m_Heap.Bind(g);

	UINT64* heapPtr = reinterpret_cast<UINT64*>(m_Heap.GPUStart().ptr);
	m_Pipeline->ComputeShaderTableAndDispatch(g,
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

	m_Output->Transition(g, D3D12_RESOURCE_STATE_COPY_SOURCE);
	diffuse->Transition(g, D3D12_RESOURCE_STATE_COPY_DEST);
	g.CL().CopyResource(diffuse->Res(), m_Output->Res());

	m_Output->Transition(g, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	diffuse->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET);
	g.Flush();
}
