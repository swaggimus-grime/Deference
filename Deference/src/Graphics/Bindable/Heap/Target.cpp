#include "Target.h"

Target::Target()
	:m_ShaderHCPU({0})
{
}

void Target::CreateShaderView(Graphics& g, HCPU h)
{
	m_ShaderHCPU = h;
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Format = m_Res->GetDesc().Format;
	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Texture2D.MipLevels = 1;
	desc.Texture2D.MostDetailedMip = 0;
	desc.Texture2D.PlaneSlice = 0;

	g.Device().CreateShaderResourceView(m_Res.Get(), &desc, m_ShaderHCPU);
}