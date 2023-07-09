//#include "AOPipeline.h"
//#include "Shader/RootSig.h"
//#include <array>
//#include <d3d12.h>
//#include <d3dx12.h>
//
//static LPCWSTR shaderFile = L"shaders\\ao.rt.hlsl";
//static LPCWSTR rayGenEP = L"DiffuseAndHardShadow";
//static LPCWSTR hitGroup = L"ShadowHit";
//static LPCWSTR anyEP = L"ShadowAnyHit";
//static LPCWSTR closestEP = L"ShadowClosestHit";
//
//AOPipeline::AOPipeline(Graphics& g, const CSUHeap& heap)
//{
//	auto pLib = CreateLibrary(shaderFile);
//	CD3DX12_STATE_OBJECT_DESC so(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE);
//	{
//		auto lib = so.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
//		auto bc = CD3DX12_SHADER_BYTECODE(pLib->GetBufferPointer(), pLib->GetBufferSize());
//		lib->SetDXILLibrary(&bc);
//	}
//	{
//		auto hg = so.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
//		hg->SetAnyHitShaderImport(anyEP);
//		hg->SetClosestHitShaderImport(closestEP);
//		hg->SetHitGroupExport(hitGroup);
//	}
//	{
//		RootParams params;
//		DescTable table;
//		table.AddRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
//		table.AddRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1);
//		table.AddRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2);
//		table.AddRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3);
//		table.AddRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
//		params.AddTable(std::move(table), D3D12_SHADER_VISIBILITY_ALL);
//		m_RayGenSig = MakeShared<RootSig>(g, std::move(params), true);
//		auto sig = so.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
//		sig->SetRootSignature(m_RayGenSig->Sig());
//
//		auto ass = so.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
//		ass->AddExport(rayGenEP);
//		ass->SetSubobjectToAssociate(*sig);
//	}
//	{
//		RootParams params;
//		m_HitSig = MakeShared<RootSig>(g, std::move(params), true);
//		auto sig = so.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
//		sig->SetRootSignature(m_HitSig->Sig());
//
//		auto ass = so.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
//		ass->AddExport(anyEP);
//		ass->AddExport(closestEP);
//		ass->SetSubobjectToAssociate(*sig);
//	}
//	{
//		auto config = so.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
//		config->Config(sizeof(int), sizeof(float) * 2);
//
//		auto ass = so.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
//		ass->AddExport(rayGenEP);
//		ass->AddExport(anyEP);
//		ass->AddExport(closestEP);
//		ass->SetSubobjectToAssociate(*config);
//	}
//	{
//		auto config = so.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
//		config->Config(2);
//	}
//
//	Create(g, so,
//		{
//			{rayGenEP, {heap.GPUStart()}}
//		},
//		{
//		},
//		{
//			{hitGroup, {}}
//		}
//		);
//}
//
