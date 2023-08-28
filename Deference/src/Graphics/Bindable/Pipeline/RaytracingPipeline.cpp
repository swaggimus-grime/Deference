#define NOMINMAX

#include "RaytracingPipeline.h"
#include <vector>
#include "util.h"
#include "Debug/Exception.h"
#include <minwindef.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <fstream>

RaytracingPipeline::DXC RaytracingPipeline::s_DXC = {};

void RaytracingPipeline::Create(Graphics& g, CD3DX12_STATE_OBJECT_DESC& desc)
{
	const D3D12_STATE_OBJECT_DESC& descRef = *desc;
	HR g.Device().CreateStateObject(&descRef, IID_PPV_ARGS(&m_State));
	HR m_State->QueryInterface(IID_PPV_ARGS(&m_Props));

	m_Dispatch = {};
	m_Dispatch.Width = g.Width();
	m_Dispatch.Height = g.Height();
	m_Dispatch.Depth = 1;
}

ComPtr<IDxcBlob> RaytracingPipeline::CreateLibrary(const std::wstring& path)
{
	std::ifstream shaderFile(path);
	BR shaderFile.good();
	std::stringstream ss;
	ss << shaderFile.rdbuf();
	auto sstr = ss.str();

	IDxcBlobEncoding* enc;
	HR s_DXC.m_Lib->CreateBlobWithEncodingFromPinned(
		LPBYTE(sstr.c_str()), static_cast<uint32_t>(sstr.size()), 0, &enc);

	// Compile
	IDxcOperationResult* result;
	HR s_DXC.m_Compiler->Compile(enc, path.c_str(), L"", L"lib_6_3", nullptr, 0, nullptr, 0,
		s_DXC.m_Includer, &result);

	// Verify the result
	HRESULT resultCode;
	HR result->GetStatus(&resultCode);
	if (FAILED(resultCode))
	{
		IDxcBlobEncoding* pError;
		HR result->GetErrorBuffer(&pError);

		// Convert error blob to a string
		std::vector<char> infoLog(pError->GetBufferSize() + 1);
		memcpy(infoLog.data(), pError->GetBufferPointer(), pError->GetBufferSize());
		infoLog[pError->GetBufferSize()] = 0;

		std::string errorMsg = "Shader Compiler Error:\n";
		errorMsg.append(infoLog.data());

		MessageBoxA(nullptr, errorMsg.c_str(), "Error!", MB_OK);
		throw std::logic_error("Failed compile shader");
	}

	ComPtr<IDxcBlob> blob;
	HR result->GetResult(&blob);
	return blob;
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

void RaytracingPipeline::SubmitTable(SHADER_TABLE_TYPE type, Shared<ShaderBindTable> table)
{
	table->Finish();
	switch (type)
	{
	case SHADER_TABLE_TYPE::RAY_GEN:
		m_RayGenTable = std::move(table);
		m_Dispatch.RayGenerationShaderRecord.StartAddress = m_RayGenTable->GetGPUAddress();
		m_Dispatch.RayGenerationShaderRecord.SizeInBytes = m_RayGenTable->GetSize();
		break;
	case SHADER_TABLE_TYPE::MISS:
		m_MissTable = std::move(table);
		m_Dispatch.MissShaderTable.StartAddress = m_MissTable->GetGPUAddress();
		m_Dispatch.MissShaderTable.StrideInBytes = m_MissTable->GetStride();
		m_Dispatch.MissShaderTable.SizeInBytes = m_MissTable->GetSize();
		break;
	case SHADER_TABLE_TYPE::HIT:
		m_HitTable = std::move(table);
		m_Dispatch.HitGroupTable.StartAddress = m_HitTable->GetGPUAddress();
		m_Dispatch.HitGroupTable.StrideInBytes = m_HitTable->GetStride();
		m_Dispatch.HitGroupTable.SizeInBytes = m_HitTable->GetSize();
		break;
	}
}

ShaderBindTable::ShaderBindTable(Graphics& g, RaytracingPipeline* parent, UINT numRecords, SIZE_T recordSize)
	:m_Parent(parent), m_RecordSize(ALIGN(recordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT))
{
	m_Size = numRecords * m_RecordSize;
	//m_Records.reserve(numRecords);
	g.CreateBuffer(m_Buffer,
		m_Size,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);

	HR m_Buffer->Map(0, nullptr, reinterpret_cast<void**>(&m_Mapped));
}

void ShaderBindTable::Add(LPCWSTR shaderName, void* localArgs, SIZE_T localArgSize)
{
	void* id = m_Parent->GetProps()->GetShaderIdentifier(shaderName);

	std::memcpy(m_Mapped, id, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
	if (localArgs)
		std::memcpy(m_Mapped + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, localArgs, localArgSize);

	m_Mapped += m_RecordSize;
}

D3D12_GPU_VIRTUAL_ADDRESS ShaderBindTable::GetGPUAddress() const
{
	return m_Buffer->GetGPUVirtualAddress();
}

void ShaderBindTable::Finish()
{
	m_Buffer->Unmap(0, nullptr);
}
