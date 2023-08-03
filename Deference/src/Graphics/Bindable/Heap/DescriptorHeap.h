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

template<D3D12_DESCRIPTOR_HEAP_TYPE T>
class DescriptorHeap : public Bindable
{
public:
	DescriptorHeap(Graphics& g, unsigned int numDescs, bool shaderVisible)
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
	}

	inline auto CPUStart() const { return m_CPUStart; }

	virtual void Bind(Graphics& g) override
	{
		g.CL().SetDescriptorHeaps(1,
			m_Heap.GetAddressOf());
	}

	inline void Reset() { m_CPUHandle = m_CPUStart; }

	inline auto* operator*() const { return m_Heap.Get(); }

	inline UINT GetIncSize() const { return m_IncSize; }

	virtual HDESC Next()
	{
		HCPU ret = m_CPUHandle;
		m_CPUHandle.ptr += m_IncSize;
		return { ret, 0 };
	}

private:
	const UINT m_IncSize;
	ComPtr<ID3D12DescriptorHeap> m_Heap;
	const UINT m_MaxNumDescs;

	D3D12_CPU_DESCRIPTOR_HANDLE m_CPUStart;
	D3D12_CPU_DESCRIPTOR_HANDLE m_CPUHandle;
};

class RenderTargetHeap : public DescriptorHeap<D3D12_DESCRIPTOR_HEAP_TYPE_RTV>
{
public:
	RenderTargetHeap(Graphics& g, UINT numTargets);
	virtual void Bind(Graphics& g) override;
	virtual void BindWithDepth(Graphics& g, DepthStencil& depth);

private:
	UINT m_NumTargets;
};

class DepthStencilHeap : public DescriptorHeap<D3D12_DESCRIPTOR_HEAP_TYPE_DSV>
{
public:
	DepthStencilHeap(Graphics& g, UINT numTargets = 1);
	virtual void Bind(Graphics& g) override;

};

class CPUShaderHeap : public DescriptorHeap<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV>
{
public:
	CPUShaderHeap(Graphics& g, UINT numDescs);
	
};

template<D3D12_DESCRIPTOR_HEAP_TYPE T>
class GPUVisibleHeap : public DescriptorHeap<T>
{
public:
	inline auto GPUStart() const { return m_GPUStart; }

	virtual HDESC Next() override
	{
		auto desc = DescriptorHeap<T>::Next();
		desc.m_HGPU = m_GPUHandle;
		m_GPUHandle.ptr += DescriptorHeap<T>::GetIncSize();

		return desc;
	}

protected:
	GPUVisibleHeap(Graphics& g, UINT numDescs)
		:DescriptorHeap<T>(g, numDescs, true)
	{
		m_GPUStart = (**this)->GetGPUDescriptorHandleForHeapStart();
		m_GPUHandle = m_GPUStart;
	}

private:
	HGPU m_GPUStart;
	HGPU m_GPUHandle;
};

class GPUShaderHeap : public GPUVisibleHeap<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV>
{
public:
	GPUShaderHeap(Graphics& g, UINT numDescs);
	
};

class SamplerHeap : public GPUVisibleHeap<D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER>
{
public:
	SamplerHeap(Graphics& g, UINT numDescs);

};