#include "Material.h"

Material::Material(Graphics& g, const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
	:ConstantBuffer(g, handle)
{
}
