#include "ShadowTracePass.h"
#include "DXR/Pipeline.h"
#include "Shader/RootSig.h"
#include "DXR/nv_helpers_dx12/RootSignatureGenerator.h"

ShadowTracePass::ShadowTracePass(Graphics& g, const std::string& name, ComPtr<ID3D12Resource> topLevelAS, Shared<Camera> cam)
	:RaytracePass(g, std::move(name), topLevelAS, cam)
{
	DXRParams pipelineParams(g);
	pipelineParams.AddLibrary(L"shaders\\RayGen.hlsl", { L"RayGen" });
	pipelineParams.AddLibrary(L"shaders\\Miss.hlsl", { L"Miss" });
	pipelineParams.AddLibrary(L"shaders\\Hit.hlsl", { L"ClosestHit", L"PlaneClosestHit" });
	pipelineParams.AddLibrary(L"shaders\\ShadowRay.hlsl", { L"ShadowClosestHit", L"ShadowMiss" });
	{
		nv_helpers_dx12::RootSignatureGenerator rsc;
		rsc.AddHeapRangesParameter(
			{
				{
					0 /*u0*/,
					1 /*1 descriptor */,
					0 /*use the implicit register space 0*/,
					D3D12_DESCRIPTOR_RANGE_TYPE_UAV /* UAV representing the output buffer*/,
					0 /*heap slot where the UAV is defined*/
				},
				{
					0 /*t0*/,
					1,
					0,
					D3D12_DESCRIPTOR_RANGE_TYPE_SRV /*Top-level acceleration structure*/,
					1
				},
				{
					0 /*b0*/,
					1,
					0,
					D3D12_DESCRIPTOR_RANGE_TYPE_CBV /*Camera parameters*/,
					2
				}
			});
		auto sig = rsc.Generate(&g.Device(), true);
		pipelineParams.AddRootSig(std::move(sig), { L"RayGen" });
	}
	{
		nv_helpers_dx12::RootSignatureGenerator rsc;
		auto sig = rsc.Generate(&g.Device(), true);
		pipelineParams.AddRootSig(std::move(sig), { L"Miss", L"ShadowMiss" });
	}
	{
		nv_helpers_dx12::RootSignatureGenerator rsc;
		rsc.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_CBV, 0);

		// #DXR Extra - Another ray type
		// Add a single range pointing to the TLAS in the heap
		rsc.AddHeapRangesParameter({
			{
				2 /*t2*/,
				1,
				0,
				D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
				1 /*2nd slot of the heap*/
			},
			{
				1,
				1,
				0,
				D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
				3 /*2nd slot of the heap*/
			}
			});
		auto sig = rsc.Generate(&g.Device(), true);
		pipelineParams.AddRootSig(sig, { L"HitGroup", L"PlaneHitGroup" });
		pipelineParams.AddRootSig(std::move(sig), { L"ShadowHitGroup" });
	}
	pipelineParams.AddHitGroup(L"HitGroup", L"ClosestHit");
	// #DXR Extra: Per-Instance Data
	pipelineParams.AddHitGroup(L"PlaneHitGroup", L"PlaneClosestHit");
	// #DXR Extra - Another ray type
	// Hit group for all geometry when hit by a shadow ray
	pipelineParams.AddHitGroup(L"ShadowHitGroup", L"ShadowClosestHit");

	pipelineParams.SetMaxPayloadSize(sizeof(FLOAT) * 4);
	pipelineParams.SetMaxAttribSize(sizeof(FLOAT) * 2);
	pipelineParams.SetMaxRecursion(2);

	const D3D12_GPU_DESCRIPTOR_HANDLE heapHandle = m_Heap.GPUHandle();
	UINT64* pHeap = reinterpret_cast<UINT64*>(heapHandle.ptr);
	SBTParams sbtParams;
	sbtParams.AddRayGenProg(L"RayGen", { pHeap });
	sbtParams.AddMissProg(L"Miss");
	sbtParams.AddMissProg(L"ShadowMiss");
	sbtParams.AddHitGroup(L"HitGroup");
	sbtParams.AddHitGroup(L"ShadowHitGroup");
	sbtParams.AddHitGroup(L"PlaneHitGroup", { pHeap });
	sbtParams.AddHitGroup(L"ShadowHitGroup");

	m_Pipeline = MakeShared<DXRPipeline>(g, pipelineParams, sbtParams);
}
