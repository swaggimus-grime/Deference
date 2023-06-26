#include "Pipeline.h"
#include "Graphics.h"
#include "Shader/RootSig.h"
#include "Resource/Heap.h"
#include "DXRHelper.h"

DXRPipeline::DXRPipeline(Graphics& g, DXRParams& pipelineParams, SBTParams& sbtParams)
{
    m_Libs = std::move(pipelineParams.m_Libs);
    m_Sigs = std::move(pipelineParams.m_Sigs);

    m_State = pipelineParams.m_Gen.Generate();
    HR m_State->QueryInterface(IID_PPV_ARGS(&m_Props));

	g.CreateBuffer(m_SBTBuff, sbtParams.m_SBT.ComputeSBTSize(), D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);
	sbtParams.m_SBT.Generate(m_SBTBuff.Get(), m_Props.Get());

	m_SBT = std::move(sbtParams.m_SBT);
}

void DXRPipeline::Dispatch(Graphics& g)
{
	D3D12_DISPATCH_RAYS_DESC m_DispatchDesc = {};
	const uint32_t rayGenerationSectionSizeInBytes = m_SBT.GetRayGenSectionSize();
	m_DispatchDesc.RayGenerationShaderRecord.StartAddress = m_SBTBuff->GetGPUVirtualAddress();
	m_DispatchDesc.RayGenerationShaderRecord.SizeInBytes = rayGenerationSectionSizeInBytes;
	const uint32_t missSectionSizeInBytes = m_SBT.GetMissSectionSize();
	m_DispatchDesc.MissShaderTable.StartAddress = m_SBTBuff->GetGPUVirtualAddress() + rayGenerationSectionSizeInBytes;
	m_DispatchDesc.MissShaderTable.SizeInBytes = missSectionSizeInBytes;
	m_DispatchDesc.MissShaderTable.StrideInBytes = m_SBT.GetMissEntrySize();
	const uint32_t hitGroupsSectionSize = m_SBT.GetHitGroupSectionSize();
	m_DispatchDesc.HitGroupTable.StartAddress = m_SBTBuff->GetGPUVirtualAddress() +
		rayGenerationSectionSizeInBytes +
		missSectionSizeInBytes;
	m_DispatchDesc.HitGroupTable.SizeInBytes = hitGroupsSectionSize;
	m_DispatchDesc.HitGroupTable.StrideInBytes = m_SBT.GetHitGroupEntrySize();
	m_DispatchDesc.Depth = 1;
	m_DispatchDesc.Width = g.Width();
	m_DispatchDesc.Height = g.Height();

	g.CL().SetPipelineState1(m_State.Get());
	g.CL().DispatchRays(&m_DispatchDesc);
}

void DXRPipeline::Bind(Graphics& g)
{
	g.CL().SetPipelineState1(m_State.Get());
}

DXRParams::DXRParams(Graphics& g)
    :m_Gen(&g.Device())
{
}

void DXRParams::AddLibrary(const std::wstring& shaderPath, const std::initializer_list<std::wstring>& symbols)
{
    auto* lib = nv_helpers_dx12::CompileShaderLibrary(std::move(shaderPath).c_str());
    m_Gen.AddLibrary(lib, std::move(symbols));
    m_Libs.push_back(std::move(lib));
}

void DXRParams::AddRootSig(ComPtr<ID3D12RootSignature> sig, const std::initializer_list<std::wstring>& symbols)
{
    m_Gen.AddRootSignatureAssociation(sig.Get(), std::move(symbols));
    m_Sigs.push_back(std::move(sig));
}

void DXRParams::AddHitGroup(const std::wstring& name, const std::wstring& closestHitSymbol)
{
    m_Gen.AddHitGroup(std::move(name), std::move(closestHitSymbol));
}

void DXRParams::SetMaxRecursion(UINT max)
{
    m_Gen.SetMaxRecursionDepth(max);
}

void DXRParams::SetMaxPayloadSize(UINT max)
{
    m_Gen.SetMaxPayloadSize(max);
}

void DXRParams::SetMaxAttribSize(UINT max)
{
    m_Gen.SetMaxAttributeSize(max);
}

void SBTParams::AddRayGenProg(const std::wstring& entryPoint, const std::initializer_list<void*> inputs)
{
	m_SBT.AddRayGenerationProgram(std::move(entryPoint), std::move(inputs));
}

void SBTParams::AddMissProg(const std::wstring& entryPoint, const std::initializer_list<void*> inputs)
{
	m_SBT.AddMissProgram(std::move(entryPoint), std::move(inputs));
}

void SBTParams::AddHitGroup(const std::wstring& entryPoint, const std::initializer_list<void*> inputs)
{
	m_SBT.AddHitGroup(std::move(entryPoint), std::move(inputs));
}