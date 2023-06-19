#pragma once

#include "Resource.h"
#include <d3d12.h>

class ShaderAccessible : public Resource
{
public:
	ShaderAccessible(D3D12_CPU_DESCRIPTOR_HANDLE handle, D3D12_RESOURCE_STATES state, ComPtr<ID3D12Resource> res = nullptr);

public:
	static constexpr auto s_Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
};