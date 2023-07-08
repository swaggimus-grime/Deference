#pragma once

#include "Debug/Exception.h"
#include "Graphics.h"
#include "Bindable/Bindable.h"
#include "DepthStencil.h"
#include "UnorderedAccess.h"
#include "AccelStruct.h"
#include "ConstantBuffer.h"
#include <map>

template<D3D12_DESCRIPTOR_HEAP_TYPE T>
class Heap : public Bindable
{
public:
	Heap(Graphics& g, unsigned int numDescs, bool shaderVisible = false)
		:m_IncSize(g.Device().GetDescriptorHandleIncrementSize(T)),
		m_MaxNumDescs(numDescs)
	{
		BR (numDescs > 0);
		D3D12_DESCRIPTOR_HEAP_DESC dd = {};
		dd.NumDescriptors = numDescs;
		dd.Type = T;
		dd.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		HR g.Device().CreateDescriptorHeap(&dd, IID_PPV_ARGS(&m_Heap));

		m_Handle = m_Heap->GetCPUDescriptorHandleForHeapStart();
	}

	inline auto CPUStart() const { return m_Heap->GetCPUDescriptorHandleForHeapStart(); }

	virtual void Bind(Graphics& g) override
	{
		g.CL().SetDescriptorHeaps(1,
			m_Heap.GetAddressOf());
	}

	inline auto* GetHeap() const { return m_Heap.Get(); }
	inline UINT IncSize() const { return m_IncSize; }

protected:
	template<typename T, typename... Args>
	Shared<T> AddToHeap(Graphics& g, Args... args)
	{
		Shared<T> d = MakeShared<T>(g, m_Handle, std::forward<Args>(args)...);
		m_Handle.ptr += m_IncSize;
		m_Resources.push_back(d);
		return d;
	}

	template<typename T, typename... Args>
	Shared<T> AddToHeapWithHandle(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, Args... args)
	{
		Shared<T> d = MakeShared<T>(g, handle, std::forward<Args>(args)...);
		m_Resources.push_back(d);
		return d;
	}

	ComPtr<ID3D12DescriptorHeap> m_Heap;
	D3D12_CPU_DESCRIPTOR_HANDLE m_Handle;
	const UINT m_IncSize;
	const UINT m_MaxNumDescs;

private:
	std::vector<Shared<Resource>> m_Resources;
};

class RenderTargetHeap : public Heap<D3D12_DESCRIPTOR_HEAP_TYPE_RTV>
{
public:
	RenderTargetHeap(Graphics& g, UINT numTargets);
	virtual void Bind(Graphics& g) override;
	virtual void BindWithDepth(Graphics& g, Shared<DepthStencil> depth);

	template<typename... Args>
	Shared<RenderTarget> Add(Graphics& g, Args... args)
	{
		return AddToHeap<RenderTarget>(g, std::forward<Args>(args)...);
	}

private:
	UINT m_NumTargets;
};

class DepthStencilHeap : public Heap<D3D12_DESCRIPTOR_HEAP_TYPE_DSV>
{
public:
	DepthStencilHeap(Graphics& g, UINT numTargets = 1);
	virtual void Bind(Graphics& g) override;

	template<typename... Args>
	Shared<DepthStencil> Add(Graphics& g, Args... args)
	{
		return AddToHeap<DepthStencil>(g, std::forward<Args>(args)...);
	}
};

class SucHeap : public Heap<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV>
{
public:
	SucHeap(Graphics& g, UINT numDescs);

	inline UINT NumActiveDescs() const { return m_ActiveHandles.size(); }

	template<typename T, typename... Args>
	Shared<T> Add(Graphics& g, Args... args)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = CPUStart();
		if (m_CurrentDescIdx < m_MaxNumDescs)
			handle.ptr += m_CurrentDescIdx++ * m_IncSize;
		else if (m_FreeHandles.size() > 0)
		{
			handle = m_FreeHandles.front();
			m_FreeHandles.pop();
		}

		m_ActiveHandles.push_back(handle);
		auto r = AddToHeapWithHandle<T>(g, handle, std::forward<Args>(args)...);
		return r;
	}

	void Free(Shared<Resource> r);

private:
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_ActiveHandles;
	std::queue<D3D12_CPU_DESCRIPTOR_HANDLE> m_FreeHandles;
	UINT m_CurrentDescIdx;
};

class CSUHeap : public Heap<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV>
{
public:
	CSUHeap(Graphics& g, UINT numDescs);

	template<typename T, typename... Args>
	Shared<T> Add(Graphics& g, Args... args)
	{
		return AddToHeap<T>(g, std::forward<Args>(args)...);
	}

	void CopyToHeap(Graphics& g, const SucHeap& other);

	auto GPUStart() const { return GetHeap()->GetGPUDescriptorHandleForHeapStart(); }

	auto CPUHandle() const { return m_Handle; }
	auto GPUHandle() const { return m_GPUHandle; }

private:
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_GPUHandle;
};

class SamplerHeap : public Heap<D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER>
{
public:
	SamplerHeap(Graphics& g, UINT numDescs);
	auto GPUStart() const { return GetHeap()->GetGPUDescriptorHandleForHeapStart(); }
	template<typename T, typename... Args>
	Shared<T> Add(Graphics& g, Args... args)
	{
		return AddToHeap<T>(g, std::forward<Args>(args)...);
	}
};