//#include "DiffusePass.h"
//#include <imgui.h>
//#include "Effects/HostDeviceShared.h"
//
//namespace Def
//{
//	DiffusePass::DiffusePass(Graphics& g, const std::string& name, FrameGraph* parent)
//		:RasterPass(g, std::move(name), parent)
//	{
//		AddOutTarget(g, "Target");
//
//		{
//			ConstantBufferLayout layout;
//			layout.Add<CONSTANT_TYPE::XMMATRIX>("world");
//			layout.Add<CONSTANT_TYPE::XMMATRIX>("vp");
//			layout.Add<CONSTANT_TYPE::XMFLOAT3X3>("normMat");
//			m_Transform = MakeUnique<ConstantBuffer>(g, std::move(layout));
//		}
//		{
//			ConstantBufferLayout layout;
//			layout.Add<CONSTANT_TYPE::XMFLOAT3>("pos");
//			m_Camera = MakeUnique<ConstantBuffer>(g, std::move(layout));
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
//		{
//			ConstantBufferLayout layout;
//			layout.Add<CONSTANT_TYPE::XMFLOAT3>("pos");
//			layout.Add<CONSTANT_TYPE::FLOAT>("intensity");
//			layout.Add<CONSTANT_TYPE::XMFLOAT3>("ambient");
//			layout.Add<CONSTANT_TYPE::FLOAT>("specPower");
//			layout.Add<CONSTANT_TYPE::XMFLOAT3>("color");
//			m_Light = MakeUnique<ConstantBuffer>(g, std::move(layout));
//			(*m_Light)["pos"] = XMFLOAT3{ 0.01, 0.01, 0.01 };
//			(*m_Light)["intensity"] = 1.f;
//			(*m_Light)["specPower"] = 16.f;
//			(*m_Light)["color"] = XMFLOAT3{ 1, 1, 1 };
//		}
//	}
//
//	void DiffusePass::Run(Graphics& g)
//	{
//		__super::Run(g);
//
//		auto& scene = m_Parent->GetScene();
//		auto& model = *m_Parent->GetScene().m_Model;
//
//		const auto& cam = *m_Parent->GetScene().m_Camera;
//		(*m_Transform)["vp"] = XMMatrixTranspose(cam.View() * cam.Proj());
//		(*m_Camera)["pos"] = cam.Pos();
//
//		ID3D12DescriptorHeap* heaps[] = { **m_GPUHeap, **m_SamplerHeap };
//		g.CL().SetDescriptorHeaps(2, heaps);
//
//		g.CL().SetGraphicsRootConstantBufferView(0, m_Transform->GetGPUAddress());
//		g.CL().SetGraphicsRootConstantBufferView(1, m_Camera->GetGPUAddress());
//		g.CL().SetGraphicsRootConstantBufferView(2, m_Light->GetGPUAddress());
//		g.CL().SetGraphicsRootConstantBufferView(3, m_Material->GetGPUAddress());
//		g.CL().SetGraphicsRootDescriptorTable(4, GetGlobalVectorResource("ModelTextures")[0][0]);
//		g.CL().SetGraphicsRootDescriptorTable(5, GetGlobalResource("EnvMap"));
//		g.CL().SetGraphicsRootDescriptorTable(6, m_SamplerHeap->GPUStart());
//
//		Rasterize(g, XMMatrixIdentity(), model, model.GetRootNode());
//
//		g.Flush();
//	}
//
//	void DiffusePass::OnSceneLoad(Graphics& g)
//	{
//		const auto& samplers = m_Parent->GetScene().GetModel().GetSamplerDescs();
//		m_SamplerHeap = MakeUnique<SamplerHeap>(g, samplers.size());
//		for (const auto& desc : samplers)
//			m_SamplerHeap->Add(g, desc);
//
//		Model& model = m_Parent->GetScene().GetModel();
//
//		VertexShader vs(L"src\\Effects\\shaders\\geometry.vs.hlsl");
//		PixelShader ps(L"src\\Effects\\shaders\\diffuse.ps.hlsl");
//
//		CD3DX12_DESCRIPTOR_RANGE1 ranges[3];
//		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, MAX_TEXTURES, 0, 1, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, 0u);
//		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 2, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, 0);
//		ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, 0);
//
//		CD3DX12_ROOT_PARAMETER1 rootParameters[7];
//		rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE, D3D12_SHADER_VISIBILITY_VERTEX);
//		rootParameters[1].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE, D3D12_SHADER_VISIBILITY_PIXEL);
//		rootParameters[2].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE, D3D12_SHADER_VISIBILITY_PIXEL);
//		rootParameters[3].InitAsConstantBufferView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE, D3D12_SHADER_VISIBILITY_PIXEL);
//		rootParameters[4].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
//		rootParameters[5].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
//		rootParameters[6].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);
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
//	void DiffusePass::ShowGUI()
//	{
//		auto scene = m_Parent->GetScene();
//		if (ImGui::Begin("Diffuse Pass"))
//		{
//			ImGui::BeginGroup();
//			ImGui::Text("Point Light");
//			ImGui::SliderFloat("Intensity", (*m_Light)["intensity"], 0.f, 100.f);
//			ImGui::SliderFloat("Specular Power", (*m_Light)["specPower"], 0.f, 256.f);
//			auto dim = scene.GetModel().GetBBox().Dim();
//			XMFLOAT3& pos = (*m_Light)["pos"];
//			ImGui::SliderFloat("X", &pos.x, -dim.x, dim.x);
//			ImGui::SliderFloat("Y", &pos.y, -dim.y, dim.y);
//			ImGui::SliderFloat("Z", &pos.z, -dim.z, dim.z);
//			ImGui::SliderFloat3("Ambient Color", (*m_Light)["ambient"], 0.f, 1.f);
//			ImGui::SliderFloat3("Diffuse Color", (*m_Light)["color"], 0.f, 1.f);
//			ImGui::EndGroup();
//		}
//		ImGui::End();
//	}
//
//	void DiffusePass::Rasterize(Graphics& g, XMMATRIX parentTransform, Model& model, Shared<SceneNode> node)
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