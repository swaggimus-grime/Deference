//#include "RasterGraph.h"
//#include "Scene/Model.h"
//#include "Effects/HostDeviceShared.h"
//
//namespace Def
//{
//	RasterGraph::RasterGraph(Graphics& g)
//	{
//		//AddPass<DiffusePass>(g, "Diffuse");
//		/*{
//			auto pass = AddPass<ToneMapPass>(g, "Tonemap");
//			pass->Link("Diffuse", "Target", "HDR");
//		}*/
//	}
//
//	void RasterGraph::PrepLoadScene(Graphics& g)
//	{
//		auto model = GetScene().GetModel();
//		auto meshes = model.GetMeshes();
//		auto textures = model.GetTextures();
//		m_ModelHeap = MakeUnique<CPUShaderHeap>(g, MAX_TEXTURES + 1);
//		for (auto& tex : textures)
//			m_ModelHeap->Add(g, tex);
//
//		AddGlobalVectorResource("ModelTextures", { m_ModelHeap->CPUStart(), MAX_TEXTURES, 1});
//
//		D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
//		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
//		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
//		desc.Texture2D.MipLevels = 1;
//		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//		for (UINT i = textures.size(); i < MAX_TEXTURES; i++)
//		{
//			HCPU current = m_ModelHeap->GetCurrentHCPU();
//			g.Device().CreateShaderResourceView(nullptr, &desc, current);
//			m_ModelHeap->IncrementHandle();
//		}
//
//		m_EnvMap = MakeShared<CubeMap>(g, L"res\\textures\\wood-cubemap.dds");
//		AddGlobalResource("EnvMap", m_EnvMap);
//		m_ModelHeap->Add(g, m_EnvMap);
//	}
//
//}