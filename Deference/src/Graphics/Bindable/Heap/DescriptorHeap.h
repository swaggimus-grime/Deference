#pragma once

#include "Debug/Exception.h"
#include "Graphics.h"
#include "Bindable/Bindable.h"
#include "DepthStencil.h"
#include "UnorderedAccess.h"
#include "AccelStruct.h"
#include "ConstantBuffer.h"
#include "Sampler.h"
#include <map>

template<D3D12_DESCRIPTOR_HEAP_TYPE T, bool shaderVisible>
class DescriptorHeap : public Bindable
{
public:
	DescriptorHeap(Graphics& g, unsigned int numDescs)
		:m_IncSize(g.Device().GetDescriptorHandleIncrementSize(T)),
		m_MaxNumDescs(numDescs)
	{
		BR (numDescs > 0);
		D3D12_DESCRIPTOR_HEAP_DESC dd = {};
		dd.NumDescriptors = numDescs;
		dd.Type = T;
		dd.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		HR g.Device().CreateDescriptorHeap(&dd, IID_PPV_ARGS(&m_Heap));

		m_CPUStart = m_Heap->GetCPUDescriptorHandleForHeapStart();
		m_CPUHandle = m_CPUStart;

		if constexpr (shaderVisible)
		{
			m_GPUStart = m_Heap->GetGPUDescriptorHandleForHeapStart();
			m_GPUHandle = m_GPUStart;
		}
	}

	inline auto CPUStart() const { return m_CPUStart; }
	inline auto GetGPUHandle() const requires shaderVisible { return m_GPUHandle; }
	inline auto GPUStart() const requires shaderVisible { return m_GPUStart; }

	virtual void Bind(Graphics& g) override
	{
		g.CL().SetDescriptorHeaps(1,
			m_Heap.GetAddressOf());
	}

	inline void Reset() { m_CPUHandle = m_CPUStart; }

	inline auto* GetHeap() const { return m_Heap.Get(); }

	inline HCPU Next()
	{
		HCPU ret = m_CPUHandle;
		m_CPUHandle.ptr += m_IncSize;

		if constexpr (shaderVisible)
			m_GPUHandle.ptr += m_IncSize;

		return ret;
	}

protected:
	template<typename T, typename... Args>
		requires Derived<Resource, T> || Derived<Sampler, T>
	Shared<T> AddToHeap(Graphics& g, Args... args)
	{
		Shared<T> d = MakeShared<T>(g, m_CPUHandle, std::forward<Args>(args)...);
		m_CPUHandle.ptr += m_IncSize;
		return d;
	}

private:
	ComPtr<ID3D12DescriptorHeap> m_Heap;
	const UINT m_IncSize;
	const UINT m_MaxNumDescs;

	D3D12_CPU_DESCRIPTOR_HANDLE m_CPUStart;
	D3D12_CPU_DESCRIPTOR_HANDLE m_CPUHandle;

	struct Empty {};
	using GPUHandle = std::conditional_t<shaderVisible, D3D12_GPU_DESCRIPTOR_HANDLE, Empty>;

	GPUHandle m_GPUStart;
	GPUHandle m_GPUHandle;
};

class RenderTargetHeap : public DescriptorHeap<D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false>
{
public:
	RenderTargetHeap(Graphics& g, UINT numTargets);
	virtual void Bind(Graphics& g) override;
	virtual void BindWithDepth(Graphics& g, Shared<DepthStencil> depth);

private:
	UINT m_NumTargets;
};

class DepthStencilHeap : public DescriptorHeap<D3D12_DESCRIPTOR_HEAP_TYPE_DSV, false>
{
public:
	DepthStencilHeap(Graphics& g, UINT numTargets = 1);
	virtual void Bind(Graphics& g) override;

};

class CPUShaderHeap : public DescriptorHeap<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, false>
{
public:
	CPUShaderHeap(Graphics& g, UINT numDescs);

};

class GPUShaderHeap : public DescriptorHeap<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true>
{
public:
	GPUShaderHeap(Graphics& g, UINT numDescs);

};

class SamplerHeap : public DescriptorHeap<D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, true>
{
public:
	SamplerHeap(Graphics& g, UINT numDescs);

	template<typename... Args>
	void Add(Graphics& g, Args&&... args)
	{
		AddToHeap<Sampler>(g, std::forward<Args>(args)...);
	}
};