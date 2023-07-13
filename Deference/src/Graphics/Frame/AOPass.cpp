#include "AOPass.h"
#include "Bindable/Heap/AccelStruct.h"
#include "Bindable/Heap/UnorderedAccess.h"
#include "Bindable/Pipeline/AOPipeline.h"
#include "Bindable/Heap/PointLight.h"
#include "Entity/Camera.h"
#include "Bindable/Heap/AOConstants.h"

AOPass::AOPass(Graphics& g)
	:m_Heap(g, 5), m_FrameCount(0)
{
	AddInTarget("Position");
	AddInTarget("Normal");

	AddOutTarget("AO");
}

void AOPass::OnAdd(Graphics& g, GeometryGraph* parent)
{
	Pass::OnAdd(g, parent);

	auto& ins = GetInTargets();
	for (auto& in : ins)
		m_Heap.Add<RTV>(g, in.second);
	auto& drawables = parent->Drawables();
	m_Heap.Add<TLAS>(g, drawables);
	m_Output = m_Heap.Add<UnorderedAccess>(g);
	m_AOConstants = m_Heap.Add<AOConstants>(g);

	m_Pipeline = MakeShared<AOPipeline>(g);
	UINT64* heapPtr = reinterpret_cast<UINT64*>(m_Heap.GPUStart().ptr);
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

void AOPass::Run(Graphics& g, GeometryGraph* parent)
{
	auto ao = GetOutTarget("AO");

	m_Pipeline->Bind(g);
	m_Heap.Bind(g);

	m_AOConstants->Update(m_FrameCount++);

	m_Pipeline->Dispatch(g);

	m_Output->Transition(g, D3D12_RESOURCE_STATE_COPY_SOURCE);
	ao->Transition(g, D3D12_RESOURCE_STATE_COPY_DEST);
	g.CL().CopyResource(ao->Res(), m_Output->Res());

	m_Output->Transition(g, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	ao->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET);
	g.Flush();
}
