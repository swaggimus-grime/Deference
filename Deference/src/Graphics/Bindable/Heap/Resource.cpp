#include "Resource.h"

void Resource::CopyViews(Graphics& g, HCPU dest, HCPU src, UINT num, D3D12_DESCRIPTOR_HEAP_TYPE type)
{
	g.Device().CopyDescriptorsSimple(num, dest, src, type);
}


Resource::Resource()
	:m_Res(nullptr), m_HCPU({0})
{
}

void Resource::Transition(Graphics& g, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
	const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_Res.Get(), before, after);
	g.CL().ResourceBarrier(1, &barrier);
}