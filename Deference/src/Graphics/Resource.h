#pragma once

#include <concepts>
#include <type_traits>

class Resource
{
public:
	Resource(D3D12_CPU_DESCRIPTOR_HANDLE handle, D3D12_RESOURCE_STATES state, ComPtr<ID3D12Resource> res = nullptr);
	auto& Handle() const { return m_Handle; }
	void Transition(Graphics& g, D3D12_RESOURCE_STATES state);
	auto Res() const { return m_Res.Get(); }

protected:
	ComPtr<ID3D12Resource> m_Res;
	D3D12_RESOURCE_STATES m_State;
	D3D12_CPU_DESCRIPTOR_HANDLE m_Handle;
};