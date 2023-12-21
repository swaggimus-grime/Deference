#include "DiffuseGIPipeline.h"
#include "Shader/RootSig.h"
#include <array>
#include <d3d12.h>
#include <d3dx12.h>

DiffuseGIPipeline::DiffuseGIPipeline(Graphics& g)
{
	auto pLib = CreateLibrary(shaderFile);
	CD3DX12_STATE_OBJECT_DESC so(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE);
	{
		auto lib = so.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
		auto bc = CD3DX12_SHADER_BYTECODE(pLib->GetBufferPointer(), pLib->GetBufferSize());
		lib->SetDXILLibrary(&bc);
	}
	{
		auto hg = so.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
		hg->SetClosestHitShaderImport(shadowClosest);
		hg->SetAnyHitShaderImport(shadowAny);
		hg->SetHitGroupExport(shadowGroup);
	}
	{
		auto hg = so.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
		hg->SetClosestHitShaderImport(indirectClosest);
		hg->SetAnyHitShaderImport(indirectAny);
		hg->SetHitGroupExport(indirectGroup);
	}
	{
		CD3DX12_DESCRIPTOR_RANGE1 ranges[8];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); //Pos
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1); //Norm
		ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2); //Albedo
		ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3); //Specular
		ranges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4); //Emissive
		ranges[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5); //Scene
		ranges[6].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6); //Output
		ranges[7].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); //Scene

		CD3DX12_ROOT_PARAMETER1 params[10];
		params[0].InitAsDescriptorTable(1, &ranges[0]);
		params[1].InitAsDescriptorTable(1, &ranges[1]);
		params[2].InitAsDescriptorTable(1, &ranges[2]);
		params[3].InitAsDescriptorTable(1, &ranges[3]);
		params[4].InitAsDescriptorTable(1, &ranges[4]);
		params[5].InitAsDescriptorTable(1, &ranges[5]);
		params[6].InitAsDescriptorTable(1, &ranges[6]);
		params[7].InitAsDescriptorTable(1, &ranges[7]);
		params[8].InitAsConstantBufferView(0); //light
		params[9].InitAsConstantBufferView(1); //

		auto pSig = MakeUnique<RootSig>(g, _countof(params), params);
		auto sig = so.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
		sig->SetRootSignature(**pSig);
		SetGlobalSig(std::move(pSig));
	}
	{		
		CD3DX12_DESCRIPTOR_RANGE1 ranges[6];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1); //Vbuff
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 1); //IBuff
		ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 1); //Diff Map
		ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 1); //Norm Map
		ranges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 1); //Spec Map
		ranges[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5, 1); //Emissive Map

		CD3DX12_ROOT_PARAMETER1 params[6];
		params[0].InitAsDescriptorTable(1, &ranges[0]);
		params[1].InitAsDescriptorTable(1, &ranges[1]);
		params[2].InitAsDescriptorTable(1, &ranges[2]);
		params[3].InitAsDescriptorTable(1, &ranges[3]);
		params[4].InitAsDescriptorTable(1, &ranges[4]);
		params[5].InitAsDescriptorTable(1, &ranges[5]);
		
		auto pSig = MakeUnique<RootSig>(g, _countof(params), params, true);
		auto sig = so.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
		sig->SetRootSignature(**pSig);

		auto ass = so.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		ass->AddExport(indirectClosest);
		ass->AddExport(indirectAny);
		ass->SetSubobjectToAssociate(*sig);
		AddLocalSig(std::move(pSig));
	}
	{
		auto pSig = MakeUnique<RootSig>(g, 0, nullptr, true);
		auto sig = so.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
		sig->SetRootSignature(**pSig);

		auto ass = so.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		ass->AddExport(shadowAny);
		ass->AddExport(shadowClosest);
		ass->AddExport(shadowMiss);
		ass->AddExport(rayGenEP);
		ass->AddExport(indirectMiss);
		ass->SetSubobjectToAssociate(*sig);
		AddLocalSig(std::move(pSig));
	}
	{
		auto config = so.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
		config->Config(sizeof(float) * 3 + sizeof(UINT) * 2, sizeof(float) * 2);

		auto ass = so.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		ass->AddExport(indirectClosest);
		ass->AddExport(indirectMiss);
		ass->AddExport(indirectAny);
		ass->AddExport(shadowAny);
		ass->AddExport(shadowClosest);
		ass->AddExport(shadowMiss);
		ass->AddExport(rayGenEP);
		ass->SetSubobjectToAssociate(*config);
	}
	{
		auto config = so.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
		config->Config(D3D12_RAYTRACING_MAX_DECLARABLE_TRACE_RECURSION_DEPTH);
	}

	Create(g, so);
}

