#pragma once

#include <string>
#include "util.h"
#include <assimp/material.h>
#include <assimp/mesh.h>
#include "Bindable/Heap/Texture.h"

class Graphics;
class InputLayout;
class FrameGraph;

struct TextureIndex
{
	INT m_Diffuse;
	INT m_Specular;
	INT m_Normal;
};

class Model
{
public:
	Model(Graphics& g, const std::string& filePath);
	inline const auto& GetBuffers() const { return m_Buffers; }
	inline const auto& GetBLAS() const { return m_BLAS; }

	inline const auto& GetTextureIndexes() const { return m_TextureIndexes; }
	inline const auto& GetTextures() const { return m_Textures; }

	inline XMMATRIX GetWorldTransform() const { return m_World; }

private:
	XMMATRIX m_World;

	std::vector<std::pair<Shared<VertexBuffer>, Shared<IndexBuffer>>> m_Buffers;
	ComPtr<ID3D12Resource> m_BLAS;

	std::vector<TextureIndex> m_TextureIndexes;
	std::vector<Shared<Texture2D>> m_Textures;
};