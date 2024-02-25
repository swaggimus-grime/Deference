#pragma once

#include "Graphics.h"
#include "Handle.h"
#include "Resource.h"

namespace Def
{
	class View
	{
	public:
		virtual void CreateView(Graphics& g, HCPU hcpu) = 0;
		virtual void CreateNullView(Graphics& g, HCPU hcpu) = 0;

	protected:
		inline Resource* GetResource() const { return m_Resource; }

		View(Resource* res)
			:m_Resource(res)
		{}

	private:
		Resource* m_Resource;
	};

	class RTV : public View
	{
	public:
		virtual const D3D12_RENDER_TARGET_VIEW_DESC& RTVDesc() const = 0;
		inline const HCPU& RTVHCPU() const { return m_HCPU; }
		virtual void CreateView(Graphics& g, HCPU hcpu) override
		{
			g.Device().CreateRenderTargetView(**GetResource(), nullptr, hcpu);
			m_HCPU = hcpu;
		}
		virtual void CreateNullView(Graphics& g, HCPU hcpu) override
		{
			g.Device().CreateRenderTargetView(nullptr, nullptr, hcpu);
		}
	protected:
		using View::View;

	private:
		HCPU m_HCPU = { 0 };
	};

	class DSV : public View
	{
	public:
		virtual const D3D12_DEPTH_STENCIL_VIEW_DESC& DSVDesc() const = 0;
		inline const HCPU& DSVHCPU() const { return m_HCPU; }
		virtual void CreateView(Graphics& g, HCPU hcpu) override
		{
			auto desc = DSVDesc();
			g.Device().CreateDepthStencilView(**GetResource(), &desc, hcpu);
			m_HCPU = hcpu;
		}
		virtual void CreateNullView(Graphics& g, HCPU hcpu) override
		{
			g.Device().CreateDepthStencilView(nullptr, nullptr, hcpu);
		}
	protected:
		using View::View;

	private:
		HCPU m_HCPU = { 0 };
	};

	class SamplerView : public View
	{
	public:
		virtual const D3D12_SAMPLER_DESC& SamplerViewDesc() const = 0;
		inline const HCPU& SamplerViewHCPU() const { return m_HCPU; }
		virtual void CreateView(Graphics& g, HCPU hcpu) override
		{
			auto desc = SamplerViewDesc();
			g.Device().CreateSampler(&desc, hcpu);
			m_HCPU = hcpu;
		}
		virtual void CreateNullView(Graphics& g, HCPU hcpu) override
		{
			g.Device().CreateSampler(nullptr, hcpu);
		}
	protected:
		using View::View;

	private:
		HCPU m_HCPU = { 0 };
	};

	class CSUView : public View
	{
	protected:
		using View::View;
	};

	class CBV : public CSUView
	{
	public:
		virtual const D3D12_CONSTANT_BUFFER_VIEW_DESC& CBVDesc() const = 0;
		inline const HCPU& CBVHCPU() const { return m_HCPU; }
		virtual void CreateView(Graphics& g, HCPU hcpu) override
		{
			auto desc = CBVDesc();
			g.Device().CreateConstantBufferView(&desc, hcpu);
			m_HCPU = hcpu;
		}
		virtual void CreateNullView(Graphics& g, HCPU hcpu) override
		{
			g.Device().CreateConstantBufferView(nullptr, hcpu);
		}
	protected:
		using CSUView::CSUView;

	private:
		HCPU m_HCPU = { 0 };
	};

	class SRV : public CSUView 
	{ 
	public: 
		virtual const D3D12_SHADER_RESOURCE_VIEW_DESC& SRVDesc() const = 0; 
		inline const HCPU& SRVHCPU() const { return m_HCPU; }		 
		virtual void CreateView(Graphics& g, HCPU hcpu) override 
		{ 
			auto desc = SRVDesc();
			g.Device().CreateShaderResourceView(**GetResource(), &desc, hcpu);
			m_HCPU = hcpu; 
		} 
		virtual void CreateNullView(Graphics& g, HCPU hcpu) override 
		{ 
			auto desc = SRVDesc();
			g.Device().CreateShaderResourceView(nullptr, &desc, hcpu);
		}
	protected: 
		using CSUView::CSUView;

	private: 
		HCPU m_HCPU = { 0 };
	}; 

	class UAV : public CSUView
	{
	public:
		virtual const D3D12_UNORDERED_ACCESS_VIEW_DESC& UAVDesc() const = 0;
		inline const HCPU& UAVHCPU() const { return m_HCPU; }
		virtual void CreateView(Graphics& g, HCPU hcpu) override
		{
			auto desc = UAVDesc();
			g.Device().CreateUnorderedAccessView(**GetResource(), nullptr, &desc, hcpu);
			m_HCPU = hcpu;
		}
		virtual void CreateNullView(Graphics& g, HCPU hcpu) override
		{
			g.Device().CreateUnorderedAccessView(nullptr, nullptr, nullptr, hcpu);
		}
	protected:
		using CSUView::CSUView;

	private:
		HCPU m_HCPU = { 0 };
	};

	/*#define VIEW_CLASS(name, base, desc, func, createParams, nullParams) \
		class name : public base \
		{ \
		public: \
			virtual const desc& name ## Desc() const = 0; \
			inline const HCPU& name ## HCPU() const { return m_HCPU; }		 \
			virtual void CreateView(Graphics& g, HCPU hcpu) override \
			{ \
				g.Device().func(createParams, hcpu); \
				m_HCPU = hcpu; \
			} \
			virtual void CreateNullView(Graphics& g, HCPU hcpu) override \
			{ \
				g.Device().func(nullParams, hcpu); \
			} \
		protected: \
			using base::base; \
		private: \
			HCPU m_HCPU; \
		}; \

	#define PACK(...) __VA_ARGS__

	VIEW_CLASS(RTV, View, D3D12_RENDER_TARGET_VIEW_DESC, CreateRenderTargetView, PACK(**GetResource(), nullptr), PACK(nullptr, nullptr))
	VIEW_CLASS(DSV, View, D3D12_DEPTH_STENCIL_VIEW_DESC, CreateDepthStencilView, PACK(**GetResource(), &DSVDesc()), PACK(nullptr, nullptr))
	VIEW_CLASS(SRV, CSUView, D3D12_SHADER_RESOURCE_VIEW_DESC, CreateShaderResourceView, PACK(**GetResource(), &SRVDesc()), PACK(nullptr, nullptr))
	VIEW_CLASS(CBV, CSUView, D3D12_CONSTANT_BUFFER_VIEW_DESC, CreateConstantBufferView, &CBVDesc(), nullptr)
	VIEW_CLASS(UAV, CSUView, D3D12_UNORDERED_ACCESS_VIEW_DESC, CreateUnorderedAccessView, PACK(**GetResource(), nullptr, &UAVDesc()), PACK(nullptr, nullptr, nullptr))
	VIEW_CLASS(Sampler, View, D3D12_SAMPLER_DESC, CreateSampler, &SamplerDesc(), nullptr)*/

}
