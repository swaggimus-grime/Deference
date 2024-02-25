//#include "GeometryPass.h"
//
//namespace Def
//{
//	GeometryPass::GeometryPass(Graphics& g, const std::string& name, FrameGraph* parent)
//		:RasterPass(g, std::move(name), parent)
//	{
//		AddOutTarget(g, "Position", DXGI_FORMAT_R32G32B32A32_FLOAT);
//		AddOutTarget(g, "Normal", DXGI_FORMAT_R32G32B32A32_FLOAT);
//		AddOutTarget(g, "Albedo");
//		AddOutTarget(g, "Specular");
//		AddOutTarget(g, "Emissive");
//
//		QueryGlobalVectorResource("ModelTextures");
//
//		{
//			ConstantBufferLayout layout;
//			layout.Add<CONSTANT_TYPE::XMMATRIX>("world");
//			layout.Add<CONSTANT_TYPE::XMMATRIX>("vp");
//			layout.Add<CONSTANT_TYPE::XMFLOAT3X3>("normMat");
//			m_Transform = MakeShared<ConstantBuffer>(g, std::move(layout));
//		}
//		{
//			ConstantBufferLayout layout;
//			layout.Add<CONSTANT_TYPE::XMFLOAT3>("pos");
//			m_Camera = MakeShared<ConstantBuffer>(g, std::move(layout));
//		}
//		{
//			ConstantBufferLayout layout;
//			layout.Add<CONSTANT_TYPE::XMFLOAT4>("BaseColor");
//			layout.Add<CONSTANT_TYPE::FLOAT>("Roughness");
//			layout.Add<CONSTANT_TYPE::FLOAT>("Metallic");
//			layout.Add<CONSTANT_TYPE::UINT>("BaseTex");
//			layout.Add<CONSTANT_TYPE::UINT>("RoughMetallicTex");
//			layout.Add<CONSTANT_TYPE::UINT>("NormalTex");
//			layout.Add<CONSTANT_TYPE::UINT>("OcclusionTex");
//			layout.Add<CONSTANT_TYPE::UINT>("EmissiveTex");
//
//			m_Material = MakeUnique<ConstantBuffer>(g, std::move(layout));
//		}
//	}
//
//	void GeometryPass::Run(Graphics& g)
//	{
//		__super::Run(g);
//
//		auto& scene = m_Parent->GetScene();
//		auto& model = scene.GetModel();
//
//		const auto& cam = scene.GetCamera();
//		(*m_Transform)["vp"] = XMMatrixTranspose(cam.View() * cam.Proj());
//		(*m_Camera)["pos"] = cam.Pos();
//
//		ID3D12DescriptorHeap* heaps[] = { **m_GPUHeap, **m_SamplerHeap };
//		g.CL().SetDescriptorHeaps(2, heaps);
//
//		m_Pipeline->Bind(g);
//
//		auto start = m_GPUHeap->GPUStart();
//		g.CL().SetGraphicsRootConstantBufferView(0, m_Transform->GetGPUAddress());
//		g.CL().SetGraphicsRootConstantBufferView(1, m_Camera->GetGPUAddress());
//		g.CL().SetGraphicsRootConstantBufferView(2, m_Material->GetGPUAddress());
//		g.CL().SetGraphicsRootDescriptorTable(3, start);
//		g.CL().SetGraphicsRootDescriptorTable(4, m_SamplerHeap->GPUStart());
//
//		Rasterize(g, XMMatrixIdentity(), model, model.GetRootNode());
//
//		g.Flush();
//	}
//
//	void GeometryPass::OnSceneLoad(Graphics& g)
//	{
//		const auto& samplers = m_Parent->GetScene().GetModel().GetSamplerDescs();
//		m_SamplerHeap = MakeUnique<SamplerHeap>(g, samplers.size());
//		for (const auto& desc : samplers)
//			m_SamplerHeap->Add(g, desc);
//
//		Model& model = m_Parent->GetScene().GetModel();
//
//		VertexShader vs(L"shaders\\geometry.vs.hlsl");
//		PixelShader ps(L"shaders\\geometry.ps.hlsl");
//
//		CD3DX12_DESCRIPTOR_RANGE1 ranges[2]{};
//		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, model.GetTextures().size(), 0, 1, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, 0u);
//		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
//
//		CD3DX12_ROOT_PARAMETER1 rootParameters[5];
//		rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE, D3D12_SHADER_VISIBILITY_VERTEX);
//		rootParameters[1].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE, D3D12_SHADER_VISIBILITY_PIXEL);
//		rootParameters[2].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE, D3D12_SHADER_VISIBILITY_PIXEL);
//		rootParameters[3].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
//		rootParameters[4].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
//
//		auto& outs = GetOutTargets();
//		const auto& formats =
//			std::views::iota(outs.begin(), outs.end()) |
//			std::views::transform([&](const auto& it) {
//			return (*it).second->GetFormat();
//				}) |
//			std::ranges::to<std::vector>();
//
//				auto sig = MakeShared<RootSig>(g, _countof(rootParameters), rootParameters);
//				m_Pipeline = MakeUnique<Pipeline>(g, std::move(sig), vs, ps, model.GetLayout(), formats);
//	}
//
//	void GeometryPass::Rasterize(Graphics& g, XMMATRIX parentTransform, Model& model, Shared<SceneNode> node)
//	{
//		XMMATRIX world = parentTransform * node->Transform;
//		if (node->Id != -1)
//		{
//			(*m_Transform)["world"] = XMMatrixTranspose(world);
//			(*m_Transform)["normMat"] = XMMatrixInverse(nullptr, world);
//
//			auto& mesh = model.GetMeshes()[node->Id];
//			for (auto& sm : mesh.m_SubMeshes)
//			{
//				auto mat = model.GetMaterials()[sm.m_Material];
//				(*m_Material)["BaseColor"] = mat.BaseColor;
//				(*m_Material)["Roughness"] = mat.Roughness;
//				(*m_Material)["Metallic"] = mat.Metallic;
//				(*m_Material)["BaseTex"] = mat.m_TextureIds["Base"];
//				(*m_Material)["RoughMetallicTex"] = mat.m_TextureIds["RoughMetallic"];
//				(*m_Material)["NormalTex"] = mat.m_TextureIds["Normal"];
//				(*m_Material)["OcclusionTex"] = mat.m_TextureIds["Occlusion"];
//				(*m_Material)["EmissiveTex"] = mat.m_TextureIds["Emissive"];
//				sm.Rasterize(g);
//			}
//		}
//
//		for (auto& child : node->Children)
//			Rasterize(g, world, model, child);
//	}
//
//}