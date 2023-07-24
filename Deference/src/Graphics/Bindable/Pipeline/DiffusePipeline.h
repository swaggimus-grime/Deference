#pragma once

#include "RaytracingPipeline.h"

class DiffusePipeline : public RaytracingPipeline
{
public:
	DiffusePipeline(Graphics& g);

	static constexpr LPCWSTR shaderFile = L"shaders\\diffuse.rt.hlsl";
	static constexpr LPCWSTR rayGenEP = L"DiffuseAndHardShadow";
	static constexpr LPCWSTR hitGroup = L"ShadowHit";
	static constexpr LPCWSTR missEP = L"ShadowMiss";
	static constexpr LPCWSTR anyEP = L"ShadowAnyHit";

private:
	Shared<RootSig> m_RayGenSig;
	Shared<RootSig> m_HitSig;
	Shared<RootSig> m_MissSig;
};