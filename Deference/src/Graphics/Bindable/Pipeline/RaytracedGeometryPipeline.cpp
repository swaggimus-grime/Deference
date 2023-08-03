#include "RaytracedGeometryPipeline.h"
#include "Shader/RootSig.h"
#include <array>
#include <d3d12.h>
#include <d3dx12.h>

RaytracedGeometryPipeline::RaytracedGeometryPipeline(Graphics& g)
{
	LPCWSTR exports[] = { anyEP, closestEP };

	auto pLib = CreateLibrary(shaderFile);
	CD3DX12_STATE_OBJECT_DESC so(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE);
	{
		auto lib = so.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
		auto bc = CD3DX12_SHADER_BYTECODE(pLib->GetBufferPointer(), pLib->GetBufferSize());
		lib->SetDXILLibrary(&bc);
	}
	{
		auto hg = so.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
		hg->SetAnyHitShaderImport(anyEP);
		hg->SetClosestHitShaderImport(closestEP);
		hg->SetHitGroupExport(hitGroup);
	}
	{
		CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 3, 0);

		CD3DX12_ROOT_PARAMETER1 params[2];
		params[0].InitAsConstantBufferView(0);
		params[1].InitAsDescriptorTable(1, &ranges[0]);

		m_GlobalSig = MakeShared<RootSig>(g, _countof(params), params);
		auto sig = so.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
		sig->SetRootSignature(m_GlobalSig->Sig());
	}
	{
		CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_ROOT_PARAMETER1 params[1];
		params[0].InitAsDescriptorTable(_countof(ranges), &ranges[0], D3D12_SHADER_VISIBILITY_ALL);

		m_RayGenSig = MakeShared<RootSig>(g, _countof(params), params, true);
		auto sig = so.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
		sig->SetRootSignature(m_RayGenSig->Sig());

		auto ass = so.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		ass->AddExport(rayGenEP);
		ass->SetSubobjectToAssociate(*sig);
	}
	{
		CD3DX12_DESCRIPTOR_RANGE1 ranges[4];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1); //Vbuff
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 1); //IBuff
		ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 1); //Diff Map
		ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 1); //Norm Map

		CD3DX12_ROOT_PARAMETER1 params[4];
		params[0].InitAsDescriptorTable(1, &ranges[0]);
		params[1].InitAsDescriptorTable(1, &ranges[1]);
		params[2].InitAsDescriptorTable(1, &ranges[2]);
		params[3].InitAsDescriptorTable(1, &ranges[3]);

		m_HitSig = MakeShared<RootSig>(g, _countof(params), params, true);
		auto sig = so.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
		sig->SetRootSignature(m_HitSig->Sig());

		auto ass = so.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		ass->AddExports(exports);
		ass->SetSubobjectToAssociate(*sig);
	}
	{
		CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

		CD3DX12_ROOT_PARAMETER1 params[1];
		params[0].InitAsDescriptorTable(_countof(ranges), &ranges[0], D3D12_SHADER_VISIBILITY_ALL);

		m_MissSig = MakeShared<RootSig>(g, _countof(params), params, true);
		auto sig = so.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
		sig->SetRootSignature(m_MissSig->Sig());

		auto ass = so.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		ass->AddExport(missEP);
		ass->SetSubobjectToAssociate(*sig);
	}
	{
		auto config = so.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
		config->Config(sizeof(float), sizeof(float) * 2);

		auto ass = so.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		ass->AddExport(rayGenEP);
		ass->AddExport(closestEP);
		ass->AddExport(anyEP);
		ass->AddExport(missEP);
		ass->SetSubobjectToAssociate(*config);
	}
	{
		auto config = so.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
		config->Config(1);
	}

	Create(g, so);
}
