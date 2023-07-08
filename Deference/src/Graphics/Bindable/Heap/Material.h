#pragma once

#include "ConstantBuffer.h"

struct MaterialParams
{
	XMFLOAT3 diffuseColor;
	UINT diffTableIdx;
};

class Material : public ConstantBuffer<MaterialParams>
{
public:
	Material(Graphics& g, const D3D12_CPU_DESCRIPTOR_HANDLE& handle);
};