#include "PointLight.h"

PointLight::PointLight(Graphics& g, const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
	:ConstantBuffer(g, handle)
{
	PointLightParams params;
	params.m_Pos = { 1, 3, 4 };
	params.m_Color = { 1.f, 1.f, 0.f };
	params.m_Intensity = 2.f;

	UpdateStruct(&params);
}