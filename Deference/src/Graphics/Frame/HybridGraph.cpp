#include "HybridGraph.h"
#include "Entity/Model.h"
#include "GeometryPass.h"
#include "HybridOutPass.h"
#include "ToneMapPass.h"

HybridGraph::HybridGraph(Graphics& g, Scene& scene)
	:FrameGraph(scene)
{
	std::vector<Model::Mesh> meshes;

	for (auto& model : GetModels())
		for (auto& mesh : model->GetMeshes())
			meshes.push_back(std::move(mesh));

	m_ModelHeap = MakeUnique<CPUShaderHeap>(g, meshes.size() * 5);

	for (auto& mesh : meshes)
	{
		m_ModelHeap->Add(g, mesh.m_VB);
		m_ModelHeap->Add(g, mesh.m_IB);
		m_ModelHeap->Add(g, mesh.m_DiffuseMap);
		m_ModelHeap->Add(g, mesh.m_NormalMap);
		m_ModelHeap->Add(g, mesh.m_SpecularMap);
	}

	AddGlobalVectorResource("Models", { m_ModelHeap->CPUStart(), m_ModelHeap->NumDescriptors(), 5 });
	AddGlobalResource("Env", MakeShared<EnvironmentMap>(g, L"textures\\MonValley_G_DirtRoad_3k.hdr"));
	AddGlobalResource("TLAS", MakeShared<TLAS>(g,
		std::views::iota(0u, (UINT)scene.m_Models.size()) |
		std::views::transform([&](UINT i) {
			return scene.m_Models[i]->GetBLAS();
			}) |
		std::ranges::to<std::vector>()
				));

	Finish(g);
}

void HybridGraph::RecordPasses(Graphics& g)
{
	AddPass<GeometryPass>(g, "Geometry")->Finish(g);
	{
		auto pass = AddPass<DiffuseGIPass>(g, "Diffuse and GI");
		pass->Link("Geometry", "Position");
		pass->Link("Geometry", "Normal");
		pass->Link("Geometry", "Albedo");
		pass->Link("Geometry", "Specular");
		pass->Finish(g);
	}
	{
		auto pass = AddPass<AccumPass>(g, "GI accum");
		pass->Link("Diffuse and GI", "Target");
		pass->Finish(g);
	}
	{
		auto pass = AddPass<AOPass>(g, "AO");
		pass->Link("Geometry", "Position");
		pass->Link("Geometry", "Normal");
		pass->Finish(g);
	}
	{
		auto pass = AddPass<AccumPass>(g, "AO accum");
		pass->Link("AO", "Target");
		pass->Finish(g);
	}
	{
		auto pass = AddPass<HybridOutPass>(g, "Hybrid");
		pass->Link("Geometry", "Position");
		pass->Link("GI accum", "Target", "Diffuse");
		pass->Link("AO accum", "Target", "AO");
		pass->Finish(g);
	}
	{
		auto pass = AddPass<ToneMapPass>(g, "Tonemap");
		pass->Link("Hybrid", "Target", "HDR");
		pass->Finish(g);
	}

	FinishRecordingPasses();
}
