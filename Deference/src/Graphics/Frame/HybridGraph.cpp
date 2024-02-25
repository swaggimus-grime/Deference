//#include "HybridGraph.h"
//#include "Scene/Model.h"
//
//namespace Def
//{
//	HybridGraph::HybridGraph(Graphics& g)
//	{
//		AddPass<GeometryPass>(g, "Geometry");
//	}
//
//	void HybridGraph::PrepLoadScene(Graphics& g)
//	{
//		auto model = GetScene().GetModel();
//		auto meshes = model.GetMeshes();
//		auto textures = model.GetTextures();
//		m_ModelHeap = MakeUnique<CPUShaderHeap>(g, textures.size());
//		for (auto& tex : textures)
//		{
//			m_ModelHeap->Add(g, tex);
//		}
//
//		AddGlobalVectorResource("ModelTextures", { m_ModelHeap->CPUStart(), m_ModelHeap->NumDescriptors(), 1 });
//
//		/*std::vector<ComPtr<ID3D12Resource>> blass = { model.GetBLAS() };
//		AddGlobalResource("TLAS", MakeShared<TLAS>(g, std::move(blass)));
//
//		auto env = MakeShared<EnvironmentMap>(g, L"textures\\MonValley_G_DirtRoad_3k.hdr");
//		env->Transition(g, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
//		AddGlobalResource("Env", env);*/
//	}
//
//}