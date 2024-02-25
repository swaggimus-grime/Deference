#include "DiffuseGIPass.h"
#include "Scene/Camera.h"
#include "Resource/ConstantBuffer.h"
#include <imgui.h>
#include "VecOps.h"

namespace Def
{
	DiffuseGIPass::DiffuseGIPass(Graphics& g, const std::string& name, FrameGraph* parent)
		:RaytracePass(std::move(name), parent), m_FrameCount(0)
	{
		AddInTarget("Position");
		AddInTarget("Normal");
		AddInTarget("Albedo");
		AddInTarget("Specular");
		AddInTarget("Emissive");
		AddOutTarget(g, "Target");

		{
			ConstantBufferLayout layout;
			layout.Add<CONSTANT_TYPE::XMFLOAT3>("pos");
			layout.Add<CONSTANT_TYPE::FLOAT>("intensity");
			layout.Add<CONSTANT_TYPE::XMFLOAT3>("color");
			layout.Add<CONSTANT_TYPE::FLOAT>("emissive");
			layout.Add<CONSTANT_TYPE::UINT>("on");
			m_Light = MakeShared<ConstantBuffer>(g, std::move(layout));
			(*m_Light)["pos"] = XMFLOAT3{ 0, 0.f, 0.f };
			(*m_Light)["color"] = XMFLOAT3{ 1, 1, 1 };
			(*m_Light)["intensity"] = 1.f;
			(*m_Light)["emissive"] = 1.f;
			(*m_Light)["on"] = 1u;
			AddResource(m_Light);
		}
		{
			ConstantBufferLayout layout;
			layout.Add<CONSTANT_TYPE::XMFLOAT3>("camPos");
			layout.Add<CONSTANT_TYPE::UINT>("maxRec");
			layout.Add<CONSTANT_TYPE::UINT>("frameCount");
			layout.Add<CONSTANT_TYPE::FLOAT>("minT");
			layout.Add<CONSTANT_TYPE::UINT>("on");
			m_Constants = MakeShared<ConstantBuffer>(g, std::move(layout));
			(*m_Constants)["maxRec"] = 1;
			(*m_Constants)["minT"] = 0.001f;
			(*m_Constants)["on"] = 1u;

			AddResource(m_Constants);
		}
	}

	void DiffuseGIPass::OnSceneLoad(Graphics& g)
	{
		RaytracingPipeline::Desc desc = {};
		ComPtr<IDxcBlob> pLib;
		DXC::Compile(shaderFile, pLib);
		CD3DX12_STATE_OBJECT_DESC so(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE);
		{
			auto lib = so.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
			auto bc = CD3DX12_SHADER_BYTECODE(pLib->GetBufferPointer(), pLib->GetBufferSize());
			lib->SetDXILLibrary(&bc);
		}
		{
			auto hg = so.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
			hg->SetClosestHitShaderImport(shadowClosest);
			hg->SetAnyHitShaderImport(shadowAny);
			hg->SetHitGroupExport(shadowGroup);
		}
		{
			auto hg = so.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
			hg->SetClosestHitShaderImport(indirectClosest);
			hg->SetAnyHitShaderImport(indirectAny);
			hg->SetHitGroupExport(indirectGroup);
		}
		//Global
		{
			CD3DX12_DESCRIPTOR_RANGE1 ranges[6];
			ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0);     
			ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5);     
			ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6);     
			ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);     
			ranges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, MAX_TEXTURES, 0, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
			ranges[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0); 

			CD3DX12_ROOT_PARAMETER1 params[9];
			params[0].InitAsDescriptorTable(1, &ranges[0]); //Inputs
			params[1].InitAsDescriptorTable(1, &ranges[1]);	//TLAS
			params[2].InitAsDescriptorTable(1, &ranges[2]);	//Env
			params[3].InitAsDescriptorTable(1, &ranges[3]);	//UAV Output
			params[4].InitAsDescriptorTable(1, &ranges[4]); //Textures
			params[5].InitAsDescriptorTable(1, &ranges[5]);	//Sampler
			params[6].InitAsShaderResourceView(5, 1); //Materials
			params[7].InitAsConstantBufferView(0); //light
			params[8].InitAsConstantBufferView(1); //constants

			auto pSig = MakeUnique<RootSig>(g, _countof(params), params);
			auto sig = so.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
			sig->SetRootSignature(**pSig);
			desc.m_GlobalSig = std::move(pSig);
		}
		//Indirect hit group
		{
			CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
			ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0, 1);

			CD3DX12_ROOT_PARAMETER1 params[2];
			params[0].InitAsConstants(2, 0, 1);
			params[1].InitAsDescriptorTable(1, &ranges[0]);

			auto pSig = MakeUnique<RootSig>(g, _countof(params), params, true);
			auto sig = so.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
			sig->SetRootSignature(**pSig);

			auto ass = so.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
			ass->AddExport(indirectClosest);
			ass->AddExport(indirectAny);
			ass->SetSubobjectToAssociate(*sig);
			desc.m_LocalSigs.push_back(std::move(pSig));
		}
		{
			auto pSig = MakeUnique<RootSig>(g, 0, nullptr, true);
			auto sig = so.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
			sig->SetRootSignature(**pSig);

			auto ass = so.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
			ass->AddExport(shadowAny);
			ass->AddExport(shadowClosest);
			ass->AddExport(shadowMiss);
			ass->AddExport(rayGenEP);
			ass->AddExport(indirectMiss);
			ass->SetSubobjectToAssociate(*sig);
			desc.m_LocalSigs.push_back(std::move(pSig));
		}
		{
			auto config = so.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
			config->Config(sizeof(float) * 3 + sizeof(UINT) * 2, sizeof(float) * 2);

			auto ass = so.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
			ass->AddExport(indirectClosest);
			ass->AddExport(indirectMiss);
			ass->AddExport(indirectAny);
			ass->AddExport(shadowAny);
			ass->AddExport(shadowClosest);
			ass->AddExport(shadowMiss);
			ass->AddExport(rayGenEP);
			ass->SetSubobjectToAssociate(*config);
		}
		{
			auto config = so.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
			config->Config(D3D12_RAYTRACING_MAX_DECLARABLE_TRACE_RECURSION_DEPTH);
		}
		m_Pipeline = MakeUnique<RaytracingPipeline>(g, so);

		auto& globals = m_Parent->GetGlobals<RaytraceGraph::Globals>();
		{
			auto table = MakeUnique<ShaderBindTable>(g, m_Pipeline.get(), 1, 0);
			table->Add(rayGenEP);
			desc.m_RayGenTable = std::move(table);
		}
		{
			auto table = MakeUnique<ShaderBindTable>(g, m_Pipeline.get(), 2, 0);
			table->Add(indirectMiss);
			table->Add(shadowMiss);
			desc.m_MissTable = std::move(table);
		}
		{
			auto table = MakeUnique<ShaderBindTable>(g, m_Pipeline.get(), globals.Geometries.size() * 2, sizeof(HitEntry));

			for (auto& geo : globals.Geometries)
			{
				HitEntry entry{};
				entry.v = m_GPUHeap->GetHGPU(geo.Vertices); //geo.Vertices->GetGPUAddress();
				entry.i = m_GPUHeap->GetHGPU(geo.Indices);  //geo.Indices->GetGPUAddress();
				entry.uv = m_GPUHeap->GetHGPU(geo.UVs);      //geo.UVs->GetGPUAddress();
				entry.n = m_GPUHeap->GetHGPU(geo.Normals);  //geo.Normals->GetGPUAddress();
				entry.t = m_GPUHeap->GetHGPU(geo.Tangents); //geo.Tangents->GetGPUAddress();
				entry.m = geo.MaterialID;
				entry.istride = geo.Indices->GetStride();

				table->Add(shadowGroup);
				table->Add(indirectGroup, &entry, sizeof(entry));
			}

			desc.m_HitTable = std::move(table);
		}

		m_Pipeline->SubmitTablesAndSigs(g, desc);
	}

	void DiffuseGIPass::ShowGUI()
	{
		auto scene = m_Parent->GetScene();
		if (ImGui::Begin("Diffuse Pass"))
		{
			ImGui::BeginGroup();
			ImGui::Text("Direct Light");
			ImGui::Checkbox("On", (*m_Light)["on"]);
			ImGui::Text("Position");
			auto dim = scene.m_Model->GetBBox().Dim();
			XMFLOAT3& pos = (*m_Light)["pos"];
			ImGui::SliderFloat("X", &pos.x, -dim.x, dim.x);
			ImGui::SliderFloat("Y", &pos.y, -dim.y, dim.y);
			ImGui::SliderFloat("Z", &pos.z, -dim.z, dim.z);
			ImGui::SliderFloat3("Color", (*m_Light)["color"], 0.f, 1.f);
			ImGui::SliderFloat("Intensity", (*m_Light)["intensity"], 0.f, 10.f);
			ImGui::SliderFloat("Emissive", (*m_Light)["emissive"], 0.f, 10.f);
			ImGui::EndGroup();

			ImGui::BeginGroup();
			ImGui::Text("Indirect Light");
			ImGui::Checkbox("On", (*m_Constants)["on"]);
			ImGui::SliderInt("Max Recursion", ((*m_Constants)["maxRec"]), 1, 30);
			ImGui::SliderFloat("minT", (*m_Constants)["minT"], 0.001f, 1.f);
			ImGui::EndGroup();
		}
		ImGui::End();
	}

	void DiffuseGIPass::Run(Graphics& g)
	{
		(*m_Constants)["camPos"] = m_Parent->GetScene().m_Camera->Pos();
		(*m_Constants)["frameCount"] = m_FrameCount++;

		auto c = Commander<D3D12_RESOURCE_BARRIER>::Init();
		for (auto& in : GetInTargets())
			c.Add(in.second->Transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
		c.Transition(g);

		m_Pipeline->Bind(g);
		Commander<ID3D12DescriptorHeap*>::Init()
			.Add(**m_GPUHeap)
			.Add(**m_SamplerHeap)
			.Bind<DescriptorHeap<View>>(g);

		auto& globals = m_Parent->GetGlobals<RaytraceGraph::Globals>();
		auto& model = m_Parent->GetScene().m_Model;
		auto& textures = model->GetTextures();
		auto& samplers = model->GetSamplers();

		g.CL().SetComputeRootDescriptorTable(0, m_GPUHeap->GetHGPU(GetInTargets()[0].second));
		g.CL().SetComputeRootDescriptorTable(1, m_GPUHeap->GetHGPU(globals.TLAS));
		g.CL().SetComputeRootDescriptorTable(2, m_GPUHeap->GetHGPU(globals.Env));
		g.CL().SetComputeRootDescriptorTable(3, m_GPUHeap->GetHGPU(m_UAVs.at(0).second));
		g.CL().SetComputeRootDescriptorTable(4, m_GPUHeap->GetHGPU(textures[0]));
		g.CL().SetComputeRootDescriptorTable(5, m_SamplerHeap->GPUStart());
		g.CL().SetComputeRootShaderResourceView(6, globals.MatBuff->GetGPUAddress());
		g.CL().SetComputeRootConstantBufferView(7, m_Light->GetGPUAddress());
		g.CL().SetComputeRootConstantBufferView(8, m_Constants->GetGPUAddress());

		m_Pipeline->Dispatch(g);

		c = Commander<D3D12_RESOURCE_BARRIER>::Init();
		for (auto& in : GetInTargets())
			c.Add(in.second->Transition(D3D12_RESOURCE_STATE_RENDER_TARGET));
		c.Transition(g);
		CopyUAVsToRTs(g);
	    g.Flush();
	}


	void DiffuseGIPass::PrepLoadScene(Graphics& g)
	{
		auto& globals = m_Parent->GetGlobals<RaytraceGraph::Globals>();
		auto& model = m_Parent->GetScene().m_Model;
		auto& textures = model->GetTextures();
		for (auto& t : textures)
			AddResource(t);

		for (UINT i = textures.size(); i < MAX_TEXTURES; i++)
			AddResource(textures[0], true);

		for (auto& geo : globals.Geometries)
		{
			AddResource(geo.Vertices);
			AddResource(geo.Indices);
			AddResource(geo.UVs);
			AddResource(geo.Normals);
			AddResource(geo.Tangents);
		}

		AddResource(globals.MatBuff);
		AddResource(globals.TLAS);
		AddResource(globals.Env);

		auto& samplers = model->GetSamplers();
		m_SamplerHeap = MakeUnique<SamplerHeap>(g, samplers.size());
		for (auto& s : samplers)
			m_SamplerHeap->Add(g, s);

		g.Flush();
	}
}