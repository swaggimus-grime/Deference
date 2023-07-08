#pragma once

#include "Resource.h"

class VertexBuffer;
class IndexBuffer;
class Drawable;

class TLAS : public Resource
{
public:
	TLAS(Graphics& g, const D3D12_CPU_DESCRIPTOR_HANDLE& handle, const std::vector<Shared<Drawable>>& drawables);
	static ComPtr<ID3D12Resource> BLAS(Graphics& g, const std::vector<Shared<VertexBuffer>>& vbs, const std::vector<Shared<IndexBuffer>>& ibs);
};

