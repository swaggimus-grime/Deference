#pragma once

#include <string>
#include "util.h"
#include "Drawable.h"
#include "Resource/Heap.h"
#include <assimp/material.h>
#include <assimp/mesh.h>
#include "Resource/Texture.h"
#include "Bindable/VertexBuffer.h"
#include "Bindable/IndexBuffer.h"

class Graphics;
class InputLayout;
class FrameGraph;

class Model : public DrawableCollection
{
public:
	Model(Graphics& g, const std::string& filePath);
	inline auto& operator[](UINT idx) const { return m_Meshes[idx]; }
	inline UINT NumMeshes() const { return m_Meshes.size(); }

private:
	class Material : public Bindable
	{
	public:
		friend class Model;
		Material(Graphics& g, const aiMaterial* mat, const std::string& dir);
		virtual void Bind(Graphics& g) override;

	private:
		Unique<Heap<Texture2D>> m_TextureHeap;
	};

private:
	class Mesh : public Drawable
	{
	public:
		friend class Model;
		Mesh(Graphics& g, const aiMesh* mesh, Shared<Material> mat, const InputLayout& layout);

	private:
		Shared<VertexBuffer> m_VB;
		Shared<IndexBuffer> m_IB;
	};


private:
	std::vector<Mesh> m_Meshes;
	std::vector<Shared<Material>> m_Materials;
};