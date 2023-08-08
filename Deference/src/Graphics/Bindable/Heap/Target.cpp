#include "Target.h"

Target::Target()
{
	m_ShaderResourceHCPU.ptr = 0;
}

void Target::CreateShaderResourceView(Graphics& g, HDESC h)
{
	m_ShaderResourceHCPU = h;
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Format = m_Res->GetDesc().Format;
	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Texture2D.MipLevels = 1;
	desc.Texture2D.MostDetailedMip = 0;
	desc.Texture2D.PlaneSlice = 0;

	g.Device().CreateShaderResourceView(m_Res.Get(), &desc, m_ShaderResourceHCPU);
}

void Target::CopyShaderResourceView(Graphics& g, HCPU h)
{
	g.Device().CopyDescriptorsSimple(1, h, m_ShaderResourceHCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}
