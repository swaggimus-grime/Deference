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
		
		RawBuffer mat_upload_buf(g, D3D12_HEAP_TYPE_UPLOAD, sizeof(Material) * materials.size(), D3D12_RESOURCE_STATE_GENERIC_READ);
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
					auto uv = sm["TEXCOORD_0"];				
					auto norm = sm["NORMAL"];
					auto tan = sm["TANGENT"];

					auto vertexBuff   = MakeShared<StructuredBuffer>(g, D3D12_HEAP_TYPE_DEFAULT,     pos.Count, pos.Stride, D3D12_RESOURCE_STATE_COMMON);
					auto indexBuff =    MakeShared<ByteBuffer>(g, D3D12_HEAP_TYPE_DEFAULT, indices.Count, indices.Stride, D3D12_RESOURCE_STATE_COMMON);
					auto uvBuff       = MakeShared<StructuredBuffer>(g, D3D12_HEAP_TYPE_DEFAULT, std::max(uv.Count, 1u), std::max(uv.Stride, 4u), D3D12_RESOURCE_STATE_COMMON);
					auto normalBuff   = MakeShared<StructuredBuffer>(g, D3D12_HEAP_TYPE_DEFAULT,    norm.Count, norm.Stride, D3D12_RESOURCE_STATE_COMMON);
					auto tangentBuff  = MakeShared<StructuredBuffer>(g, D3D12_HEAP_TYPE_DEFAULT,     tan.Count, tan.Stride, D3D12_RESOURCE_STATE_COMMON);

					Commander<D3D12_RESOURCE_BARRIER>::Init()
						.Add(vertexBuff->Transition(D3D12_RESOURCE_STATE_COPY_DEST))
						.Add(indexBuff->Transition(D3D12_RESOURCE_STATE_COPY_DEST))
						.Add(uvBuff->Transition(D3D12_RESOURCE_STATE_COPY_DEST))
						.Add(normalBuff->Transition(D3D12_RESOURCE_STATE_COPY_DEST))
						.Add(tangentBuff->Transition(D3D12_RESOURCE_STATE_COPY_DEST))
						.Transition(g);

					g.CL().CopyBufferRegion(**vertexBuff, 0, *buffers[pos.BufferID], pos.ByteOffset, pos.ByteLength);
					g.CL().CopyBufferRegion(**indexBuff, 0, *buffers[indices.BufferID], indices.ByteOffset, indices.ByteLength);
					g.CL().CopyBufferRegion(**uvBuff, 0, *buffers[uv.BufferID], uv.ByteOffset, uv.ByteLength);
					g.CL().CopyBufferRegion(**normalBuff, 0, *buffers[norm.BufferID], norm.ByteOffset, norm.ByteLength);
					g.CL().CopyBufferRegion(**tangentBuff, 0, *buffers[tan.BufferID], tan.ByteOffset, tan.ByteLength);

					Commander<D3D12_RESOURCE_BARRIER>::Init()
						.Add(vertexBuff->Transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE))
						.Add(indexBuff->Transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE))
						.Add(uvBuff->Transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE))
						.Add(normalBuff->Transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE))
						.Add(tangentBuff->Transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

					m_Globals.Geometries.emplace_back(vertexBuff, indexBuff, uvBuff, normalBuff, tangentBuff, sm.m_Material);
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