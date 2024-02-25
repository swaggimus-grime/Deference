#define NOMINMAX

#include "RaytracingPipeline.h"
#include <vector>
#include "util.h"
#include "Debug/Exception.h"
#include <minwindef.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <fstream>

namespace Def
{
	RaytracingPipeline::RaytracingPipeline(Graphics& g, const D3D12_STATE_OBJECT_DESC& so)
	{
		m_Dispatch = {};
		m_Dispatch.Width = g.Width();
		m_Dispatch.Height = g.Height();
		m_Dispatch.Depth = 1;

		HR g.Device().CreateStateObject(&so, IID_PPV_ARGS(&m_State));
		HR m_State->QueryInterface(IID_PPV_ARGS(&m_Props));
	}

	void RaytracingPipeline::SubmitTablesAndSigs(Graphics& g, Desc& desc)
	{
		m_GlobalSig = std::move(desc.m_GlobalSig);
		m_LocalSigs = std::move(desc.m_LocalSigs);
		m_RayGenTable = desc.m_RayGenTable->Finish(g);
		m_MissTable = desc.m_MissTable->Finish(g);
		m_HitTable = desc.m_HitTable->Finish(g);

		m_Dispatch.RayGenerationShaderRecord.StartAddress = m_RayGenTable->GetGPUAddress();
		m_Dispatch.RayGenerationShaderRecord.SizeInBytes = m_RayGenTable->Size();
		m_Dispatch.MissShaderTable.StartAddress = m_MissTable->GetGPUAddress();
		m_Dispatch.MissShaderTable.StrideInBytes = m_MissTable->GetStride();
		m_Dispatch.MissShaderTable.SizeInBytes = m_MissTable->Size();
		m_Dispatch.HitGroupTable.StartAddress = m_HitTable->GetGPUAddress();
		m_Dispatch.HitGroupTable.StrideInBytes = m_HitTable->GetStride();
		m_Dispatch.HitGroupTable.SizeInBytes = m_HitTable->Size();
	}

	void RaytracingPipeline::Bind(Graphics& g)
	{
		g.CL().SetComputeRootSignature(**m_GlobalSig);
		g.CL().SetPipelineState1(m_State.Get());
	}

	void RaytracingPipeline::Dispatch(Graphics& g)
	{
		g.CL().DispatchRays(&m_Dispatch);
	}

	ShaderBindTable::ShaderBindTable(Graphics& g, RaytracingPipeline* parent, UINT numRecords, SIZE_T recordSize)
		:m_Parent(parent),
		m_Buffer(g, D3D12_HEAP_TYPE_UPLOAD, numRecords, ALIGN(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + recordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT),
			D3D12_RESOURCE_STATE_GENERIC_READ)
	{
		BR (GetStride() <= D3D12_RAYTRACING_MAX_SHADER_RECORD_STRIDE);
		m_Mapped = (uint8_t*)m_Buffer.Map();
	}

	void ShaderBindTable::Add(LPCWSTR shaderName, void* localArgs, SIZE_T localArgSize)
	{
		void* id = m_Parent->GetProps()->GetShaderIdentifier(shaderName);

		std::memcpy(m_Mapped, id, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		if (localArgs)
			std::memcpy(m_Mapped + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, localArgs, localArgSize);

		m_Mapped += GetStride();
	}

	Shared<StructuredBuffer> ShaderBindTable::Finish(Graphics& g)
	{
		m_Buffer.Unmap();

		Shared<StructuredBuffer> def = MakeShared<StructuredBuffer>(g, D3D12_HEAP_TYPE_DEFAULT, m_Buffer.NumElements(), GetStride(), D3D12_RESOURCE_STATE_COMMON);
		Commander<D3D12_RESOURCE_BARRIER>::Init()
			.Add(def->Transition(D3D12_RESOURCE_STATE_COPY_DEST))
			.Add(m_Buffer.Transition(D3D12_RESOURCE_STATE_COPY_SOURCE))
			.Transition(g);
		g.CL().CopyResource(**def, *m_Buffer);

		def->Transition(g, D3D12_RESOURCE_STATE_GENERIC_READ);
		g.Flush();
		return std::move(def);
	}

}