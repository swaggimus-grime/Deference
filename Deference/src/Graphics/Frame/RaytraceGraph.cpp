#include "RaytraceGraph.h"
#include "Scene/Model.h"

namespace Def
{
	void RaytraceGraph::PrepLoadScene(Graphics& g)
	{
		auto model = *GetScene().m_Model;
		auto meshes = model.GetMeshes();
		auto textures = model.GetTextures();
		auto materials = model.GetMaterials();
		auto buffers = model.GetBuffers();
		
		GenericBuffer mat_upload_buf(g, D3D12_HEAP_TYPE_UPLOAD, sizeof(Material) * materials.size(), D3D12_RESOURCE_STATE_GENERIC_READ);
		std::memcpy(mat_upload_buf.Map(), materials.data(), mat_upload_buf.Size());
		mat_upload_buf.Unmap();

		m_Globals.MatBuff = MakeShared<StructuredBuffer>(g, D3D12_HEAP_TYPE_DEFAULT, materials.size(), sizeof(Material), D3D12_RESOURCE_STATE_COMMON);
		m_Globals.MatBuff->Transition(g, D3D12_RESOURCE_STATE_COPY_DEST);
		g.CL().CopyResource(**m_Globals.MatBuff, *mat_upload_buf);
		m_Globals.MatBuff->Transition(g, D3D12_RESOURCE_STATE_GENERIC_READ);

		auto c = Commander<D3D12_RESOURCE_BARRIER>::Init();
		for (auto& buff : buffers)
			c.Add(buff.Transition(D3D12_RESOURCE_STATE_COPY_SOURCE));
		c.Transition(g);
		{
			for (auto& mesh : meshes)
			{
				for (auto& sm : mesh.second.m_SubMeshes) {
					auto pos = sm["POSITION"];
					auto indices = sm.GetIndexAttrib();
					auto uv0 = sm["TEXCOORD_0"];
					auto uv1 = sm["TEXCOORD_1"];
					auto uv2 = sm["TEXCOORD_2"];
					auto norm = sm["NORMAL"];
					auto tan = sm["TANGENT"];

					auto vertexBuff   = MakeShared<StructuredBuffer>(g, D3D12_HEAP_TYPE_DEFAULT,     pos.Count, pos.Stride, D3D12_RESOURCE_STATE_COMMON);
					auto indexBuff =    MakeShared<RawBuffer>(g, D3D12_HEAP_TYPE_DEFAULT, indices.Count, indices.Stride, D3D12_RESOURCE_STATE_COMMON);
					auto uvBuff0       = MakeShared<StructuredBuffer>(g, D3D12_HEAP_TYPE_DEFAULT, std::max(1u, uv0.Count), std::max(4u, uv0.Stride), D3D12_RESOURCE_STATE_COMMON);
					auto uvBuff1 = MakeShared<StructuredBuffer>(g, D3D12_HEAP_TYPE_DEFAULT, std::max(1u, uv1.Count), std::max(4u, uv1.Stride), D3D12_RESOURCE_STATE_COMMON);
					auto uvBuff2 = MakeShared<StructuredBuffer>(g, D3D12_HEAP_TYPE_DEFAULT, std::max(1u, uv2.Count), std::max(4u, uv2.Stride), D3D12_RESOURCE_STATE_COMMON);
					auto normalBuff   = MakeShared<StructuredBuffer>(g, D3D12_HEAP_TYPE_DEFAULT,    norm.Count, norm.Stride, D3D12_RESOURCE_STATE_COMMON);
					auto tangentBuff  = MakeShared<StructuredBuffer>(g, D3D12_HEAP_TYPE_DEFAULT,     tan.Count, tan.Stride, D3D12_RESOURCE_STATE_COMMON);

					Commander<D3D12_RESOURCE_BARRIER>::Init()
						.Add(vertexBuff->Transition(D3D12_RESOURCE_STATE_COPY_DEST))
						.Add(indexBuff->Transition(D3D12_RESOURCE_STATE_COPY_DEST))
						.Add(uvBuff0->Transition(D3D12_RESOURCE_STATE_COPY_DEST))
						.Add(uvBuff1->Transition(D3D12_RESOURCE_STATE_COPY_DEST))
						.Add(uvBuff2->Transition(D3D12_RESOURCE_STATE_COPY_DEST))
						.Add(normalBuff->Transition(D3D12_RESOURCE_STATE_COPY_DEST))
						.Add(tangentBuff->Transition(D3D12_RESOURCE_STATE_COPY_DEST))
						.Transition(g);

					g.CL().CopyBufferRegion(**vertexBuff, 0, *buffers[pos.BufferID], pos.ByteOffset, vertexBuff->Size());
					g.CL().CopyBufferRegion(**indexBuff, 0, *buffers[indices.BufferID], indices.ByteOffset, indexBuff->Size());
					g.CL().CopyBufferRegion(**uvBuff0, 0, *buffers[uv0.BufferID], uv0.ByteOffset, uvBuff0->Size());
					g.CL().CopyBufferRegion(**uvBuff1, 0, *buffers[uv1.BufferID], uv1.ByteOffset, uvBuff1->Size());
					g.CL().CopyBufferRegion(**uvBuff2, 0, *buffers[uv2.BufferID], uv2.ByteOffset, uvBuff2->Size());
					g.CL().CopyBufferRegion(**normalBuff, 0, *buffers[norm.BufferID], norm.ByteOffset, normalBuff->Size());
					g.CL().CopyBufferRegion(**tangentBuff, 0, *buffers[tan.BufferID], tan.ByteOffset, tangentBuff->Size());

					Commander<D3D12_RESOURCE_BARRIER>::Init()
						.Add(vertexBuff->Transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE))
						.Add(indexBuff->Transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE))
						.Add(uvBuff0->Transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE))
						.Add(uvBuff1->Transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE))
						.Add(uvBuff2->Transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE))
						.Add(normalBuff->Transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE))
						.Add(tangentBuff->Transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

					m_Globals.Geometries.emplace_back(vertexBuff, indexBuff, uvBuff0, uvBuff1, uvBuff2, normalBuff, tangentBuff, sm.m_Material);
				}
			}
		}
		c = Commander<D3D12_RESOURCE_BARRIER>::Init();
		for (auto& buff : buffers)
			c.Add(buff.Transition(D3D12_RESOURCE_STATE_GENERIC_READ));
		c.Transition(g);

		{
			std::vector<ComPtr<ID3D12Resource>> blass = { model.GetBLAS() };
			m_Globals.TLAS = MakeShared<TLAS>(g, std::move(blass));
		}
		{
			m_Globals.Env = MakeShared<EnvironmentMap>(g, L"res\\textures\\MonValley_G_DirtRoad_3k.hdr");
			m_Globals.Env->Transition(g, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}

		SetGlobals(&m_Globals);
	}
}