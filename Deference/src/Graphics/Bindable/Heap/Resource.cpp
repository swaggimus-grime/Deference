#include "Resource.h"

void Resource::Transition(Graphics& g, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
	const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_Res.Get(), before, after);
	g.CL().ResourceBarrier(1, &barrier);
}

void Resource::CopyView(Graphics& g, HCPU hcpu)
{
	g.Device().CopyDescriptorsSimple(1, hcpu, m_HCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

Resource::Resource()
{
	m_Res = nullptr;
	m_HCPU.ptr = 0;
}
