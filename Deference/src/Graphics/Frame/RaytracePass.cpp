#include "RaytracePass.h"
#include "Resource/Heap.h"

RaytracePass::RaytracePass(Graphics& g, const std::string& name, ComPtr<ID3D12Resource> topLevelAS, Shared<Camera> cam)
	:BindablePass(std::move(name)), m_Heap(g, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4, true)
{
	AddTarget("rt");

	m_Output = m_Heap.AddResource<UnorderedAccess>(g);
	m_Heap.AddResource<TopLevelAS>(g, std::move(topLevelAS));
	m_Heap.AddResource<ConstantBuffer>(g, cam->Res(), sizeof(Camera::CamData));
}

void RaytracePass::Run(Graphics& g)
{
	m_RT->Bind(g);
	BindablePass::Run(g);
	m_Pipeline->Bind(g);
	m_Heap.Bind(g);

	m_Pipeline->Dispatch(g);
	
	m_Output->Transition(g, D3D12_RESOURCE_STATE_COPY_SOURCE);
	m_RT->Transition(g, D3D12_RESOURCE_STATE_COPY_DEST);
	g.CL().CopyResource(m_RT->Res(), m_Output->Res());

	m_Output->Transition(g, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	m_RT->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void RaytracePass::OnAdd(Graphics& g)
{
	m_RT = GetTarget("rt");
	m_Heap.AddResource<TargetSR>(g, m_RT);
}
