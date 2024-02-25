#include "RaytracedGeometryPass.h"
#include <ranges>
#include "RaytraceGraph.h"
#include "Scene/Model.h"
#include "Scene/Camera.h"
#include <ranges>
#include "Resource/DXC.h"

namespace Def
{
	RaytracedGeometryPass::RaytracedGeometryPass(Graphics& g, const std::string& name, FrameGraph* parent)
		:RaytracePass(std::move(name), parent)
	{
		AddOutTarget(g, "Position", DXGI_FORMAT_R8G8B8A8_UNORM);
		AddOutTarget(g, "Normal",   DXGI_FORMAT_R8G8B8A8_UNORM);
		AddOutTarget(g, "Albedo",   DXGI_FORMAT_R8G8B8A8_UNORM);
		AddOutTarget(g, "Specular", DXGI_FORMAT_R8G8B8A8_UNORM);
		AddOutTarget(g, "Emissive", DXGI_FORMAT_R8G8B8A8_UNORM);

		ConstantBufferLayout layout;
		layout.Add<CONSTANT_TYPE::XMFLOAT3>("u");
		layout.Add<CONSTANT_TYPE::FLOAT>("lensRadius");
		layout.Add<CONSTANT_TYPE::XMFLOAT3>("v");
		layout.Add<CONSTANT_TYPE::FLOAT>("focalLength");
		layout.Add<CONSTANT_TYPE::XMFLOAT3>("w");
		layout.Add<CONSTANT_TYPE::UINT>("frameCount");
		layout.Add<CONSTANT_TYPE::XMFLOAT4>("wPos");
		layout.Add<CONSTANT_TYPE::XMFLOAT2>("jitter");
		m_Transform = MakeShared<ConstantBuffer>(g, std::move(layout));
		AddResource(m_Transform);

		auto now = std::chrono::high_resolution_clock::now();
		auto msTime = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
		m_Rng = std::mt19937(uint32_t(msTime.time_since_epoch().count()));
	}

	void RaytracedGeometryPass::Run(Graphics& g)
	{

		static const float kMSAA[8][2] = { { 1,-3 },{ -1,3 },{ 5,1 },{ -3,-5 },{ -5,5 },{ -7,-1 },{ 3,7 },{ 7,-7 } };

		(*m_Transform)["frameCount"] = m_FrameCount++;
		(*m_Transform)["lensRadius"] = 0.f;
		(*m_Transform)["focalLength"] = m_FocalLength;

		// Compute our jitter, either (0,0) as the center or some computed random/MSAA offset
		float xOff = 0.0f, yOff = 0.0f;
		xOff = kMSAA[m_FrameCount % 8][0] * 0.0625f;
		yOff = kMSAA[m_FrameCount % 8][1] * 0.0625f;

		const auto& cam = *m_Parent->GetScene().m_Camera;

		m_LensRadius = m_FocalLength / (2.0f * m_FStop);

		XMStoreFloat3((*m_Transform)["u"], cam.U());
		XMStoreFloat3((*m_Transform)["v"], cam.V());
		XMStoreFloat3((*m_Transform)["w"], cam.W());
		(*m_Transform)["wPos"] = cam.Pos();
		(*m_Transform)["jitter"] = XMFLOAT2{ xOff + 0.5f, yOff + 0.5f };
		
		auto& globals = m_Parent->GetGlobals<RaytraceGraph::Globals>();
		auto& model = m_Parent->GetScene().m_Model;
		auto& textures = model->GetTextures();
		auto& samplers = model->GetSamplers();

		m_Pipeline->Bind(g);
		Commander<ID3D12DescriptorHeap*>::Init()
			.Add(**m_GPUHeap)
			.Add(**m_SamplerHeap)
			.Bind<DescriptorHeap<View>>(g);
		g.CL().SetComputeRootDescriptorTable(0, m_GPUHeap->GetHGPU(m_UAVs.at(0).second));
		g.CL().SetComputeRootDescriptorTable(1, m_GPUHeap->GetHGPU(textures[0]));
		g.CL().SetComputeRootDescriptorTable(2, m_SamplerHeap->GetHGPU(samplers[0]));
		g.CL().SetComputeRootShaderResourceView(3, globals.MatBuff->GetGPUAddress());
		g.CL().SetComputeRootConstantBufferView(4, m_Transform->GetGPUAddress());

		m_Pipeline->Dispatch(g);
		CopyUAVsToRTs(g);
		g.Flush();
	}

	void RaytracedGeometryPass::PrepLoadScene(Graphics& g)
	{
		auto& globals = m_Parent->GetGlobals<RaytraceGraph::Globals>();
		auto& model = m_Parent->GetScene().m_Model;
		auto& textures = model->GetTextures();
		auto& c = Commander<D3D12_RESOURCE_BARRIER>::Init();
		for (auto& t : textures)
		{
			c.Add(t->Transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
			AddResource(t);
		}
		c.Transition(g);
		
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
	}

	void RaytracedGeometryPass::OnSceneLoad(Graphics& g)
	{
		RaytracingPipeline::Desc desc{};

		LPCWSTR exports[] = { anyEP, closestEP };

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
			hg->SetAnyHitShaderImport(anyEP);
			hg->SetClosestHitShaderImport(closestEP);
			hg->SetHitGroupExport(hitGroup);
		}
		//Global
		{
			CD3DX12_DESCRIPTOR_RANGE1 ranges[3];
			ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 5, 0);
			ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, MAX_TEXTURES, 0, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, 0);
			ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);

			CD3DX12_ROOT_PARAMETER1 params[5];
			params[0].InitAsDescriptorTable(1, &ranges[0]);
			params[1].InitAsDescriptorTable(1, &ranges[1]);
			params[2].InitAsDescriptorTable(1, &ranges[2]);
			params[3].InitAsShaderResourceView(5, 1);
			params[4].InitAsConstantBufferView(0);

			auto pSig = MakeUnique<RootSig>(g, _countof(params), params);
			auto sig = so.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
			sig->SetRootSignature(**pSig);
			desc.m_GlobalSig = std::move(pSig);
		}
		//Raygen
		{
			CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
			ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

			CD3DX12_ROOT_PARAMETER1 params[1];
			params[0].InitAsDescriptorTable(1, &ranges[0]);

			auto pSig = MakeUnique<RootSig>(g, _countof(params), params, true);
			auto sig = so.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
			sig->SetRootSignature(**pSig);

			auto ass = so.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
			ass->AddExport(rayGenEP);
			ass->SetSubobjectToAssociate(*sig);
			desc.m_LocalSigs.push_back(std::move(pSig));
		}
		//Hit Group
		{
			CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
			ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0, 1); 

			CD3DX12_ROOT_PARAMETER1 params[2];
			params[0].InitAsConstants(2, 0, 1);
			params[1].InitAsDescriptorTable(1, &ranges[0]);
			auto pSig = MakeUnique<RootSig>(g, 2, params, true);
			auto sig = so.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
			sig->SetRootSignature(**pSig);

			auto ass = so.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
			ass->AddExports(exports);
			ass->SetSubobjectToAssociate(*sig);
			desc.m_LocalSigs.push_back(std::move(pSig));
		}
		//Miss
		{
			CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
			ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

			CD3DX12_ROOT_PARAMETER1 params[1];
			params[0].InitAsDescriptorTable(1, &ranges[0]);

			auto pSig = MakeUnique<RootSig>(g, _countof(params), params, true);
			auto sig = so.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
			sig->SetRootSignature(**pSig);

			auto ass = so.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
			ass->AddExport(missEP);
			ass->SetSubobjectToAssociate(*sig);
			desc.m_LocalSigs.push_back(std::move(pSig));
		}
		{
			auto config = so.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
			config->Config(sizeof(float), sizeof(float) * 2);

			auto ass = so.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
			ass->AddExport(rayGenEP);
			ass->AddExport(closestEP);
			ass->AddExport(anyEP);
			ass->AddExport(missEP);
			ass->SetSubobjectToAssociate(*config);
		}
		{
			auto config = so.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
			config->Config(1);
		}
		m_Pipeline = MakeUnique<RaytracingPipeline>(g, so);

		auto& globals = m_Parent->GetGlobals<RaytraceGraph::Globals>();
		{
			auto tlas = m_GPUHeap->GetHGPU(globals.TLAS);
			auto table = MakeUnique<ShaderBindTable>(g, m_Pipeline.get(), 1, sizeof(tlas));
			table->Add(rayGenEP, &tlas, sizeof(tlas)); //scene
			desc.m_RayGenTable = std::move(table);
		}
		{
			auto env = m_GPUHeap->GetHGPU(globals.Env);
			auto table = MakeUnique<ShaderBindTable>(g, m_Pipeline.get(), 1, sizeof(env));
			table->Add(missEP, &env, sizeof(env));
			desc.m_MissTable = std::move(table);
		}
		{
			auto table = MakeUnique<ShaderBindTable>(g, m_Pipeline.get(), globals.Geometries.size(), sizeof(HitEntry));
			for (auto& geo : globals.Geometries)
			{
				HitEntry entry{};
				entry.v = m_GPUHeap->GetHGPU(geo.Vertices); //geo.Vertices->GetGPUAddress();
				entry.i = m_GPUHeap->GetHGPU(geo.Indices);  //geo.Indices->GetGPUAddress();
				entry.uv= m_GPUHeap->GetHGPU(geo.UVs);      //geo.UVs->GetGPUAddress();
				entry.n = m_GPUHeap->GetHGPU(geo.Normals);  //geo.Normals->GetGPUAddress();
				entry.t = m_GPUHeap->GetHGPU(geo.Tangents); //geo.Tangents->GetGPUAddress();
				entry.m = geo.MaterialID;
				entry.istride = geo.Indices->GetStride();
				
				table->Add(hitGroup, &entry, sizeof(entry));
			}

			desc.m_HitTable = std::move(table);
		}
		m_Pipeline->SubmitTablesAndSigs(g, desc);
	}

	void RaytracedGeometryPass::ShowGUI()
	{
	}
}