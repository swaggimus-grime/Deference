#pragma once

#include "Resource/VertexBuffer.h"
#include "Resource/Buffer.h"
#include "Resource/Texture.h"
#include "Resource/Sampler.h"
#include "BBox.h"
#include <tiny_gltf.h>

namespace Def
{
	struct Material
	{
		struct TextureIndexer {
			INT ID;
			INT TexCoord;
		};

		XMFLOAT4 BaseColor;
		float Roughness;
		float Metallic;
		TextureIndexer BaseTex	   ;
		TextureIndexer RMTex;
		TextureIndexer NormTex   ;
		TextureIndexer OccTex;
		TextureIndexer EmissiveTex;
		XMFLOAT3 EmissiveColor;
		UINT IsDoubleSided;
		float RefractiveIndex;
	};

	struct SceneNode
	{
		int Id = -1;
		XMMATRIX Transform = XMMatrixIdentity();
		std::vector<Shared<SceneNode>> Children;
	};

	class Model
	{
	public:
		struct GltfAttribute
		{
			UINT BufferID;
			D3D12_GPU_VIRTUAL_ADDRESS Location;
			UINT ByteOffset;
			UINT Stride;
			UINT Count;
			DXGI_FORMAT Format;
			UINT ByteLength;
		};

		struct SubMesh
		{
			D3D_PRIMITIVE_TOPOLOGY m_Topology;
			UINT m_Material;
			UINT m_NumVertices = 0;
			D3D12_GPU_VIRTUAL_ADDRESS m_PosStart;
			D3D12_GPU_VIRTUAL_ADDRESS m_IndexStart;

			void AddVertexAttrib(const std::string& name, GltfAttribute& attr)
			{
				D3D12_VERTEX_BUFFER_VIEW view{
					.BufferLocation = attr.Location + attr.ByteOffset,
					.SizeInBytes = attr.ByteLength,
					.StrideInBytes = attr.Stride
				};

				m_VertAttribs.insert({ std::move(name), std::move(attr) });
				m_VertViews.push_back(std::move(view));
			}

			void SetIndexAttrib(GltfAttribute& attr)
			{
				m_Index.first = std::move(attr);
				m_Index.second = {
					.BufferLocation = attr.Location + attr.ByteOffset,
					.SizeInBytes = attr.ByteLength,
					.Format = attr.Format
				};
			}

			void Rasterize(Graphics& g)
			{
				g.CL().IASetVertexBuffers(0, m_VertAttribs.size(), m_VertViews.data());
				g.CL().IASetIndexBuffer(&m_Index.second);
				g.CL().IASetPrimitiveTopology(m_Topology);
				g.CL().DrawIndexedInstanced(m_Index.first.Count, 1, 0, 0, 0);
			}

			inline UINT NumIndices() const { return m_Index.first.Count; }
			inline GltfAttribute GetIndexAttrib() const { return m_Index.first; }

			inline GltfAttribute operator[](const std::string& name)
			{
				return m_VertAttribs[name];
			}

		private:
			std::unordered_map<std::string, GltfAttribute> m_VertAttribs;
			std::vector<D3D12_VERTEX_BUFFER_VIEW> m_VertViews;
			std::pair<GltfAttribute, D3D12_INDEX_BUFFER_VIEW> m_Index;
		};

		struct Mesh
		{
			std::vector<SubMesh> m_SubMeshes;
		};

	public:
		Model(Graphics& g, const std::string& filePath);
		inline Shared<SceneNode> GetRootNode() { return m_RootNode; }
		inline const auto& GetBLAS() const { return m_BLAS; }
		inline const auto& GetSamplers() const { return m_Samplers; }
		inline auto& GetBuffers() const { return m_GPUBuffers; }
		inline auto& GetMeshes() { return m_Meshes; }
		inline auto& GetMaterials() { return m_Materials; }
		inline auto& GetTextures() { return m_Textures; }
		inline BBox GetBBox() const { return m_BBox; }
		inline D3D12_INPUT_LAYOUT_DESC GetLayout() const { return *m_Layout; }

		inline UINT NumSubMeshes() const { return m_NumSubMeshes; }

	private:
		Shared<SceneNode> ParseScene(int sceneId);
		void ParseVertices(Graphics& g);
		void ParseMaterials(Graphics& g);
		void ParseTextures(Graphics& g, const std::string& baseDir);
		void ParseSamplers();
		void CreateGeometry(Graphics& g, std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>& descs, XMMATRIX parentTransform, Shared<SceneNode> node);

		template<typename IDX>
		void CalculateTangents(Graphics& g, tinygltf::Primitive& primitive, SubMesh& sm, GltfAttribute& attrib);

	private:
		tinygltf::Model m_Model;
		ComPtr<ID3D12Resource> m_BLAS;
		BBox m_BBox;
		Shared<SceneNode> m_RootNode;
		std::map<UINT, Mesh> m_Meshes;
		std::vector<Material> m_Materials;
		std::vector<Shared<Texture2D>> m_Textures;
		std::vector<Shared<Sampler>> m_Samplers;
		std::vector<GenericBuffer> m_GPUBuffers;
		InputLayout m_Layout;
		UINT m_NumSubMeshes;
	};

	template<typename IDX>
	void Model::CalculateTangents(Graphics& g, tinygltf::Primitive& primitive, SubMesh& sm, GltfAttribute& attrib)
	{
		auto pos = sm["POSITION"];
		size_t verticesCount = pos.Count;
		auto norm = sm["NORMAL"];
		auto uv = sm["TEXCOORD_0"];
		auto idx = sm.GetIndexAttrib();

		tinygltf::BufferView positionsBV = m_Model.bufferViews[m_Model.accessors[primitive.attributes["POSITION"]].bufferView];
		DirectX::XMFLOAT3* vertices = (DirectX::XMFLOAT3*)(m_Model.buffers[positionsBV.buffer].data.data() + pos.ByteOffset);

		tinygltf::BufferView normalsBV = m_Model.bufferViews[m_Model.accessors[primitive.attributes["NORMAL"]].bufferView];
		DirectX::XMFLOAT3* normals = (DirectX::XMFLOAT3*)(m_Model.buffers[normalsBV.buffer].data.data() + norm.ByteOffset);

		tinygltf::BufferView texCoord0BV = m_Model.bufferViews[m_Model.accessors[primitive.attributes["TEXCOORD_0"]].bufferView];
		DirectX::XMFLOAT2* texCoords = (DirectX::XMFLOAT2*)(m_Model.buffers[texCoord0BV.buffer].data.data() + uv.ByteOffset);

		tinygltf::BufferView indicesBV = m_Model.bufferViews[m_Model.accessors[primitive.indices].bufferView];
		IDX* indexes = (IDX*)(m_Model.buffers[indicesBV.buffer].data.data() + idx.ByteOffset);
		size_t indexesCount = idx.Count;

		std::vector<XMFLOAT3> tan1(verticesCount);
		std::vector<XMFLOAT3> tan2(verticesCount);
		std::vector<XMFLOAT4> tangent(verticesCount);

		for (size_t i = 0; i < indexesCount; i += 3)
		{
			long i1 = indexes[i];
			long i2 = indexes[i + 1];
			long i3 = indexes[i + 2];

			DirectX::XMFLOAT3 v1 = vertices[i1];
			DirectX::XMFLOAT3 v2 = vertices[i2];
			DirectX::XMFLOAT3 v3 = vertices[i3];

			DirectX::XMFLOAT2 w1 = texCoords[i1];
			DirectX::XMFLOAT2 w2 = texCoords[i2];
			DirectX::XMFLOAT2 w3 = texCoords[i3];

			float x1 = v2.x - v1.x;
			float x2 = v3.x - v1.x;
			float y1 = v2.y - v1.y;
			float y2 = v3.y - v1.y;
			float z1 = v2.z - v1.z;
			float z2 = v3.z - v1.z;
			float s1 = w2.x - w1.x;
			float s2 = w3.x - w1.x;
			float t1 = w2.y - w1.y;
			float t2 = w3.y - w1.y;
			float r = 1.0f / (s1 * t2 - s2 * t1);
			DirectX::XMFLOAT3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
			DirectX::XMFLOAT3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);
			tan1[i1] = { tan1[i1].x + sdir.x, tan1[i1].y + sdir.y, tan1[i1].z + sdir.z };
			tan1[i2] = { tan1[i2].x + sdir.x, tan1[i2].y + sdir.y, tan1[i2].z + sdir.z };
			tan1[i3] = { tan1[i3].x + sdir.x, tan1[i3].y + sdir.y, tan1[i3].z + sdir.z };
			tan2[i1] = { tan2[i1].x + sdir.x, tan2[i1].y + sdir.y, tan2[i1].z + sdir.z };
			tan2[i2] = { tan2[i2].x + sdir.x, tan2[i2].y + sdir.y, tan2[i2].z + sdir.z };
			tan2[i3] = { tan2[i3].x + sdir.x, tan2[i3].y + sdir.y, tan2[i3].z + sdir.z };
		}

		for (size_t i = 0; i < verticesCount; i++)
		{

			DirectX::XMVECTOR n = DirectX::XMLoadFloat3(&normals[i]);
			DirectX::XMVECTOR t = DirectX::XMLoadFloat3(&tan1[i]);

			// Gram-Schmidt orthogonalize        
			DirectX::XMVECTOR xmTangent = XMVector3Normalize(XMVectorSubtract(t, XMVectorScale(n, XMVectorGetX(XMVector3Dot(n, t)))));

			// Calculate handedness        
			DirectX::XMVECTOR t2 = DirectX::XMLoadFloat3(&tan2[i]);
			float handedness = (XMVectorGetX(XMVector3Dot(XMVector3Cross(n, t), t2)) < 0.0f) ? -1.0f : 1.0f;
			xmTangent = DirectX::XMVectorSetW(xmTangent, handedness);
			DirectX::XMStoreFloat4(&tangent[i], xmTangent);
		}

		GenericBuffer buffer(g, tangent.data(), verticesCount * sizeof(DirectX::XMFLOAT4));
		attrib.BufferID = m_GPUBuffers.size();
		attrib.Location = buffer.GetGPUAddress();
		m_GPUBuffers.push_back(std::move(buffer));
	}

}