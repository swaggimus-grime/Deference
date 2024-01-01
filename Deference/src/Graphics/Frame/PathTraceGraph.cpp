#include "PathTraceGraph.h"
#include "DiffuseGIPass.h"
#include "Scene/Model.h"
#include "CopyPass.h"

PathTraceGraph::PathTraceGraph(Graphics& g)
{
	AddPass<RaytracedGeometryPass>(g, "Geometry");
	{
		auto pass = AddPass<DiffuseGIPass>(g, "Diffuse and GI");
		pass->Link("Geometry", "Position");
		pass->Link("Geometry", "Normal");
		pass->Link("Geometry", "Albedo");
		pass->Link("Geometry", "Specular");
		pass->Link("Geometry", "Emissive");
	}
	{
		auto pass = AddPass<AccumPass>(g, "GI accum");
		pass->Link("Diffuse and GI", "Target");
	}
	{
		auto pass = AddPass<ToneMapPass>(g, "Tonemap");
		pass->Link("GI accum", "Target", "HDR");
	}
}

void PathTraceGraph::PrepLoadScene(Graphics& g)
{
	auto model = GetScene().GetModel();
	auto meshes = model.GetMeshes();
	m_ModelHeap = MakeUnique<CPUShaderHeap>(g, meshes.size() * 6);
	for (auto& mesh : meshes)
	{
		m_ModelHeap->Add(g, mesh.m_VB);
		m_ModelHeap->Add(g, mesh.m_IB);
		m_ModelHeap->Add(g, mesh.m_DiffuseMap);
		m_ModelHeap->Add(g, mesh.m_NormalMap);
		m_ModelHeap->Add(g, mesh.m_SpecularMap);
		m_ModelHeap->Add(g, mesh.m_EmissiveMap);
	}

	AddGlobalVectorResource("Models", { m_ModelHeap->CPUStart(), m_ModelHeap->NumDescriptors(), 6 });

	std::vector<ComPtr<ID3D12Resource>> blass = { model.GetBLAS() };
	AddGlobalResource("TLAS", MakeShared<TLAS>(g, std::move(blass)));

	auto env = MakeShared<EnvironmentMap>(g, L"textures\\MonValley_G_DirtRoad_3k.hdr");
	env->Transition(g, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	AddGlobalResource("Env", env);
}
