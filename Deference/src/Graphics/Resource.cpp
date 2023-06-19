#include "Resource.h"

Resource::Resource(D3D12_CPU_DESCRIPTOR_HANDLE handle, D3D12_RESOURCE_STATES state, ComPtr<ID3D12Resource> res)
	:m_Handle(handle), m_State(state), m_Res(res)
{
}

void Resource::Transition(Graphics& g, D3D12_RESOURCE_STATES state)
{
	const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_Res.Get(), m_State, state);
	g.CL().ResourceBarrier(1, &barrier);
	m_State = state;
}
