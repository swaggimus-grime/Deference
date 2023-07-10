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

void RaytracingPipeline::Create(Graphics& g, CD3DX12_STATE_OBJECT_DESC& desc,
	const std::vector<UINT>& rgNumArgsPerEntry,
	const std::vector<UINT>& missNumArgsPerEntry,
	const std::vector<UINT>& hitNumArgsPerEntry)
{
	{
		m_GlobalSig = MakeShared<RootSig>(g, 0, nullptr);
		auto sig = desc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
		sig->SetRootSignature(m_GlobalSig->Sig());
	}

	const D3D12_STATE_OBJECT_DESC& descRef = *desc;
	HR g.Device().CreateStateObject(&descRef, IID_PPV_ARGS(&m_State));
	HR m_State->QueryInterface(IID_PPV_ARGS(&m_Props));

	auto getEntrySize = [&](const std::vector<UINT>& entrySet)
	{
		UINT maxArgs = 0;
		for (UINT numArgs : entrySet)
			maxArgs = std::max(maxArgs, numArgs);

		UINT entrySize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + 8 * maxArgs;
		entrySize = ALIGN(entrySize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		return entrySize;
	};

	m_RayGenSize = getEntrySize(rgNumArgsPerEntry);
	m_MissSize = getEntrySize(missNumArgsPerEntry);
	m_HitSize = getEntrySize(hitNumArgsPerEntry);
	g.CreateBuffer(m_Table,
		ALIGN(
			m_RayGenSize * rgNumArgsPerEntry.size() +
			m_MissSize * missNumArgsPerEntry.size() +
			m_HitSize * hitNumArgsPerEntry.size(),
			256
		),
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);
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
	g.CL().SetGraphicsRootSignature(m_GlobalSig->Sig());
	g.CL().SetPipelineState1(m_State.Get());
}

void RaytracingPipeline::UpdateTable(Graphics& g, 
	const std::vector<std::pair<const std::wstring&, const std::vector<void*>&>>& raygen, 
	const std::vector<std::pair<const std::wstring&, const std::vector<void*>&>>& miss, 
	const std::vector<std::pair<const std::wstring&, const std::vector<void*>&>>& hit)
{
	uint8_t* pData;
	HR m_Table->Map(0, nullptr, reinterpret_cast<void**>(&pData));
	auto copyData = [&](decltype(raygen) entrySet, UINT entrySize) {
		for (const auto& e : entrySet)
		{
			// Get the shader identifier, and check whether that identifier is known
			void* id = m_Props->GetShaderIdentifier(e.first.c_str());
			BR id;
			// Copy the shader identifier
			std::memcpy(pData, id, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
			if (!e.second.empty())
				std::memcpy(reinterpret_cast<uint64_t*>(pData + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES), e.second.data(), e.second.size() * 8);

			pData += entrySize;
		}
	};

	copyData(raygen, m_RayGenSize);
	copyData(miss, m_MissSize);
	copyData(hit, m_HitSize);
	m_Table->Unmap(0, nullptr);

	m_Dispatch = {};
	m_Dispatch.Width = g.Width();
	m_Dispatch.Height = g.Height();
	m_Dispatch.Depth = 1;

	// RayGen is the first entry in the shader-table
	const auto rgSectionSize = m_RayGenSize * raygen.size();
	m_Dispatch.RayGenerationShaderRecord.StartAddress = m_Table->GetGPUVirtualAddress();
	m_Dispatch.RayGenerationShaderRecord.SizeInBytes = rgSectionSize;

	const auto missSectionSize = m_MissSize * miss.size();
	m_Dispatch.MissShaderTable.StartAddress = m_Table->GetGPUVirtualAddress() + rgSectionSize;
	m_Dispatch.MissShaderTable.StrideInBytes = m_MissSize;
	m_Dispatch.MissShaderTable.SizeInBytes = missSectionSize;

	const auto hitSectionSize = m_HitSize * hit.size();
	m_Dispatch.HitGroupTable.StartAddress = m_Table->GetGPUVirtualAddress() + rgSectionSize + missSectionSize;
	m_Dispatch.HitGroupTable.StrideInBytes = m_HitSize;
	m_Dispatch.HitGroupTable.SizeInBytes = hitSectionSize;
}

void RaytracingPipeline::Dispatch(Graphics& g)
{
	g.CL().DispatchRays(&m_Dispatch);
}
