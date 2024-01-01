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
class DescriptorHeap : public Bindable
{
public:
	DescriptorHeap(Graphics& g, unsigned int numDescs, bool shaderVisible)
		:m_IncSize(g.Device().GetDescriptorHandleIncrementSize(T)),
		m_MaxNumDescs(numDescs), m_HeapIdx(0)
	{
		BR (numDescs > 0);
		D3D12_DESCRIPTOR_HEAP_DESC dd = {};
		dd.NumDescriptors = numDescs;
		dd.Type = T;
		dd.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		HR g.Device().CreateDescriptorHeap(&dd, IID_PPV_ARGS(&m_Heap));

		m_CPUStart = m_Heap->GetCPUDescriptorHandleForHeapStart();
		m_HCPU = m_CPUStart;
	}

	inline auto CPUStart() const { return m_CPUStart; }

	virtual void Bind(Graphics& g) override
	{
		g.CL().SetDescriptorHeaps(1,
			m_Heap.GetAddressOf());
	}

	inline void Reset() { m_HCPU = m_CPUStart; m_HeapIdx = 0; }

	inline auto* operator*() const { return m_Heap.Get(); }

	inline UINT GetIncSize() const { return m_IncSize; }

	inline UINT NumDescriptors() const { return m_HeapIdx; }

	inline void IncrementHandle() { m_HCPU.ptr += m_IncSize; m_HeapIdx++; }

protected:
	template<typename R>
	void Add(Graphics& g, Shared<R> r)
	{
		r->CreateView(g, m_HCPU);
		IncrementHandle();
	}

	HCPU m_CPUStart;
	HCPU m_HCPU;
	UINT m_HeapIdx;
	const UINT m_IncSize;

private:
	ComPtr<ID3D12DescriptorHeap> m_Heap;
	const UINT m_MaxNumDescs;
};

class RenderTargetHeap : public DescriptorHeap<D3D12_DESCRIPTOR_HEAP_TYPE_RTV>
{
public:
	RenderTargetHeap(Graphics& g, UINT numTargets);
	virtual void Bind(Graphics& g) override;
	virtual void BindWithDepth(Graphics& g, DepthStencil& depth);

	template<typename R>
		requires Derived<RenderTarget, R>
	void Add(Graphics& g, Shared<R> r)
	{
		__super::Add(g, r);
	}

private:
	UINT m_NumTargets;
};

class DepthStencilHeap : public DescriptorHeap<D3D12_DESCRIPTOR_HEAP_TYPE_DSV>
{
public:
	DepthStencilHeap(Graphics& g, UINT numTargets = 1);
	virtual void Bind(Graphics& g) override;

	template<typename R>
		requires Derived<DepthStencil, R>
	void Add(Graphics& g, Shared<R> r)
	{
		__super::Add(g, r);
	}
};

class CPUShaderHeap : public DescriptorHeap<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV>
{
public:
	CPUShaderHeap(Graphics& g, UINT numDescs);
	
	template<typename R>
		requires NotSame<RenderTarget, R> && NotSame<DepthStencil, R>
	void Add(Graphics& g, Shared<R> r)
	{
		__super::Add(g, r);
	}
};

template<D3D12_DESCRIPTOR_HEAP_TYPE T>
class GPUVisibleHeap : public DescriptorHeap<T>
{
public:
	inline auto GPUStart() const { return m_GPUStart; }

protected:
	GPUVisibleHeap(Graphics& g, UINT numDescs)
		:DescriptorHeap<T>(g, numDescs, true)
	{
		m_GPUStart = (**this)->GetGPUDescriptorHandleForHeapStart();
	}

protected:
	HGPU m_GPUStart;
};

class GPUShaderHeap : public GPUVisibleHeap<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV>
{
public:
	GPUShaderHeap(Graphics& g, UINT numDescs);
	
	template<typename R>
		requires Derived<Resource, R>
	HGPU Add(Graphics& g, Shared<R> r)
	{
		HGPU hgpu{ m_GPUStart.ptr + (m_HCPU.ptr - m_CPUStart.ptr) };
		__super::Add(g, r);

		return hgpu;
	}

	HGPU AddTarget(Graphics& g, Shared<Target> r)
	{
		HGPU hgpu{ m_GPUStart.ptr + (m_HCPU.ptr - m_CPUStart.ptr) };
		r->CreateShaderView(g, m_HCPU);
		m_HCPU.ptr += m_IncSize;

		return hgpu;
	}

	void ReturnTo(HGPU hgpu)
	{
		UINT bytes = (hgpu.ptr - m_GPUStart.ptr);
		m_HCPU.ptr = m_CPUStart.ptr + bytes;
		m_HeapIdx = bytes / m_IncSize;
	}

	template<typename R>
		requires Derived<Resource, R>
	HGPU Copy(Graphics& g, Shared<R> r)
	{
		g.Device().CopyDescriptorsSimple(1, m_HCPU, r->GetHCPU(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		HGPU hgpu{ m_GPUStart.ptr + (m_HCPU.ptr - m_CPUStart.ptr) };
		m_HCPU.ptr += m_IncSize;
		return hgpu;
	}

	std::vector<std::vector<HGPU>> Copy(Graphics& g, HCPU src, UINT num, UINT stride);
};

class SamplerHeap : public GPUVisibleHeap<D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER>
{
public:
	SamplerHeap(Graphics& g, UINT numDescs);
	HGPU Add(Graphics& g);
};