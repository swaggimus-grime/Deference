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
	const std::vector<std::pair<const std::wstring&, const std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>&>>& raygen,
	const std::vector<std::pair<const std::wstring&, const std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>&>>& miss,
	const std::vector<std::pair<const std::wstring&, const std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>&>>& hit)
{
	{
		RootParams params;
		m_GlobalSig = MakeShared<RootSig>(g, std::move(params));
		auto sig = desc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
		sig->SetRootSignature(m_GlobalSig->Sig());
	}

	const D3D12_STATE_OBJECT_DESC& descRef = *desc;
	HR g.Device().CreateStateObject(&descRef, IID_PPV_ARGS(&m_State));
	HR m_State->QueryInterface(IID_PPV_ARGS(&m_Props));

	auto getEntrySize = [&](decltype(raygen) entrySet)
	{
		SIZE_T maxArgs = 0;
		for (const auto& e : entrySet)
			maxArgs = std::max(maxArgs, e.second.size());
		
		UINT entrySize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + 8 * maxArgs;
		entrySize = ALIGN(entrySize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		return entrySize;
	};
	
	const UINT rgSize = getEntrySize(raygen);
	const UINT missSize = getEntrySize(miss);
	const UINT hitSize = getEntrySize(hit);
	g.CreateBuffer(m_Table, 
		ALIGN(
			rgSize * raygen.size() + 
			missSize * miss.size() + 
			hitSize * hit.size(),
			256
		),
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);
	
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
			{
				const uint64_t heapStart = e.second[0].ptr;
				*reinterpret_cast<uint64_t*>(pData + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES) = heapStart;
			}

			pData += entrySize;
		}
	};

	copyData(raygen, rgSize);
	copyData(miss, missSize);
	copyData(hit, hitSize);
	m_Table->Unmap(0, nullptr);

	m_Dispatch.Width = g.Width();
	m_Dispatch.Height = g.Height();
	m_Dispatch.Depth = 1;

	// RayGen is the first entry in the shader-table
	const auto rgSectionSize = rgSize * raygen.size();
	m_Dispatch.RayGenerationShaderRecord.StartAddress = m_Table->GetGPUVirtualAddress();
	m_Dispatch.RayGenerationShaderRecord.SizeInBytes = rgSectionSize;

	const auto missSectionSize = missSize * miss.size();
	m_Dispatch.MissShaderTable.StartAddress = m_Table->GetGPUVirtualAddress() + rgSectionSize;
	m_Dispatch.MissShaderTable.StrideInBytes = missSize;
	m_Dispatch.MissShaderTable.SizeInBytes = missSectionSize;

	const auto hitSectionSize = hitSize * hit.size();
	m_Dispatch.HitGroupTable.StartAddress = m_Table->GetGPUVirtualAddress() + rgSectionSize + missSectionSize;
	m_Dispatch.HitGroupTable.StrideInBytes = hitSize;
	m_Dispatch.HitGroupTable.SizeInBytes = hitSectionSize;
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

void RaytracingPipeline::Dispatch(Graphics& g)
{
	g.CL().DispatchRays(&m_Dispatch);
}

void RaytracingPipeline::Bind(Graphics& g)
{
	g.CL().SetGraphicsRootSignature(m_GlobalSig->Sig());
	g.CL().SetPipelineState1(m_State.Get());
}