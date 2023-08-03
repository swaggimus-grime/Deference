#include "Resource.h"

void Resource::Transition(Graphics& g, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
	const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_Res.Get(), before, after);
	g.CL().ResourceBarrier(1, &barrier);
}

Resource::Resource()
{
	m_Res = nullptr;
	m_Handle.m_HCPU.ptr = 0;
	m_Handle.m_HGPU.ptr = 0;
}
