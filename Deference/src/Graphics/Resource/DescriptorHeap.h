#pragma once

#include "Debug/Exception.h"
#include "Graphics.h"
#include "Resource.h"
#include "DepthStencil.h"
#include "View.h"
#include "Handle.h"
#include "Commander.h"

namespace Def
{
	struct DescriptorCopyRange
	{
		HCPU From{ 0 };
		UINT NumDescs = 1;

		DescriptorCopyRange() = default;
		DescriptorCopyRange(HCPU from, UINT numDescs)
			:From(from), NumDescs(numDescs)
		{}
	};

	template<class V>
		requires Derived<View, V>
	class DescriptorHeap : public Bindable
	{
	public:
		inline UINT NumDescriptors() const { return m_NumDescs; }
		inline UINT GetIncrementSize() const { return m_IncSize; }
		inline ID3D12DescriptorHeap* operator*() const { return m_Heap.Get(); }

		virtual void Bind(Graphics& g) override
		{
			ID3D12DescriptorHeap* heaps[] = {**this};
			g.CL().SetDescriptorHeaps(1, heaps);
		}

		static void Bind(Graphics& g, const std::vector<ID3D12DescriptorHeap*> heaps)
		{
			g.CL().SetDescriptorHeaps(heaps.size(), heaps.data());
		}

		inline HCPU GetHCPU(const Shared<V>& r) const { return m_HCPUs.find(r)->second; }
		inline HCPU CPUStart() const { return m_CPUStart; }

		inline void Skip(UINT numSlots)
		{
			m_NumDescs += numSlots;
		}

		HCPU Add(Graphics& g, Shared<V> r)
		{
			auto hcpu = NextHCPU();
			m_HCPUs.insert({ r, hcpu });
			r->CreateView(g, hcpu);
			return hcpu;
		}

		HCPU AddNull(Graphics& g, Shared<V> r)
		{
			auto hcpu = NextHCPU();
			r->CreateNullView(g, hcpu);
			return hcpu;
		}
		
		HCPU Copy(Graphics& g, HCPU src)
		{
			auto hcpu = NextHCPU();
			g.Device().CopyDescriptorsSimple(1, hcpu, src, m_Type);
			return hcpu;
		}

		HCPU Copy(Graphics& g, DescriptorCopyRange range)
		{
			auto hcpu = CurrentHCPU();
			g.Device().CopyDescriptorsSimple(range.NumDescs, hcpu, range.From, m_Type);
			m_NumDescs += range.NumDescs;
			return hcpu;
		}

		inline void Reset()
		{
			m_HCPUs.clear();
			m_NumDescs = 0;
		}

	protected:
		DescriptorHeap(Graphics& g, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescs, bool shaderVisible = false)
			:m_Type(type), m_IncSize(g.Device().GetDescriptorHandleIncrementSize(type)), m_NumDescs(0u)
		{
			BR(numDescs > 0);
			D3D12_DESCRIPTOR_HEAP_DESC dd = {};
			dd.NumDescriptors = numDescs;
			dd.Type = type;
			dd.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			HR g.Device().CreateDescriptorHeap(&dd, IID_PPV_ARGS(&m_Heap));

			m_CPUStart = m_Heap->GetCPUDescriptorHandleForHeapStart();
		}

		DescriptorHeap() = default;

	protected:
		inline HCPU CurrentHCPU() { return m_CPUStart + m_IncSize * m_NumDescs; }
		inline HCPU NextHCPU() { return m_CPUStart + m_IncSize * m_NumDescs++; }

	private:
		HCPU m_CPUStart;
		UINT m_IncSize;
		UINT m_NumDescs;
		D3D12_DESCRIPTOR_HEAP_TYPE m_Type;
		ComPtr<ID3D12DescriptorHeap> m_Heap;
		std::unordered_map<Shared<V>, HCPU> m_HCPUs;
	};

	class RenderTargetHeap : public DescriptorHeap<RTV>
	{
	public:
		RenderTargetHeap(Graphics& g, UINT numDescs)
			:DescriptorHeap(g, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, numDescs)
		{}

		void Clear(Graphics& g)
		{
			const UINT numDescs = NumDescriptors();
			const FLOAT col[] = { 0.f, 0.f, 0.f, 0.f };
			for (UINT i = 0; i < numDescs; i++)
				g.CL().ClearRenderTargetView(CPUStart() + GetIncrementSize() * i, col, 0, nullptr);
		}

		void BindWithDepth(Graphics& g, DepthStencil* d)
		{
			auto hcpu = CPUStart();
			g.CL().OMSetRenderTargets(NumDescriptors(), &hcpu, true, &d->DSVHCPU());
		}
	};

	class DepthStencilHeap : public DescriptorHeap<DSV>
	{
	public:
		DepthStencilHeap(Graphics& g, UINT numDescs)
			:DescriptorHeap(g, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, numDescs)
		{}

		void Clear(Graphics& g)
		{
			const UINT numDescs = NumDescriptors();
			for (UINT i = 0; i < numDescs; i++)
				g.CL().ClearDepthStencilView(CPUStart() + GetIncrementSize() * i, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);
		}
	};

	template<class V>
	class ShaderVisibleHeap : public DescriptorHeap<V>
	{
	public:
		inline HGPU GPUStart() const { return m_GPUStart; }
		inline HGPU GetHGPU(Shared<V> r) const { return GPUStart() + (this->GetHCPU(r) - this->CPUStart()).ptr; }

	protected:
		ShaderVisibleHeap(Graphics& g, D3D12_DESCRIPTOR_HEAP_TYPE type, unsigned int numDescs)
			:DescriptorHeap<V>(g, type, numDescs, true)
		{
			m_GPUStart = (**this)->GetGPUDescriptorHandleForHeapStart();
		}

		ShaderVisibleHeap() = default;
	private:
		HGPU m_GPUStart;
	};

	class CPUHeap : public DescriptorHeap<CSUView>
	{
	public:
		CPUHeap(Graphics& g, UINT numDescs)
			:DescriptorHeap(g, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, numDescs)
		{}
	};

	class GPUHeap : public ShaderVisibleHeap<CSUView>
	{
	public:
		GPUHeap(Graphics& g, UINT numDescs)
			:ShaderVisibleHeap(g, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, numDescs)
		{}
	};

	class SamplerHeap : public ShaderVisibleHeap<SamplerView>
	{
	public:
		SamplerHeap(Graphics& g, UINT numDescs)
			:ShaderVisibleHeap(g, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, numDescs)
		{}
	};
}
