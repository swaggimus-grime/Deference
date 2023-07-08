#include "DiffusePass.h"
#include "Bindable/Heap/AccelStruct.h"
#include "Bindable/Heap/UnorderedAccess.h"
#include "Bindable/Pipeline/DiffusePipeline.h"

DiffusePass::DiffusePass(Graphics& g)
	:m_SucHeap(g, 5)
{
	AddOutTarget("Diffuse");
}

void DiffusePass::OnAdd(Graphics& g, GeometryGraph* parent)
{
	BindablePass::OnAdd(g, parent);

	auto& ins = GetInTargets();
	for (auto& in : ins)
		m_SucHeap.Add<RTV>(g, in.second);

	auto& drawables = parent->Drawables();
	m_SucHeap.Add<TLAS>(g, drawables);
	m_Output = m_SucHeap.Add<UnorderedAccess>(g);

	m_Pipeline = MakeShared<DiffusePipeline>(g, m_SucHeap);
}

void DiffusePass::Run(Graphics& g, GeometryGraph* parent)
{
	auto diffuse = GetOutTarget("Diffuse");

	m_Pipeline->Bind(g);
	m_SucHeap.Bind(g);
	m_Pipeline->Dispatch(g);

	m_Output->Transition(g, D3D12_RESOURCE_STATE_COPY_SOURCE);
	diffuse->Transition(g, D3D12_RESOURCE_STATE_COPY_DEST);
	g.CL().CopyResource(diffuse->Res(), m_Output->Res());

	m_Output->Transition(g, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	diffuse->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET);
	g.Flush();
}
