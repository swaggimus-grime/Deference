#pragma once

#include <concepts>
#include <type_traits>
#include <d3d12.h>
#include <wrl.h>

class Resource
{
public:
	inline ID3D12Resource* operator*() const { return m_Res.Get(); }
	void Transition(Graphics& g, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

	template<typename T>
	static void Transition(Graphics& g, const std::vector<Shared<T>>& resources,
		D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
		requires Derived<Resource, T>
	{
		std::vector<D3D12_RESOURCE_BARRIER> barriers(resources.size());

		UINT i = 0;
		for (const auto& res : resources)
			barriers[i++] = CD3DX12_RESOURCE_BARRIER::Transition(**res, before, after);

		g.CL().ResourceBarrier(i, barriers.data());
	}

	inline auto GetHCPU() const { return m_HCPU; }

	virtual void CreateView(Graphics& g, HCPU h) = 0;

	void CopyView(Graphics& g, HCPU hcpu);

protected:
	Resource();
	inline void SetHCPU(HCPU h) { m_HCPU = h; }

protected:
	ComPtr<ID3D12Resource> m_Res;
	HCPU m_HCPU;
};
