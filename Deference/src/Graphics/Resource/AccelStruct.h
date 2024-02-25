#pragma once

#include "Resource.h"
#include "View.h"

namespace Def
{
	class TLAS : public Resource, public SRV
	{
	public:
		TLAS(Graphics& g, const std::vector<ComPtr<ID3D12Resource>>& blass);
		virtual ID3D12Resource* operator*() const override { return nullptr; }
		static ComPtr<ID3D12Resource> BLAS(Graphics& g, const std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>& geometries);
		virtual const D3D12_SHADER_RESOURCE_VIEW_DESC& SRVDesc() const override;
	};
}