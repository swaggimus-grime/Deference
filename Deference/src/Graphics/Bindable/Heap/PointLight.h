#pragma once

#include "ConstantBuffer.h"
#include "util.h"

struct PointLightParams
{
	alignas(16) XMFLOAT3 m_Pos;
	alignas(16) XMFLOAT3 m_Color;
	float m_Intensity;
};

class PointLight : public ConstantBuffer<PointLightParams>
{
public:
	PointLight(Graphics& g, const D3D12_CPU_DESCRIPTOR_HANDLE& handle);
};