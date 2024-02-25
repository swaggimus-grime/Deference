#include "Target.h"

namespace Def
{
	Target::Target()
		:SRV(this)
	{}

	const D3D12_SHADER_RESOURCE_VIEW_DESC& Target::SRVDesc() const
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
		desc.Format = GetFormat();
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.Texture2D.MipLevels = 1;
		desc.Texture2D.MostDetailedMip = 0;
		desc.Texture2D.PlaneSlice = 0;
		return std::move(desc);
	}
}