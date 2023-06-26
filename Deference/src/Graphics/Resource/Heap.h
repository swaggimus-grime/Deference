#pragma once

#include "Debug/Exception.h"
#include "Graphics.h"
#include "Resource.h"

template<typename R>
	requires Derived<Resource, R> || SameClass<Resource, R>
class Heap
{
public:
	Heap(Graphics& g, D3D12_DESCRIPTOR_HEAP_TYPE type, unsigned int numDescs, bool shaderVisible = false)
	{
		BR (numDescs > 0);
		D3D12_DESCRIPTOR_HEAP_DESC dd = {};
		dd.NumDescriptors = numDescs;
		dd.Type = type;
		dd.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		HR g.Device().CreateDescriptorHeap(&dd, IID_PPV_ARGS(&m_Heap));

		m_IncSize = g.Device().GetDescriptorHandleIncrementSize(type);
		m_Handle = m_Heap->GetCPUDescriptorHandleForHeapStart();
		m_Resources.reserve(numDescs);
	}

	void Bind(Graphics& g)
	{
		g.CL().SetDescriptorHeaps(1,
			m_Heap.GetAddressOf());
	}

	auto& Handle() const { return m_Handle; }
	auto GPUHandle() const { return m_Heap->GetGPUDescriptorHandleForHeapStart(); }
	ID3D12DescriptorHeap* GetHeap() const { return m_Heap.Get(); }

	template<typename T, typename... Args>
		requires Derived<Resource, T>
	Shared<T> AddResource(Graphics& g, Args... args)
	{
		Shared<T> res = MakeShared<T>(g, m_Handle, std::forward<Args>(args)...);
		m_Handle.ptr += m_IncSize;
		m_Resources.push_back(res);
		return res;
	}

	Shared<R> operator[](UINT idx) const
	{
		return m_Resources[idx];
	}

private:
	std::vector<Shared<R>> m_Resources;
	ComPtr<ID3D12DescriptorHeap> m_Heap;
	D3D12_CPU_DESCRIPTOR_HANDLE m_Handle;
	UINT m_IncSize;
};