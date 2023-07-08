#pragma once

#include <string>
#include "util.h"
#include "Bindable/Heap/Heap.h"
#include <assimp/material.h>
#include <assimp/mesh.h>
#include "Bindable/Heap/Texture.h"
#include "Drawable.h"

class Graphics;
class InputLayout;
class FrameGraph;

class Model : public DrawableCollection
{
public:
	Model(Graphics& g, const std::string& filePath);

private:
	friend class Mesh;

	class Mesh : public Drawable
	{
	public:
		friend class Model;
		Mesh(Graphics& g, Model* parent, UINT& diffIdx, const aiMesh* mesh, const aiMaterial* mat, const std::string& dir);
	};
};