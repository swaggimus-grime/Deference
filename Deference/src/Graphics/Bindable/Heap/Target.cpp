#include "Target.h"

void Target::CreateShaderResourceView(Graphics& g, HDESC h)
{
	m_ShaderResourceHandle = h;
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Format = m_Res->GetDesc().Format;
	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Texture2D.MipLevels = 1;
	desc.Texture2D.MostDetailedMip = 0;
	desc.Texture2D.PlaneSlice = 0;

	g.Device().CreateShaderResourceView(m_Res.Get(), &desc, m_ShaderResourceHandle.m_HCPU);
}