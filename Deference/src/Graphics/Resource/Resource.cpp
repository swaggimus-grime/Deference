#include "Resource.h"

namespace Def
{
	void Resource::Transition(Graphics& g, D3D12_RESOURCE_STATES after)
	{
		auto t = Transition(after);
		g.CL().ResourceBarrier(1, &t);
	}

	CD3DX12_RESOURCE_BARRIER Resource::Transition(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
	{
		m_State = after;
		return CD3DX12_RESOURCE_BARRIER::Transition(m_Res.Get(), before, after);
	}

	CD3DX12_RESOURCE_BARRIER Resource::Transition(D3D12_RESOURCE_STATES after)
	{
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_Res.Get(), m_State, after);
		m_State = after;
		return barrier;
	}
}