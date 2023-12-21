#pragma once

#include <string>
#include "util.h"
#include <assimp/material.h>
#include <assimp/mesh.h>
#include "Bindable/Heap/Texture.h"
#include "BBox.h"

class Graphics;
class InputLayout;
class FrameGraph;

class Model
{
public:
	Model(Graphics& g, const std::string& filePath);
	inline const auto& GetBLAS() const { return m_BLAS; }
	inline const auto& GetMeshes() const { return m_Meshes; }
	inline XMMATRIX GetWorldTransform() const { return m_World; }
	inline BBox GetBBox() const { return m_BBox; }

public:
	struct Mesh
	{
		Shared<VertexBuffer> m_VB;
		Shared<IndexBuffer> m_IB;
		Shared<Texture2D> m_DiffuseMap;
		Shared<Texture2D> m_NormalMap;
		Shared<Texture2D> m_SpecularMap;
		Shared<Texture2D> m_EmissiveMap;
		Shared<ConstantBuffer> m_Materials;
	};

private:
	XMMATRIX m_World;
	ComPtr<ID3D12Resource> m_BLAS;
	std::vector<Mesh> m_Meshes;
	BBox m_BBox;
};