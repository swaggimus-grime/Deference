#pragma once

#include <concepts>
#include <type_traits>
#include <d3d12.h>
#include <wrl.h>

namespace Def
{
	class Resource
	{
	public:
		virtual ID3D12Resource* operator*() const { return m_Res.Get(); }
		void Transition(Graphics& g, D3D12_RESOURCE_STATES after);
		CD3DX12_RESOURCE_BARRIER Transition(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
		CD3DX12_RESOURCE_BARRIER Transition(D3D12_RESOURCE_STATES after);
		inline D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const { return m_Res->GetGPUVirtualAddress(); }

		static void Transition(Graphics& g, const std::vector<D3D12_RESOURCE_BARRIER>& barriers)
		{
			g.CL().ResourceBarrier(barriers.size(), barriers.data());
		}
	protected:
		Resource() = default;

	protected:
		ComPtr<ID3D12Resource> m_Res;
		D3D12_RESOURCE_STATES m_State;
	};

}