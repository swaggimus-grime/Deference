#include "PathTraceGraph.h"
#include "DiffuseGIPass.h"
#include "Entity/Model.h"
#include "CopyPass.h"

PathTraceGraph::PathTraceGraph(Graphics& g, Scene& scene)
	:FrameGraph(scene)
{
	std::vector<Model::Mesh> meshes;

	for (auto& model : GetModels())
		for (auto& mesh : model->GetMeshes())
			meshes.push_back(std::move(mesh));

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
	auto env = MakeShared<EnvironmentMap>(g, L"textures\\MonValley_G_DirtRoad_3k.hdr");
	env->Transition(g, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	AddGlobalResource("Env", env);
	AddGlobalResource("TLAS", MakeShared<TLAS>(g,
		std::views::iota(0u, (UINT)scene.m_Models.size()) |
		std::views::transform([&](UINT i) {
			return scene.m_Models[i]->GetBLAS();
			}) |
		std::ranges::to<std::vector>()
				));

	Finish(g);
}

void PathTraceGraph::RecordPasses(Graphics& g)
{
	AddPass<RaytracedGeometryPass>(g, "Geometry")->Finish(g);
	{
		auto pass = AddPass<DiffuseGIPass>(g, "Diffuse and GI");
		pass->Link("Geometry", "Position");
		pass->Link("Geometry", "Normal");
		pass->Link("Geometry", "Albedo");
		pass->Link("Geometry", "Specular");
		pass->Link("Geometry", "Emissive");
		pass->Finish(g);
	}
	{
		auto pass = AddPass<AccumPass>(g, "GI accum");
		pass->Link("Diffuse and GI", "Target");
		pass->Finish(g);
	}
	{
		auto pass = AddPass<ToneMapPass>(g, "Tonemap");
		pass->Link("GI accum", "Target", "HDR");
		pass->Finish(g);
	}
	{
		auto pass = AddPass<CopyPass>(g, "Copy");
		pass->Link("Tonemap", "SDR", "Target");
		pass->Finish(g);
	}

	FinishRecordingPasses();
}
