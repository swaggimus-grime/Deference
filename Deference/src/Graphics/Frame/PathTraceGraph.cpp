//#include "PathTraceGraph.h"
//#include "DiffuseGIPass.h"
//#include "Entity/Model.h"
//
//PathTraceGraph::PathTraceGraph(Graphics& g)
//{
//	AddGlobalResource("Env", MakeShared<EnvironmentMap>(g, L"textures\\MonValley_G_DirtRoad_3k.hdr"));
//
//	//AddPass<RaytracedGeometryPass>(g);
//	/*AddPass<DiffuseGIPass>(g);*/
//	/*AddPass<AOPass>(g);
//	AddPass<AccumPass>(g);
//	AddPass<HybridOutPass>(g);*/
//}
//
//void PathTraceGraph::FinishScene(Graphics& g)
//{
//	std::vector<Model::Mesh> meshes;
//
//	for (auto& model : GetModels())
//		for (auto& mesh : model->GetMeshes())
//			meshes.push_back(std::move(mesh));
//
//	m_ModelHeap = MakeUnique<CPUShaderHeap>(g, meshes.size() * 4);
//
//	for (auto& mesh : meshes)
//	{
//		m_ModelHeap->Add(g, mesh.m_VB);
//		m_ModelHeap->Add(g, mesh.m_IB);
//		m_ModelHeap->Add(g, mesh.m_DiffuseMap);
//		m_ModelHeap->Add(g, mesh.m_NormalMap);
//	}
//	
//	AddGlobalVectorResource("Models", { m_ModelHeap->CPUStart(), m_ModelHeap->NumDescriptors(), 4 });
//
//	__super::FinishScene(g);
//}
