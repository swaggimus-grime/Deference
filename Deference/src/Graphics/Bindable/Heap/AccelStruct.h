#pragma once

#include "Resource.h"

class VertexBuffer;
class IndexBuffer;

class TLAS : public Resource
{
public:
	TLAS(Graphics& g, const std::vector<ComPtr<ID3D12Resource>>& blass);
	static ComPtr<ID3D12Resource> BLAS(Graphics& g, const std::vector<Shared<VertexBuffer>>& vbs, const std::vector<Shared<IndexBuffer>>& ibs);
	static ComPtr<ID3D12Resource> BLAS(Graphics& g, const std::vector<std::pair<Shared<VertexBuffer>, Shared<IndexBuffer>>>& buffers);

	virtual void CreateView(Graphics& g, HCPU hcpu) override;

};