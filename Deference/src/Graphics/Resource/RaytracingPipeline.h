#pragma once

#include "Bindable.h"

namespace Def
{
	class RaytracingPipeline;

	class ShaderBindTable
	{
	public:
		ShaderBindTable(Graphics& g, RaytracingPipeline* parent, UINT numRecords, SIZE_T recordSize);
		void Add(LPCWSTR shaderName, void* localArgs = nullptr, SIZE_T localArgSize = 0);
		inline D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const { return m_Buffer.GetGPUAddress(); }
		inline auto GetSize() const { return m_Buffer.Size(); }
		inline auto GetStride() const { return m_Buffer.GetStride(); }
		Shared<StructuredBuffer> Finish(Graphics& g);

	private:
		RaytracingPipeline* m_Parent;
		StructuredBuffer m_Buffer;
		uint8_t* m_Mapped;
	};

	class RaytracingPipeline : public Bindable
	{
	public:
		struct Desc
		{
			
			Unique<RootSig> m_GlobalSig;
			std::vector<Unique<RootSig>> m_LocalSigs;
			Unique<ShaderBindTable> m_RayGenTable;
			Unique<ShaderBindTable> m_MissTable;
			Unique<ShaderBindTable> m_HitTable;
		};

	public:
		RaytracingPipeline(Graphics& g, const D3D12_STATE_OBJECT_DESC& so);
		void SubmitTablesAndSigs(Graphics& g, Desc& desc);
		virtual void Bind(Graphics& g) override;
		inline auto* GetProps() const { return m_Props.Get(); }
		void Dispatch(Graphics& g);

	private:
		Shared<StructuredBuffer> m_RayGenTable;
		Shared<StructuredBuffer> m_MissTable;
		Shared<StructuredBuffer> m_HitTable;

		ComPtr<ID3D12StateObject> m_State;
		ComPtr<ID3D12StateObjectProperties> m_Props;

		D3D12_DISPATCH_RAYS_DESC m_Dispatch;
		Unique<RootSig> m_GlobalSig;
		std::vector<Unique<RootSig>> m_LocalSigs;
	};
}